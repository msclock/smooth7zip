#pragma once

#include <functional>

#include <7zip/CPP/7zip/Common/FileStreams.h>
#include <7zip/CPP/Common/ComTry.h>
#include <7zip/CPP/Common/IntToString.h>
#include <7zip/CPP/Common/MyCom.h>
#include <7zip/CPP/Windows/FileFind.h>
#include <7zip/CPP/Windows/FileIO.h>

#include <7zip/CPP/Windows/FileDir.h>
#include <7zip/CPP/Windows/PropVariant.h>
#include <7zip/CPP/Windows/PropVariantConv.h>

#include "common.hpp"
#include "format.hpp"

namespace smooth7zip {

struct handler_callbacks {
    /**
     * @brief A std::function whose argument is the total size of the ongoing operation.
     */
    std::function<void(uint64_t)> total{nullptr};

    /**
     * @brief A std::function whose argument is the currently processed size of the ongoing operation and returns true
     * or false whether the operation must continue or not.
     */
    std::function<bool(uint64_t)> progress{nullptr};

    /**
     * @brief A std::function whose arguments are the current processed input size, and the current output size of
     * theongoing operation.
     */
    std::function<void(uint64_t, uint64_t)> ratio{nullptr};

    /**
     * @brief A std::function whose argument is the path, in the archive, of the file currently being processed by the
     * ongoing operation.
     */
    std::function<void(std::string)> file{nullptr};

    /**
     * @brief A std::function returning the password to be used to handle an archive.
     */
    std::function<std::string()> password{nullptr};
};

class archive_handler {
public:
    archive_handler(handler_callbacks _callbacks = {}) : callbacks_(_callbacks) {}

    virtual ~archive_handler() = default;

    handler_callbacks &callbacks() { return this->callbacks_; }

private:
    handler_callbacks callbacks_;
};

class archive_update_callback : public IArchiveUpdateCallback2,
                                public ICompressProgressInfo,
                                public ICryptoGetTextPassword2,
                                public CMyUnknownImp {
    Z7_COM_UNKNOWN_IMP_3(IArchiveUpdateCallback2, ICryptoGetTextPassword2, ICompressProgressInfo)

public:
    archive_update_callback(archive_handler &_handler) : handler(_handler) {};

    virtual ~archive_update_callback() { finalize(); }

    // ICompressProgressInfo
    // S7_STDMETHOD(SetRatioInfo, const UInt64 *inSize, const UInt64 *outSize);
    auto SetRatioInfo(const UInt64 *inSize, const UInt64 *outSize) noexcept -> LONG override;

    // IProgress from IArchiveUpdateCallback2
    S7_STDMETHOD(SetTotal, UInt64 size) {
        if (handler.callbacks().total) {
            handler.callbacks().total(size);
        }
        return S_OK;
    }

    S7_STDMETHOD(SetCompleted, const UInt64 *completeValue) {
        return handler.callbacks().progress ? handler.callbacks().progress(*completeValue) : S_OK;
    }

    // IArchiveUpdateCallback2

    S7_STDMETHOD(GetUpdateItemInfo, UInt32 index, Int32 *newData, Int32 *newProps, UInt32 *indexInArchive) {
        if (newData != nullptr) {
            *newData = static_cast<Int32>(this->has_new_data(index)); // 1 = true, 0 = false;
        }
        if (newProps != nullptr) {
            *newProps = static_cast<Int32>(this->has_new_properties(index)); // 1 = true, 0 = false;
        }
        if (indexInArchive != nullptr) {
            *indexInArchive = this->index_in_archive(index);
        }
        return S_OK;
    }

    S7_STDMETHOD(GetProperty, UInt32 index, PROPID propID, PROPVARIANT *value) {
        NWindows::NCOM::CPropVariant prop;
        if (propID == kpidIsAnti) {
            prop = false;
            prop.Detach(value);
            return S_OK;
        }

        // TODO
        const NWindows::NFile::NFind::CFileInfo &file_info = (*dir_items)[index];
        switch (propID) {
            case kpidPath:
                prop = fs2us(file_info.Name);
                break;
            case kpidIsDir:
                prop = file_info.IsDir();
                break;
            case kpidSize:
                prop = file_info.Size;
                break;
            case kpidCTime:
                PropVariant_SetFrom_FiTime(prop, file_info.CTime);
                break;
            case kpidATime:
                PropVariant_SetFrom_FiTime(prop, file_info.ATime);
                break;
            case kpidMTime:
                PropVariant_SetFrom_FiTime(prop, file_info.MTime);
                break;
            case kpidAttrib:
                prop = file_info.GetWinAttrib();
                break;
            case kpidPosixAttrib:
                prop = file_info.GetPosixAttrib();
                break;
            default:
                break;
        }
        return prop.Detach(value);
    }

    S7_STDMETHOD(GetStream, UInt32 index, ISequentialInStream **inStream) {
        RINOK(finalize());

        // TODO
        COM_TRY_BEGIN
        const NWindows::NFile::NFind::CFileInfo &fi = (*dir_items)[index];

        if (fi.IsDir())
            return S_OK;

        {
            CInFileStream *in_file_stream_spec = new CInFileStream;
            CMyComPtr<ISequentialInStream> sequential_in_stream = in_file_stream_spec;

            FString update_path_prefix = this->path_prefix;
            if (update_path_prefix.Back() != CHAR_PATH_SEPARATOR)
                update_path_prefix.Add_PathSepar();

            if (!in_file_stream_spec->Open(update_path_prefix + fi.Name)) {
                const auto last_sys_error = ::GetLastError();
                failed_codes.Add(HRESULT_FROM_WIN32(last_sys_error));
                failed_files.Add(update_path_prefix + fi.Name);
                return S_FALSE;
            }
            *inStream = sequential_in_stream.Detach();
        }

        return S_OK;

        COM_TRY_END
    }

    S7_STDMETHOD(SetOperationResult, Int32 operationResult) {
        // TODO
        (void)operationResult;
        is_need_be_closed = true;
        return S_OK;
    }

    S7_STDMETHOD(GetVolumeSize, UInt32 index, UInt64 *size) {
        if (volumes_sizes.Size() == 0)
            return S_FALSE;

        if (index > volumes_sizes.Size())
            index = volumes_sizes.Size() - 1;

        *size = volumes_sizes[index];
        return S_OK;
    }

    S7_STDMETHOD(GetVolumeStream, UInt32 index, ISequentialOutStream **volumeStream) {
        COM_TRY_BEGIN
        wchar_t temp[16]{};
        ConvertUInt32ToString(index, temp);
        UString res = temp;
        while (res.Len() < 2) {
            res.InsertAtFront(L'0');
        }

        UString volume_file_name = this->volume_name;
        volume_file_name.Add_Dot();
        volume_file_name += res;
        volume_file_name += this->volume_ext;

        COutFileStream *volume_out_file_stream_spec = new COutFileStream;
        CMyComPtr<ISequentialOutStream> volume_sequential_out_stream = volume_out_file_stream_spec;
        if (volume_out_file_stream_spec->Create(us2fs(volume_file_name), false)) {
            return GetLastError_noZero_HRESULT();
        }
        *volumeStream = volume_sequential_out_stream.Detach();
        return S_OK;
        COM_TRY_END
    }

    // ICryptoGetTextPassword2
    S7_STDMETHOD(CryptoGetTextPassword2, Int32 *passwordIsDefined, BSTR *password) {
        *passwordIsDefined = static_cast<Int32>(!this->passwd.IsEmpty());
        return StringToBstr(passwd, password);
    }

    virtual auto has_new_data(uint32_t index) const noexcept -> bool {
        // TODO
        (void)index;
        return true;
    }

    virtual auto has_new_properties(uint32_t index) const noexcept -> bool {
        // TODO
        (void)index;
        return true;
    }

    auto index_in_archive(uint32_t index) const noexcept -> uint32_t {
        constexpr uint32_t index_in_archive_not_set = static_cast<uint32_t>(-1);
        // TODO
        (void)index;
        return index_in_archive_not_set;
    }

    HRESULT finalize() {
        is_need_be_closed = false;
        return S_OK;
    }

    archive_handler &handler;
    bool is_need_be_closed{false};

    const CObjectVector<NWindows::NFile::NFind::CFileInfo> *dir_items{nullptr};
    FString path_prefix;
    FStringVector failed_files;
    CRecordVector<HRESULT> failed_codes;
    CRecordVector<UInt64> volumes_sizes;
    UString volume_name;
    UString volume_ext;
    UString passwd;
};

class archive_open_callback : public IArchiveOpenCallback, public ICryptoGetTextPassword, public CMyUnknownImp {
    Z7_COM_UNKNOWN_IMP_2(IArchiveOpenCallback, ICryptoGetTextPassword)

public:
    archive_open_callback(archive_handler &_handler) : handler(_handler) {};

    S7_STDMETHOD(SetTotal, const UInt64 * /* files */, const UInt64 * /* bytes */) { return S_OK; }

    S7_STDMETHOD(SetCompleted, const UInt64 * /* files */, const UInt64 * /* bytes */) { return S_OK; }

    S7_STDMETHOD(CryptoGetTextPassword, BSTR *password) { return StringToBstr(passwd, password); }

    archive_handler &handler;
    UString passwd;
};

struct archive_item {
    FILETIME filetime{0, 0};
    UInt16 precision;
    Byte ns100;
    bool def;

    archive_item() { clear(); }

    void clear() {
        this->filetime = {0, 0};
        this->precision = 0;
        this->ns100 = 0;
        this->def = false;
    }

    bool is_zero() const {
        return this->filetime.dwLowDateTime == 0 && this->filetime.dwHighDateTime == 0 && this->ns100 == 0;
    }

    int get_num_digits() const {
        if (this->precision == k_PropVar_TimePrec_Unix || this->precision == k_PropVar_TimePrec_DOS)
            return 0;
        if (this->precision == k_PropVar_TimePrec_HighPrec)
            return 9;
        if (this->precision == k_PropVar_TimePrec_0)
            return 7;
        int digits = static_cast<int>(this->precision) - k_PropVar_TimePrec_Base;
        if (digits < 0)
            digits = 0;
        return digits;
    }

    void write_to_filetime(CFiTime &dest) const {
#ifdef _WIN32
        dest = this->filetime;
#else
        if (FILETIME_To_timespec(this->filetime, dest)) {
            if ((this->precision == k_PropVar_TimePrec_Base + 8 || this->precision == k_PropVar_TimePrec_Base + 9)
                && this->ns100 != 0) {
                dest.tv_nsec += this->ns100;
            }
        }
#endif
    }

    void set_from_prop(const PROPVARIANT &prop) {
        this->filetime = prop.filetime;

        unsigned prec = 0;
        unsigned ns = 0;
        const unsigned prec_temp = prop.wReserved1;
        if (prec_temp != 0 && prec_temp <= k_PropVar_TimePrec_1ns && prop.wReserved3 == 0) {
            const unsigned ns_temp = prop.wReserved2;
            if (ns_temp < 100) {
                ns = ns_temp;
                prec = prec_temp;
            }
        }
        this->precision = static_cast<UInt16>(prec);
        this->ns100 = static_cast<Byte>(ns);
        this->def = true;
    }
};

inline const wchar_t *const g_empty_file_alias = L"[Content]";

inline HRESULT check_bool_archive_item_prop(IInArchive *archive, UInt32 index, PROPID prop_id, bool &result) {
    NWindows::NCOM::CPropVariant prop;
    RINOK(archive->GetProperty(index, prop_id, &prop));

    if (prop.vt == VT_BOOL)
        result = VARIANT_BOOLToBool(prop.boolVal);
    else if (prop.vt == VT_EMPTY)
        result = false;
    else
        return E_FAIL;

    return S_OK;
}

class archive_extract_callback : public IArchiveExtractCallback,
                                 public ICompressProgressInfo,
                                 public ICryptoGetTextPassword,
                                 public CMyUnknownImp {
    Z7_COM_UNKNOWN_IMP_3(IArchiveExtractCallback, ICompressProgressInfo, ICryptoGetTextPassword);

public:
    archive_extract_callback(archive_handler &_handler) : handler(_handler) {};

    S7_STDMETHOD(SetTotal, UInt64 total) {
        if (handler.callbacks().total) {
            handler.callbacks().total(total);
        }
        return S_OK;
    }

    S7_STDMETHOD(SetCompleted, const UInt64 *completeValue) {
        return handler.callbacks().progress ? handler.callbacks().progress(*completeValue) : S_OK;
    }

    S7_STDMETHOD(SetRatioInfo, const UInt64 *inSize, const UInt64 *outSize) {
        if (inSize != nullptr && outSize != nullptr && handler.callbacks().ratio)
            handler.callbacks().ratio(*inSize, *outSize);
        return S_OK;
    }

    S7_STDMETHOD(GetStream, UInt32 index, ISequentialOutStream **outStream, Int32 askExtractMode) {
        COM_TRY_BEGIN
        *outStream = nullptr;
        sequential_out_stream.Release();

        {
            // Get Name
            NWindows::NCOM::CPropVariant prop;
            RINOK(in_archive->GetProperty(index, kpidPath, &prop));

            if (prop.vt == VT_EMPTY)
                this->path_in_archive = g_empty_file_alias;
            else {
                if (prop.vt != VT_BSTR)
                    return E_FAIL;
                this->path_in_archive = prop.bstrVal;
            }
        }

        if (askExtractMode != NArchive::NExtract::NAskMode::kExtract)
            return S_OK;

        {
            // Get Attrib
            NWindows::NCOM::CPropVariant prop;
            RINOK(in_archive->GetProperty(index, kpidAttrib, &prop))
            if (prop.vt == VT_EMPTY) {
                this->processed_file_info.attrib = 0;
                this->processed_file_info.is_attrib_defined = false;
            }
            else {
                if (prop.vt != VT_UI4)
                    return E_FAIL;
                this->processed_file_info.attrib = prop.ulVal;
                this->processed_file_info.is_attrib_defined = true;
            }
        }

        RINOK(check_bool_archive_item_prop(in_archive, index, kpidIsDir, this->processed_file_info.is_dir))

        {
            // Get Modified Time
            this->processed_file_info.arhive_mtime.clear();
            NWindows::NCOM::CPropVariant prop;
            RINOK(in_archive->GetProperty(index, kpidMTime, &prop))
            switch (prop.vt) {
                case VT_EMPTY:
                    break;
                case VT_FILETIME:
                    this->processed_file_info.arhive_mtime.set_from_prop(prop);
                    break;
                default:
                    return E_FAIL;
            }
        }
        {
            // Get Size
            NWindows::NCOM::CPropVariant prop;
            RINOK(in_archive->GetProperty(index, kpidSize, &prop))
            try {
                UInt64 new_file_size;
                /* bool newFileSizeDefined = */ ConvertPropVariantToUInt64(prop, new_file_size);
            }
            catch (const std::exception &e) {
                // std::cout << "Error: " << e.what() << std::endl;
            }
        }

        {
            // Create folders for file
            int slash_pos = path_in_archive.ReverseFind_PathSepar();
            if (slash_pos >= 0)
                NWindows::NFile::NDir::CreateComplexDir(extract_out_dir + us2fs(path_in_archive.Left(slash_pos)));
        }

        FString full_processed_path = extract_out_dir + us2fs(path_in_archive);
        this->path_on_disk = full_processed_path;

        if (this->processed_file_info.is_dir) {
            NWindows::NFile::NDir::CreateComplexDir(full_processed_path);
        }
        else {
            // try to delete existing file
            NWindows::NFile::NFind::CFileInfo file_info;
            if (file_info.Find(full_processed_path)) {
                if (!NWindows::NFile::NDir::DeleteFileAlways(full_processed_path)) {
                    // PrintError("Cannot delete output file", fullProcessedPath);
                    return E_ABORT;
                }
            }

            out_file_stream_spec = new COutFileStream;
            CMyComPtr<ISequentialOutStream> extract_sequential_out_stream(out_file_stream_spec);
            if (!out_file_stream_spec->Open(full_processed_path, CREATE_ALWAYS)) {
                // PrintError("Cannot open output file", fullProcessedPath);
                return E_ABORT;
            }
            sequential_out_stream = extract_sequential_out_stream;
            *outStream = sequential_out_stream.Detach();
        }

        return S_OK;
        COM_TRY_END
    }

    S7_STDMETHOD(PrepareOperation, Int32 askExtractMode) {
        extract_mode = false;
        switch (askExtractMode) {
            case NArchive::NExtract::NAskMode::kExtract:
                extract_mode = true;
                break;
        }
        // TODO
        switch (askExtractMode) {
            case NArchive::NExtract::NAskMode::kExtract:
                // Extracting...
                break;
            case NArchive::NExtract::NAskMode::kTest:
                // Testing...
                break;
            case NArchive::NExtract::NAskMode::kSkip:
                // Skipping...
                break;
            case NArchive::NExtract::NAskMode::kReadExternal:
                // Reading...
                break;
            default:
                //("??? ");
                break;
        }
        return S_OK;
    }

    S7_STDMETHOD(SetOperationResult, Int32 opRes) {
        switch (opRes) {
            case NArchive::NExtract::NOperationResult::kOK:
                break;
            default: {
                num_errors++;
                const char *s = nullptr;
                switch (opRes) {
                    case NArchive::NExtract::NOperationResult::kUnsupportedMethod:
                        s = "Unsupported Method";
                        break;
                    case NArchive::NExtract::NOperationResult::kCRCError:
                        s = "CRC Failed";
                        break;
                    case NArchive::NExtract::NOperationResult::kDataError:
                        s = "Data Error";
                        break;
                    case NArchive::NExtract::NOperationResult::kUnavailable:
                        s = "Unavailable data";
                        break;
                    case NArchive::NExtract::NOperationResult::kUnexpectedEnd:
                        s = "Unexpected end of data";
                        break;
                    case NArchive::NExtract::NOperationResult::kDataAfterEnd:
                        s = "There are some data after the end of the payload data";
                        break;
                    case NArchive::NExtract::NOperationResult::kIsNotArc:
                        s = "Is not archive";
                        break;
                    case NArchive::NExtract::NOperationResult::kHeadersError:
                        s = "Headers Error";
                        break;
                }
                if (s) {
                    // std::cout << "Error: " << s << std::endl;
                }
                else {
                    char temp[16];
                    ConvertUInt32ToString(static_cast<UInt32>(opRes), temp);
                    // std::cout << "Error: " << temp << std::endl;
                }
            }
        }
        // Set file time and attributes, then close stream
        if (sequential_out_stream) {
            if (this->processed_file_info.arhive_mtime.def) {
                CFiTime ft;
                this->processed_file_info.arhive_mtime.write_to_filetime(ft);
                out_file_stream_spec->SetMTime(&ft);
            }
            RINOK(out_file_stream_spec->Close())
        }
        sequential_out_stream.Release();
        if (extract_mode && this->processed_file_info.is_attrib_defined)
            NWindows::NFile::NDir::SetFileAttrib_PosixHighDetect(path_on_disk, this->processed_file_info.attrib);
        return S_OK;
    }

    S7_STDMETHOD(CryptoGetTextPassword, BSTR *password) {
        if (!this->passwd.IsEmpty())
            return StringToBstr(this->passwd, password);
        return S_OK;
    }

    CMyComPtr<IInArchive> in_archive;
    FString extract_out_dir;
    UString path_in_archive;
    FString path_on_disk;
    bool extract_mode{false};
    struct {
        archive_item arhive_mtime;
        UInt32 attrib{0};
        bool is_dir{false};
        bool is_attrib_defined{false};
    } processed_file_info;

    COutFileStream *out_file_stream_spec{nullptr};
    CMyComPtr<ISequentialOutStream> sequential_out_stream;
    UInt64 num_errors{0};
    UString passwd;
    archive_handler &handler;
};

class archive_open_handler : public archive_handler {
public:
    archive_open_handler(handler_callbacks _callbacks = {}) : archive_handler(_callbacks) {}

protected:
    format::format format_;
};

} // namespace smooth7zip
