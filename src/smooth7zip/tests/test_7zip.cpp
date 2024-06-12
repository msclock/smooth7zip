#include <7zip/7zip.h>
#include <gtest/gtest.h>

#include <iomanip>
#include <sstream>

#include <7zip/CPP/7zip/Archive/HandlerCont.h>
#include <7zip/CPP/7zip/Common/FileStreams.h>
#include <7zip/CPP/Common/ComTry.h>
#include <7zip/CPP/Common/IntToString.h>
#include <7zip/CPP/Common/MyCom.h>
#include <7zip/CPP/Common/MyString.h>
#include <7zip/CPP/Common/MyVector.h>
#include <7zip/CPP/Windows/FileFind.h>
#include <7zip/CPP/Windows/FileIO.h>

#include "7zip/CPP/Common/StringConvert.h"
#include "7zip/CPP/Windows/FileDir.h"
#include "7zip/CPP/Windows/FileName.h"
#include "7zip/CPP/Windows/PropVariantConv.h"

#include "smooth7zip/smooth7zip.hpp"

#include "utils.hpp"

// Format IDs
constexpr auto id_format_zip = GUID{0x23170F69, 0x40C1, 0x278A, {0x10, 0x00, 0x00, 0x01, 0x10, 0x01, 0x00, 0x00}};

TEST(verify_7zip, exported_interfaces) {
    // CreateObject
    CMyComPtr<IInArchive> in_zip;
    EXPECT_EQ(CreateObject(&id_format_zip, &IID_IInArchive, reinterpret_cast<void **>(&in_zip)), S_OK);

    CMyComPtr<IOutArchive> out_zip;
    EXPECT_EQ(CreateObject(&id_format_zip, &IID_IOutArchive, reinterpret_cast<void **>(&out_zip)), S_OK);

    // GetNumberOfFormats
    uint32_t num_of_formats;
    EXPECT_EQ(GetNumberOfFormats(&num_of_formats), S_OK);
    EXPECT_EQ(num_of_formats, 55); // Default to 55 formats.

    // GetHandlerProperty2
    for (size_t i = 0; i < num_of_formats; ++i) {
        NWindows::NCOM::CPropVariant prop{};
        EXPECT_EQ(GetHandlerProperty2(static_cast<UInt32>(i), NArchive::NHandlerPropID::kName, &prop), S_OK);
        auto name = UnicodeStringToMultiByte(prop.bstrVal);
        prop.Clear();
        EXPECT_EQ(GetHandlerProperty2(static_cast<UInt32>(i), NArchive::NHandlerPropID::kClassID, &prop), S_OK);
        auto handler_id = *reinterpret_cast<const GUID *>(prop.bstrVal);
        prop.Clear();
        EXPECT_EQ(GetHandlerProperty2(static_cast<UInt32>(i), NArchive::NHandlerPropID::kExtension, &prop), S_OK);
        auto ext = UnicodeStringToMultiByte(prop.bstrVal);
        prop.Clear();
        EXPECT_EQ(GetHandlerProperty2(static_cast<UInt32>(i), NArchive::NHandlerPropID::kAddExtension, &prop), S_OK);
        auto add_ext = prop.bstrVal ? UnicodeStringToMultiByte(prop.bstrVal) : AString{};
        prop.Clear();
        EXPECT_EQ(GetHandlerProperty2(static_cast<UInt32>(i), NArchive::NHandlerPropID::kUpdate, &prop), S_OK);
        bool update = prop.boolVal;
        prop.Clear();
        EXPECT_EQ(GetHandlerProperty2(static_cast<UInt32>(i), NArchive::NHandlerPropID::kFlags, &prop), S_OK);
        auto flags = prop.ulVal;
        prop.Clear();
        EXPECT_EQ(GetHandlerProperty2(static_cast<UInt32>(i), NArchive::NHandlerPropID::kTimeFlags, &prop), S_OK);
        auto time_flags = prop.ulVal;
        prop.Clear();
        EXPECT_EQ(GetHandlerProperty2(static_cast<UInt32>(i), NArchive::NHandlerPropID::kSignature, &prop), S_OK);
        auto sig_len = ::SysStringByteLen(prop.bstrVal);
        std::vector<Byte> signature_buffer(sig_len);
        if (prop.bstrVal) {
            auto *beg = reinterpret_cast<Byte *>(prop.bstrVal);
            signature_buffer.assign(beg, beg + sig_len);
        }
        auto signature_stream = test::utils::hex_to_str(reinterpret_cast<const char *>(signature_buffer.data()),
                                                        signature_buffer.size(),
                                                        2);
        prop.Clear();
        EXPECT_EQ(GetHandlerProperty2(static_cast<UInt32>(i), NArchive::NHandlerPropID::kMultiSignature, &prop), S_OK);
        auto multi_sig_len = ::SysStringByteLen(prop.bstrVal);
        std::vector<Byte> multi_signature_buffer(multi_sig_len);
        if (prop.bstrVal) {
            auto *beg = reinterpret_cast<Byte *>(prop.bstrVal);
            multi_signature_buffer.assign(beg, beg + multi_sig_len);
        }
        std::vector<std::vector<Byte>> multi_signatures{};
        std::ostringstream multi_signature_stream;
        multi_signature_stream << std::hex << std::setfill('0') << std::setw(2);
        auto size = multi_signature_buffer.size();
        auto *data = multi_signature_buffer.data();
        while (size != 0) {
            const unsigned len = *data++;
            size--;
            if (len > size)
                break;

            multi_signatures.emplace_back(data, data + len);
            multi_signature_stream << test::utils::hex_to_str(reinterpret_cast<const char *>(data), len, 2) << ";";
            data += len;
            size -= len;
        }
        prop.Clear();
        EXPECT_EQ(GetHandlerProperty2(static_cast<UInt32>(i), NArchive::NHandlerPropID::kSignatureOffset, &prop), S_OK);
        auto signature_offset = prop.ulVal;
        Func_IsArc IsArc = nullptr;
        GetIsArc(static_cast<UInt32>(i), &IsArc);
        GTEST_LOG_(INFO) << "Format: " << i << ", name: " << name
                         << ", handler_id: " << test::utils::to_string(handler_id) << ", ext: " << ext
                         << ", add_ext: " << add_ext << ", update: " << update << ", flags: " << flags
                         << ", time_flags: " << time_flags << ", signature: " << signature_stream
                         << ", multi_signature: " << multi_signature_stream.str()
                         << ", signature_offset: " << signature_offset << ", IsArc: " << (IsArc ? "Yes" : "No");
    }

    // GetNumberOfMethods
    uint32_t num_of_methods;
    EXPECT_EQ(GetNumberOfMethods(&num_of_methods), S_OK);
    EXPECT_EQ(num_of_methods, 24); // Default to 24 methods

    // GetMethodProperty
    for (size_t i = 0; i < num_of_methods; ++i) {
        NWindows::NCOM::CPropVariant prop{};
        EXPECT_EQ(GetMethodProperty(static_cast<UInt32>(i), NMethodPropID::kName, &prop), S_OK);
        auto name = UnicodeStringToMultiByte(prop.bstrVal);
        prop.Clear();
        EXPECT_EQ(GetMethodProperty(static_cast<UInt32>(i), NMethodPropID::kID, &prop), S_OK);
        auto id = prop.ulVal;
        prop.Clear();
        EXPECT_EQ(GetMethodProperty(static_cast<UInt32>(i), NMethodPropID::kDecoder, &prop), S_OK);
        auto decoder_id = prop.bstrVal ? *reinterpret_cast<const GUID *>(prop.bstrVal) : GUID{};
        prop.Clear();
        EXPECT_EQ(GetMethodProperty(static_cast<UInt32>(i), NMethodPropID::kEncoder, &prop), S_OK);
        auto encoder_id = prop.bstrVal ? *reinterpret_cast<const GUID *>(prop.bstrVal) : GUID{};
        prop.Clear();
        EXPECT_EQ(GetMethodProperty(static_cast<UInt32>(i), NMethodPropID::kIsFilter, &prop), S_OK);
        bool is_filter = prop.boolVal;
        GTEST_LOG_(INFO) << "Method: " << i << ", name: " << name << ", ID: " << id
                         << ", decoder_id: " << test::utils::to_string(decoder_id)
                         << ", encoder_id: " << test::utils::to_string(encoder_id) << ", is_filter: " << is_filter;
    }

    // GetHashers
    uint32_t num_of_hashers;
    CMyComPtr<IHashers> com_hashers;
    EXPECT_EQ(GetHashers(&com_hashers), S_OK);
    num_of_hashers = com_hashers->GetNumHashers();
    EXPECT_EQ(num_of_hashers, 5); // Default to 5 hashers
}

class archive_update_callback Z7_final : public IArchiveUpdateCallback2,
                                         public ICryptoGetTextPassword2,
                                         public CMyUnknownImp {
    Z7_IFACE_COM7_IMP(IProgress)                                          // cppcheck-suppress missingOverride
    Z7_IFACE_COM7_IMP(IArchiveUpdateCallback)                             // cppcheck-suppress missingOverride
    Z7_IFACES_IMP_UNK_2(IArchiveUpdateCallback2, ICryptoGetTextPassword2) // cppcheck-suppress missingOverride

public:
    const CObjectVector<NWindows::NFile::NFind::CFileInfo> *dir_items{nullptr};
    bool is_password_is_defined{false};
    bool is_ask_password{false};
    bool is_need_be_closed{false};
    UString volume_name;
    UString volume_ext;
    UString passwd;
    FString path_prefix;
    FStringVector failed_files;
    CRecordVector<HRESULT> failed_codes;
    CRecordVector<UInt64> voulmes_sizes;

    archive_update_callback() = default;

    HRESULT finalize() {
        is_need_be_closed = false;
        return S_OK;
    }
};

Z7_COM7F_IMF(archive_update_callback::SetTotal(UInt64 /* size */)) {
    return S_OK;
}

Z7_COM7F_IMF(archive_update_callback::SetCompleted(const UInt64 * /* completeValue */)) {
    return S_OK;
}

Z7_COM7F_IMF(
    archive_update_callback::GetUpdateItemInfo(UInt32 index, Int32 *newData, Int32 *newProps, UInt32 *indexInArchive)) {
    (void)index;
    if (newData) {
        *newData = static_cast<Int32>(true);
    }
    if (newProps) {
        *newProps = static_cast<Int32>(true);
    }

    if (indexInArchive) {
        *indexInArchive = static_cast<UInt32>(-1);
    }
    return S_OK;
}

Z7_COM7F_IMF(archive_update_callback::GetProperty(UInt32 index, PROPID propID, PROPVARIANT *value)) {
    NWindows::NCOM::CPropVariant prop;
    if (propID == kpidIsAnti) {
        prop = false;
        prop.Detach(value);
        return S_OK;
    }

    {
        const NWindows::NFile::NFind::CFileInfo &fi = (*dir_items)[index];
        switch (propID) {
            case kpidPath:
                prop = fs2us(fi.Name);
                break;
            case kpidIsDir:
                prop = fi.IsDir();
                break;
            case kpidSize:
                prop = fi.Size;
                break;
            case kpidCTime:
                PropVariant_SetFrom_FiTime(prop, fi.CTime);
                break;
            case kpidATime:
                PropVariant_SetFrom_FiTime(prop, fi.ATime);
                break;
            case kpidMTime:
                PropVariant_SetFrom_FiTime(prop, fi.MTime);
                break;
            case kpidAttrib:
                prop = fi.GetWinAttrib();
                break;
            case kpidPosixAttrib:
                prop = fi.GetPosixAttrib();
                break;
            default:
                break;
        }
    }
    prop.Detach(value);
    return S_OK;
}

Z7_COM7F_IMF(archive_update_callback::GetStream(UInt32 index, ISequentialInStream **inStream)) {
    COM_TRY_BEGIN
    RINOK(finalize());

    const NWindows::NFile::NFind::CFileInfo &fi = (*dir_items)[index];

    if (fi.IsDir())
        return S_OK;

    {
        CInFileStream *in_file_stream_spec = new CInFileStream;
        CMyComPtr<ISequentialInStream> sequential_in_stream(in_file_stream_spec);

        FString update_path_prefix = this->path_prefix;
        if (update_path_prefix.Back() != CHAR_PATH_SEPARATOR)
            update_path_prefix.Add_PathSepar();

        if (!in_file_stream_spec->Open(update_path_prefix + fi.Name)) {
            const auto last_sys_error = ::GetLastError();
            try {
                failed_codes.Add(HRESULT_FROM_WIN32(last_sys_error));
                failed_files.Add(update_path_prefix + fi.Name);
            }
            catch (const std::exception &e) {
                std::cerr << e.what() << '\n';
            }
            return S_FALSE;
        }
        *inStream = sequential_in_stream.Detach();
    }

    return S_OK;
    COM_TRY_END
}

Z7_COM7F_IMF(archive_update_callback::SetOperationResult(Int32 operationResult)) {
    (void)operationResult;
    is_need_be_closed = true;
    return S_OK;
}

Z7_COM7F_IMF(archive_update_callback::GetVolumeSize(UInt32 index, UInt64 *size)) {
    if (voulmes_sizes.Size() == 0)
        return S_FALSE;
    if (index >= voulmes_sizes.Size())
        index = voulmes_sizes.Size() - 1;
    *size = voulmes_sizes[index];
    return S_OK;
}

Z7_COM7F_IMF(archive_update_callback::GetVolumeStream(UInt32 index, ISequentialOutStream **volumeStream)) {
    COM_TRY_BEGIN
    wchar_t temp[16]{};
    ConvertUInt32ToString(index + 1, temp);
    UString res = temp;
    while (res.Len() < 2) {
        res.InsertAtFront(L'0');
    }
    UString volume_file_name = volume_name;
    volume_file_name.Add_Dot();
    volume_file_name += res;
    volume_file_name += volume_ext;
    COutFileStream *volume_out_file_stream_spec = new COutFileStream;
    CMyComPtr<ISequentialOutStream> volume_sequential_out_stream(volume_out_file_stream_spec);
    if (volume_out_file_stream_spec->Create(us2fs(volume_file_name), false)) {
        return GetLastError_noZero_HRESULT();
    }
    *volumeStream = volume_sequential_out_stream.Detach();
    return S_OK;
    COM_TRY_END
}

Z7_COM7F_IMF(archive_update_callback::CryptoGetTextPassword2(Int32 *passwordIsDefined, BSTR *password)) {
    if (!is_password_is_defined) {
        if (is_ask_password) {
            return E_ABORT;
        }
    }
    *passwordIsDefined = static_cast<Int32>(is_password_is_defined);
    return StringToBstr(this->passwd, password);
}

using verify_7zip_command = test::utils::rc_dir_test;

// Create a zip archive from the current directory
TEST_F(verify_7zip_command, a) {
    NWindows::NFile::NFind::CFileInfo fi;
    CObjectVector<NWindows::NFile::NFind::CFileInfo> to_archive_items;

    auto to_archive_path = std::filesystem::directory_entry(std::filesystem::current_path());
    for (const auto &to_archive_entry : std::filesystem::directory_iterator(to_archive_path)) {
        if (to_archive_entry.is_regular_file()) {
            ASSERT_TRUE(fi.Find(to_archive_entry.path().c_str()));
            to_archive_items.Add(fi);
        }
    }

    // Update callback
    archive_update_callback *update_callback_spec = new archive_update_callback();
    update_callback_spec->dir_items = &to_archive_items;
    update_callback_spec->path_prefix = FString(to_archive_path.path().c_str());
    CMyComPtr<IArchiveUpdateCallback2> update_callback(update_callback_spec);

    // Out Stream
    const auto archive_path = this->test_system_tmp_dir_ / "temp.zip";
    auto archive_name = FString(archive_path.c_str());
    auto *out_file_stream_spec = new COutFileStream;
    ASSERT_TRUE(out_file_stream_spec->Create(archive_name, True));
    CMyComPtr<IOutStream> out_stream(out_file_stream_spec);

    // Out Archive
    CMyComPtr<IOutArchive> out_archive;
    EXPECT_EQ(CreateObject(&id_format_zip, &IID_IOutArchive, reinterpret_cast<void **>(&out_archive)), S_OK);

    // Set properties
    CMyComPtr<ISetProperties> set_properties;
    EXPECT_EQ(out_archive->QueryInterface(IID_ISetProperties, reinterpret_cast<void **>(&set_properties)), S_OK);
    ASSERT_TRUE(set_properties);

    const uint8_t num_prop = 1;
    const wchar_t *names[num_prop] = {L"x"}; // Compress level 9(utral)
    NWindows::NCOM::CPropVariant values[num_prop] = {static_cast<UINT32>(9)};
    EXPECT_EQ(set_properties->SetProperties(names, values, num_prop), S_OK);

    // Update items in archive
    EXPECT_EQ(out_archive->UpdateItems(out_stream, to_archive_items.Size(), update_callback), S_OK);
    EXPECT_EQ(S_OK, update_callback_spec->finalize());
    EXPECT_TRUE(std::filesystem::exists(archive_path));
}

class archive_open_callback Z7_final : public IArchiveOpenCallback,
                                       public ICryptoGetTextPassword,
                                       public CMyUnknownImp {
    Z7_IFACES_IMP_UNK_2(IArchiveOpenCallback, ICryptoGetTextPassword) // cppcheck-suppress missingOverride
public:
    bool password_is_defined{false};
    UString passwd;
};

Z7_COM7F_IMF(archive_open_callback::SetTotal(const UInt64 * /* files */, const UInt64 * /* bytes */)) {
    return S_OK;
}

Z7_COM7F_IMF(archive_open_callback::SetCompleted(const UInt64 * /* files */, const UInt64 * /* bytes */)) {
    return S_OK;
}

Z7_COM7F_IMF(archive_open_callback::CryptoGetTextPassword(BSTR *password)) {
    if (!password_is_defined) {
        // You can ask real password here from user
        // Password = GetPassword(OutStream);
        // PasswordIsDefined = true;
        // ("Password is not defined");
        return E_ABORT;
    }
    return StringToBstr(this->passwd, password);
}

TEST_F(verify_7zip_command, l) {
    // In Archive
    CMyComPtr<IInArchive> in_archive;
    EXPECT_EQ(CreateObject(&id_format_zip, &IID_IInArchive, reinterpret_cast<void **>(&in_archive)), S_OK);

    // In Stream
    auto regualr_zip = this->test_data_dir_ / "regular.zip";
    GTEST_LOG_(INFO) << "regualr_zip: " << regualr_zip;
    auto list_file_name = FString(regualr_zip.c_str());
    CInFileStream *in_file_stream_spec = new CInFileStream;

    EXPECT_TRUE(in_file_stream_spec->Open(list_file_name));
    CMyComPtr<IInStream> in_stream(in_file_stream_spec);

    {
        archive_open_callback *open_callback_spec = new archive_open_callback;
        open_callback_spec->password_is_defined = false;
        open_callback_spec->passwd = "";
        CMyComPtr<IArchiveOpenCallback> open_callback(open_callback_spec);

        const UInt64 scan_size = 1 << 23;
        EXPECT_EQ(in_archive->Open(in_stream, &scan_size, open_callback), S_OK);
    }

    UInt32 num_of_items = 0;
    in_archive->GetNumberOfItems(&num_of_items);
    for (UInt32 i = 0; i < num_of_items; i++) {
        {
            // Get uncompressed size of file
            NWindows::NCOM::CPropVariant prop;
            in_archive->GetProperty(i, kpidSize, &prop);
            char s[32];
            ConvertPropVariantToShortString(prop, s);
            // GTEST_LOG_(INFO) << "Size: " << s;
        }
        {
            // Get name of file
            NWindows::NCOM::CPropVariant prop;
            in_archive->GetProperty(i, kpidPath, &prop);
            ASSERT_TRUE(prop.bstrVal);
        }
    }
}

struct archive_time {
    FILETIME filetime{0, 0};
    UInt16 precision;
    Byte ns100;
    bool def;

    archive_time() { clear(); }

    void clear() {
        filetime.dwHighDateTime = filetime.dwLowDateTime = 0;
        precision = 0;
        ns100 = 0;
        def = false;
    }

    bool is_zero() const { return filetime.dwLowDateTime == 0 && filetime.dwHighDateTime == 0 && ns100 == 0; }

    int get_num_digits() const {
        if (precision == k_PropVar_TimePrec_Unix || precision == k_PropVar_TimePrec_DOS)
            return 0;
        if (precision == k_PropVar_TimePrec_HighPrec)
            return 9;
        if (precision == k_PropVar_TimePrec_0)
            return 7;
        int digits = static_cast<int>(precision) - k_PropVar_TimePrec_Base;
        if (digits < 0)
            digits = 0;
        return digits;
    }

    void write_to_filetime(CFiTime &dest) const {
#ifdef _WIN32
        dest = filetime;
#else
        if (FILETIME_To_timespec(filetime, dest))
            if ((precision == k_PropVar_TimePrec_Base + 8 || precision == k_PropVar_TimePrec_Base + 9) && ns100 != 0) {
                dest.tv_nsec += ns100;
            }
#endif
    }

    void set_from_prop(const PROPVARIANT &prop) {
        filetime = prop.filetime;
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
        precision = static_cast<UInt16>(prec);
        ns100 = static_cast<Byte>(ns);
        def = true;
    }
};

class archive_extract_callback Z7_final : public IArchiveExtractCallback,
                                          public ICryptoGetTextPassword,
                                          public CMyUnknownImp {
    Z7_IFACES_IMP_UNK_2(IArchiveExtractCallback, ICryptoGetTextPassword) // cppcheck-suppress missingOverride
    Z7_IFACE_COM7_IMP(IProgress)                                         // cppcheck-suppress missingOverride

public:
    archive_extract_callback() = default;

    void Init(IInArchive *_archive_handler, const FString &_extract_dir) {
        in_archive = _archive_handler;
        extract_dir = _extract_dir;
        NWindows::NFile::NName::NormalizeDirPathPrefix(extract_dir);
    }

    CMyComPtr<IInArchive> in_archive;
    FString extract_dir;     // Output directory
    UString path_in_archive; // name inside arcvhive
    FString path_on_disk;    // full path to file on disk
    bool extract_mode{false};
    struct {
        archive_time arhive_mtime;
        UInt32 attrib{0};
        bool is_dir{false};
        bool is_attrib_defined{false};
    } processed_file_info;

    COutFileStream *out_file_stream_spec{nullptr};
    CMyComPtr<ISequentialOutStream> sequential_out_stream;
    UInt64 num_errors{0};
    bool is_password_defined{false};
    UString passwd;
};

Z7_COM7F_IMF(archive_extract_callback::SetTotal(UInt64 /* size */)) {
    return S_OK;
}

Z7_COM7F_IMF(archive_extract_callback::SetCompleted(const UInt64 * /* completeValue */)) {
    return S_OK;
}

static const wchar_t *const gs_empty_file_alias = L"[Content]";

static HRESULT IsArchiveItemProp(IInArchive *archive, UInt32 index, PROPID propID, bool &result) {
    NWindows::NCOM::CPropVariant prop;
    RINOK(archive->GetProperty(index, propID, &prop))
    if (prop.vt == VT_BOOL)
        result = VARIANT_BOOLToBool(prop.boolVal);
    else if (prop.vt == VT_EMPTY)
        result = false;
    else
        return E_FAIL;
    return S_OK;
}

Z7_COM7F_IMF(archive_extract_callback::GetStream(UInt32 index,
                                                 ISequentialOutStream **outStream,
                                                 Int32 askExtractMode)) {
    COM_TRY_BEGIN
    *outStream = nullptr;
    sequential_out_stream.Release();

    {
        // Get Name
        NWindows::NCOM::CPropVariant prop;
        RINOK(in_archive->GetProperty(index, kpidPath, &prop))

        if (prop.vt == VT_EMPTY)
            this->path_in_archive = gs_empty_file_alias;
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

    RINOK(IsArchiveItemProp(in_archive, index, kpidIsDir, this->processed_file_info.is_dir))

    {
        this->processed_file_info.arhive_mtime.clear();
        // Get Modified Time
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
            std::cerr << e.what() << '\n';
        }
    }

    {
        // Create folders for file
        int slash_pos = path_in_archive.ReverseFind_PathSepar();
        if (slash_pos >= 0)
            NWindows::NFile::NDir::CreateComplexDir(extract_dir + us2fs(path_in_archive.Left(slash_pos)));
    }

    FString full_processed_path = extract_dir + us2fs(path_in_archive);
    this->path_on_disk = full_processed_path;

    if (this->processed_file_info.is_dir) {
        NWindows::NFile::NDir::CreateComplexDir(full_processed_path);
    }
    else {
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

Z7_COM7F_IMF(archive_extract_callback::PrepareOperation(Int32 askExtractMode)) {
    extract_mode = false;
    switch (askExtractMode) {
        case NArchive::NExtract::NAskMode::kExtract:
            extract_mode = true;
            break;
    }
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

Z7_COM7F_IMF(archive_extract_callback::SetOperationResult(Int32 opRes)) {
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

Z7_COM7F_IMF(archive_extract_callback::CryptoGetTextPassword(BSTR *password)) {
    if (!is_password_defined) {
        // You can ask real password here from user
        // Password = GetPassword(OutStream);
        // PasswordIsDefined = true;
        // PrintError("Password is not defined");
        return E_ABORT;
    }
    return StringToBstr(this->passwd, password);
}

TEST_F(verify_7zip_command, x) {
    // In Archive
    CMyComPtr<IInArchive> in_archive;
    EXPECT_EQ(CreateObject(&id_format_zip, &IID_IInArchive, reinterpret_cast<void **>(&in_archive)), S_OK);

    // In Stream
    auto regualr_zip = this->test_data_dir_ / "regular.zip";
    const FString archive_name = FString(regualr_zip.c_str());
    CInFileStream *in_file_stream_spec = new CInFileStream;
    CMyComPtr<IInStream> in_stream(in_file_stream_spec);
    EXPECT_TRUE(in_file_stream_spec->Open(archive_name));

    {
        archive_open_callback *open_callback_spec = new archive_open_callback;
        CMyComPtr<IArchiveOpenCallback> open_callback(open_callback_spec);
        open_callback_spec->password_is_defined = false;
        open_callback_spec->passwd = "";

        const UInt64 scan_size = 1 << 23;

        EXPECT_EQ(in_archive->Open(in_stream, &scan_size, open_callback), S_OK);
    }

    // Extract command
    archive_extract_callback *extract_callback_spec = new archive_extract_callback;
    CMyComPtr<IArchiveExtractCallback> extract_callback(extract_callback_spec);
    auto extract_dir = FString(this->test_system_tmp_dir_.c_str());
    extract_callback_spec->Init(in_archive, extract_dir);

    const wchar_t *names[] = {L"mt"};
    const unsigned num_props = sizeof(names) / sizeof(names[0]);
    NWindows::NCOM::CPropVariant values[num_props] = {static_cast<UInt32>(1)};
    CMyComPtr<ISetProperties> set_properties;
    in_archive->QueryInterface(IID_ISetProperties, reinterpret_cast<void **>(&set_properties));
    EXPECT_EQ(set_properties->SetProperties(names, values, num_props), S_OK);

    EXPECT_EQ(in_archive->Extract(nullptr, static_cast<UInt32>(-1), false, extract_callback), S_OK);
}
