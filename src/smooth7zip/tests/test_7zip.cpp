#include <7zip/7zip.h>
#include <gtest/gtest.h>

#include <iomanip>
#include <sstream>

#include <7zip/CPP/7zip/Archive/HandlerCont.h>
#include <7zip/CPP/7zip/Common/FileStreams.h>
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
constexpr auto id_format_7z = GUID{0x23170F69, 0x40C1, 0x278A, {0x10, 0x00, 0x00, 0x01, 0x10, 0x07, 0x00, 0x00}};

TEST(verify_7zip, CreateObject) {
    CMyComPtr<IInArchive> ia_zip, ia_7z;
    EXPECT_EQ(CreateObject(&id_format_zip, &IID_IInArchive, reinterpret_cast<void **>(&ia_zip)), S_OK);
    EXPECT_EQ(CreateObject(&id_format_7z, &IID_IInArchive, reinterpret_cast<void **>(&ia_7z)), S_OK);

    CMyComPtr<IOutArchive> oa_7zip, oa_7z;
    EXPECT_EQ(CreateObject(&id_format_zip, &IID_IOutArchive, reinterpret_cast<void **>(&oa_7zip)), S_OK);
    EXPECT_EQ(CreateObject(&id_format_7z, &IID_IOutArchive, reinterpret_cast<void **>(&oa_7z)), S_OK);
}

TEST(verify_7zip, GetNumberOfFormats) {
    uint32_t number;
    EXPECT_EQ(GetNumberOfFormats(&number), S_OK);
    EXPECT_EQ(number, 55); // Default to 55 formats

    for (size_t i = 0; i < number; ++i) {
        NWindows::NCOM::CPropVariant prop{};
        EXPECT_EQ(GetHandlerProperty2(static_cast<UInt32>(i), NArchive::NHandlerPropID::kName, &prop), S_OK);
        auto name = us2fs(prop.bstrVal);
        prop.Clear();
        EXPECT_EQ(GetHandlerProperty2(static_cast<UInt32>(i), NArchive::NHandlerPropID::kClassID, &prop), S_OK);
        auto handler_id = *reinterpret_cast<const GUID *>(prop.bstrVal);
        prop.Clear();
        EXPECT_EQ(GetHandlerProperty2(static_cast<UInt32>(i), NArchive::NHandlerPropID::kExtension, &prop), S_OK);
        auto ext = us2fs(prop.bstrVal);
        prop.Clear();
        EXPECT_EQ(GetHandlerProperty2(static_cast<UInt32>(i), NArchive::NHandlerPropID::kAddExtension, &prop), S_OK);
        auto add_ext = prop.bstrVal ? us2fs(prop.bstrVal) : FString{};
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
            auto beg = reinterpret_cast<Byte *>(prop.bstrVal);
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
            auto beg = reinterpret_cast<Byte *>(prop.bstrVal);
            multi_signature_buffer.assign(beg, beg + multi_sig_len);
        }
        std::vector<std::vector<Byte>> multi_signatures{};
        std::ostringstream multi_signature_stream;
        multi_signature_stream << std::hex << std::setfill('0') << std::setw(2);
        auto size = multi_signature_buffer.size();
        auto data = multi_signature_buffer.data();
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
                         << ", signature_offset: " << signature_offset << ", IsArc: " << (IsArc ? "true" : "false");
    }
}

TEST(verify_7zip, GetNumberOfMethods) {
    uint32_t number;
    EXPECT_EQ(GetNumberOfMethods(&number), S_OK);
    EXPECT_EQ(number, 24); // Default to 24 methods

    for (size_t i = 0; i < number; ++i) {
        NWindows::NCOM::CPropVariant prop{};
        EXPECT_EQ(GetMethodProperty(static_cast<UInt32>(i), NMethodPropID::kName, &prop), S_OK);
        auto name = us2fs(prop.bstrVal);
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
}

TEST(verify_7zip, GetHashers) {
    uint32_t number;
    CMyComPtr<IHashers> com_hashers;
    EXPECT_EQ(GetHashers(&com_hashers), S_OK);
    number = com_hashers->GetNumHashers();
    EXPECT_EQ(number, 5); // Default to 5 hashers
}

class CArchiveUpdateCallback Z7_final : public IArchiveUpdateCallback2,
                                        public ICryptoGetTextPassword2,
                                        public CMyUnknownImp {
    Z7_IFACE_COM7_IMP(IProgress)                                          // cppcheck-suppress missingOverride
    Z7_IFACE_COM7_IMP(IArchiveUpdateCallback)                             // cppcheck-suppress missingOverride
    Z7_IFACES_IMP_UNK_2(IArchiveUpdateCallback2, ICryptoGetTextPassword2) // cppcheck-suppress missingOverride

public:
    const CObjectVector<NWindows::NFile::NFind::CFileInfo> *dirItems{nullptr};
    bool passwdIsDefined{false};
    bool askPassword{false};
    bool needBeClosed{false};
    UString volName;
    UString volExt;
    UString passwd;
    FString pathPrefix;
    FStringVector failedFiles;
    CRecordVector<HRESULT> failedCodes;
    CRecordVector<UInt64> volumesSizes;

    CArchiveUpdateCallback(const CObjectVector<NWindows::NFile::NFind::CFileInfo> *dirItmes, const FString path_prefix)
        : dirItems{dirItmes},
          pathPrefix{path_prefix} {
        if (path_prefix.Back() != CHAR_PATH_SEPARATOR)
            this->pathPrefix.Add_PathSepar();
    }

    virtual ~CArchiveUpdateCallback() = default;

    HRESULT finalize() {
        needBeClosed = false;
        return S_OK;
    }
};

Z7_COM7F_IMF(CArchiveUpdateCallback::SetTotal(UInt64)) {
    return S_OK;
}

Z7_COM7F_IMF(CArchiveUpdateCallback::SetCompleted(const UInt64 *)) {
    return S_OK;
}

Z7_COM7F_IMF(
    CArchiveUpdateCallback::GetUpdateItemInfo(UInt32 index, Int32 *newData, Int32 *newProps, UInt32 *indexInArchive)) {
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

Z7_COM7F_IMF(CArchiveUpdateCallback::GetProperty(UInt32 index, PROPID propID, PROPVARIANT *value)) {
    NWindows::NCOM::CPropVariant prop;
    if (propID == kpidIsAnti) {
        prop = false;
        prop.Detach(value);
        return S_OK;
    }

    {
        const NWindows::NFile::NFind::CFileInfo &fi = (*dirItems)[index];
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

Z7_COM7F_IMF(CArchiveUpdateCallback::GetStream(UInt32 index, ISequentialInStream **inStream)) {
    RINOK(finalize());

    const NWindows::NFile::NFind::CFileInfo &fi = (*dirItems)[index];

    if (fi.IsDir())
        return S_OK;

    {
        CInFileStream *inStreamSpec = new CInFileStream;
        CMyComPtr<ISequentialInStream> inSequentialInStream(inStreamSpec);

        if (!inStreamSpec->Open(pathPrefix + fi.Name)) {
            const auto sysError = ::GetLastError();
            try {
                failedCodes.Add(HRESULT_FROM_WIN32(sysError));
                failedFiles.Add(pathPrefix + fi.Name);
            }
            catch (const std::exception &e) {
                std::cerr << e.what() << '\n';
            }
            return S_FALSE;
        }
        *inStream = inSequentialInStream.Detach();
    }

    return S_OK;
}

Z7_COM7F_IMF(CArchiveUpdateCallback::SetOperationResult(Int32 operationResult)) {
    (void)operationResult;
    needBeClosed = true;
    return S_OK;
}

Z7_COM7F_IMF(CArchiveUpdateCallback::GetVolumeSize(UInt32 index, UInt64 *size)) {
    if (volumesSizes.Size() == 0)
        return S_FALSE;
    if (index >= volumesSizes.Size())
        index = volumesSizes.Size() - 1;
    *size = volumesSizes[index];
    return S_OK;
}

Z7_COM7F_IMF(CArchiveUpdateCallback::GetVolumeStream(UInt32 index, ISequentialOutStream **volumeStream)) {
    wchar_t temp[16]{};
    ConvertUInt32ToString(index + 1, temp);
    UString res = temp;
    while (res.Len() < 2) {
        res.InsertAtFront(L'0');
    }
    UString fileName = volName;
    fileName.Add_Dot();
    fileName += res;
    fileName += volExt;
    COutFileStream *streamSpec = new COutFileStream;
    CMyComPtr<ISequentialOutStream> outSequentialStream(streamSpec);
    if (streamSpec->Create(us2fs(fileName), false)) {
        return GetLastError_noZero_HRESULT();
    }
    *volumeStream = outSequentialStream.Detach();
    return S_OK;
}

Z7_COM7F_IMF(CArchiveUpdateCallback::CryptoGetTextPassword2(Int32 *passwordIsDefined, BSTR *password)) {
    if (!passwdIsDefined) {
        if (askPassword) {
            return E_ABORT;
        }
    }
    *passwordIsDefined = static_cast<Int32>(passwdIsDefined);
    return StringToBstr(passwd, password);
}

using verify_7zip_command = test::utils::rc_dir_test;

// Create a zip archive from the current directory
TEST_F(verify_7zip_command, a) {
    const auto archive_path = temp_dir / "temp.zip";
    NWindows::NFile::NFind::CFileInfo fi;
    CObjectVector<NWindows::NFile::NFind::CFileInfo> dir_items;

    auto current_path = std::filesystem::directory_entry(std::filesystem::current_path());
    for (auto &entry : std::filesystem::directory_iterator(current_path)) {
        if (entry.is_regular_file()) {
            ASSERT_TRUE(fi.Find(entry.path().c_str()));
            dir_items.Add(fi);
        }
    }

    // Update callback
    CArchiveUpdateCallback *updateCallbackSpec =
        new CArchiveUpdateCallback(&dir_items, FString(current_path.path().c_str()));
    CMyComPtr<IArchiveUpdateCallback2> updateCallback(updateCallbackSpec);

    // Out Stream
    auto archive_name = FString(archive_path.c_str());
    auto out_file_string_spec = new COutFileStream;
    ASSERT_TRUE(out_file_string_spec->Create(archive_name, True));
    CMyComPtr<IOutStream> out_file_stream(out_file_string_spec);

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
    EXPECT_EQ(out_archive->UpdateItems(out_file_stream, dir_items.Size(), updateCallback), S_OK);
    EXPECT_EQ(S_OK, updateCallbackSpec->finalize());
    EXPECT_TRUE(std::filesystem::exists(archive_path));
}

class CArchiveOpenCallback Z7_final : public IArchiveOpenCallback, public ICryptoGetTextPassword, public CMyUnknownImp {
    Z7_IFACES_IMP_UNK_2(IArchiveOpenCallback, ICryptoGetTextPassword) // cppcheck-suppress missingOverride
public:
    bool password_is_defined{false};
    UString passwd;

    virtual ~CArchiveOpenCallback() = default;
};

Z7_COM7F_IMF(CArchiveOpenCallback::SetTotal(const UInt64 * /* files */, const UInt64 * /* bytes */)) {
    return S_OK;
}

Z7_COM7F_IMF(CArchiveOpenCallback::SetCompleted(const UInt64 * /* files */, const UInt64 * /* bytes */)) {
    return S_OK;
}

Z7_COM7F_IMF(CArchiveOpenCallback::CryptoGetTextPassword(BSTR *password)) {
    if (!password_is_defined) {
        // You can ask real password here from user
        // Password = GetPassword(OutStream);
        // PasswordIsDefined = true;
        // ("Password is not defined");
        return E_ABORT;
    }
    return StringToBstr(passwd, password);
}

TEST_F(verify_7zip_command, l) {
    // In Archive
    CMyComPtr<IInArchive> in_archive;
    EXPECT_EQ(CreateObject(&id_format_zip, &IID_IInArchive, reinterpret_cast<void **>(&in_archive)), S_OK);

    // In Stream
    auto regualr_zip = get_test_data("test_data/regular.zip");
    GTEST_LOG_(INFO) << "regualr_zip: " << regualr_zip;
    auto archiveName = FString(regualr_zip.c_str());
    CInFileStream *fileSpec = new CInFileStream;
    CMyComPtr<IInStream> file = fileSpec;

    EXPECT_TRUE(fileSpec->Open(archiveName));

    {
        CArchiveOpenCallback *openCallbackSpec = new CArchiveOpenCallback;
        CMyComPtr<IArchiveOpenCallback> openCallback(openCallbackSpec);
        openCallbackSpec->password_is_defined = false;
        openCallbackSpec->passwd = "";

        const UInt64 scanSize = 1 << 23;
        EXPECT_EQ(in_archive->Open(file, &scanSize, openCallback), S_OK);
    }

    UInt32 numItems = 0;
    in_archive->GetNumberOfItems(&numItems);
    for (UInt32 i = 0; i < numItems; i++) {
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

struct CArcTime {
    FILETIME FT;
    UInt16 Prec;
    Byte Ns100;
    bool Def;

    CArcTime() { Clear(); }

    void Clear() {
        FT.dwHighDateTime = FT.dwLowDateTime = 0;
        Prec = 0;
        Ns100 = 0;
        Def = false;
    }

    bool IsZero() const { return FT.dwLowDateTime == 0 && FT.dwHighDateTime == 0 && Ns100 == 0; }

    int GetNumDigits() const {
        if (Prec == k_PropVar_TimePrec_Unix || Prec == k_PropVar_TimePrec_DOS)
            return 0;
        if (Prec == k_PropVar_TimePrec_HighPrec)
            return 9;
        if (Prec == k_PropVar_TimePrec_0)
            return 7;
        int digits = static_cast<int>(Prec) - k_PropVar_TimePrec_Base;
        if (digits < 0)
            digits = 0;
        return digits;
    }

    void Write_To_FiTime(CFiTime &dest) const {
#ifdef _WIN32
        dest = FT;
#else
        if (FILETIME_To_timespec(FT, dest))
            if ((Prec == k_PropVar_TimePrec_Base + 8 || Prec == k_PropVar_TimePrec_Base + 9) && Ns100 != 0) {
                dest.tv_nsec += Ns100;
            }
#endif
    }

    void Set_From_Prop(const PROPVARIANT &prop) {
        FT = prop.filetime;
        unsigned prec = 0;
        unsigned ns100 = 0;
        const unsigned prec_Temp = prop.wReserved1;
        if (prec_Temp != 0 && prec_Temp <= k_PropVar_TimePrec_1ns && prop.wReserved3 == 0) {
            const unsigned ns100_Temp = prop.wReserved2;
            if (ns100_Temp < 100) {
                ns100 = ns100_Temp;
                prec = prec_Temp;
            }
        }
        Prec = static_cast<UInt16>(prec);
        Ns100 = static_cast<Byte>(ns100);
        Def = true;
    }
};

class CArchiveExtractCallback Z7_final : public IArchiveExtractCallback,
                                         public ICryptoGetTextPassword,
                                         public CMyUnknownImp {
    Z7_IFACES_IMP_UNK_2(IArchiveExtractCallback, ICryptoGetTextPassword) // cppcheck-suppress missingOverride
    Z7_IFACE_COM7_IMP(IProgress)                                         // cppcheck-suppress missingOverride

    CMyComPtr<IInArchive> _archiveHandler;
    FString _directoryPath; // Output directory
    UString _filePath;      // name inside arcvhive
    FString _diskFilePath;  // full path to file on disk
    bool _extractMode{false};
    struct CProcessedFileInfo {
        CArcTime MTime;
        UInt32 Attrib{0};
        bool isDir{false};
        bool Attrib_Defined{false};
    } _processedFileInfo;

    COutFileStream *_outFileStreamSpec{nullptr};
    CMyComPtr<ISequentialOutStream> _outFileStream;

public:
    void Init(IInArchive *archiveHandler, const FString &directoryPath);

    UInt64 NumErrors{0};
    bool PasswordIsDefined{false};
    UString Password;
};

void CArchiveExtractCallback::Init(IInArchive *archiveHandler, const FString &directoryPath) {
    NumErrors = 0;
    _archiveHandler = archiveHandler;
    _directoryPath = directoryPath;
    NWindows::NFile::NName::NormalizeDirPathPrefix(_directoryPath);
}

Z7_COM7F_IMF(CArchiveExtractCallback::SetTotal(UInt64 /* size */)) {
    return S_OK;
}

Z7_COM7F_IMF(CArchiveExtractCallback::SetCompleted(const UInt64 * /* completeValue */)) {
    return S_OK;
}

static const wchar_t *const kEmptyFileAlias = L"[Content]";

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

Z7_COM7F_IMF(CArchiveExtractCallback::GetStream(UInt32 index, ISequentialOutStream **outStream, Int32 askExtractMode)) {
    *outStream = NULL;
    _outFileStream.Release();

    {
        // Get Name
        NWindows::NCOM::CPropVariant prop;
        RINOK(_archiveHandler->GetProperty(index, kpidPath, &prop))

        UString fullPath;
        if (prop.vt == VT_EMPTY)
            fullPath = kEmptyFileAlias;
        else {
            if (prop.vt != VT_BSTR)
                return E_FAIL;
            fullPath = prop.bstrVal;
        }
        _filePath = fullPath;
    }

    if (askExtractMode != NArchive::NExtract::NAskMode::kExtract)
        return S_OK;

    {
        // Get Attrib
        NWindows::NCOM::CPropVariant prop;
        RINOK(_archiveHandler->GetProperty(index, kpidAttrib, &prop))
        if (prop.vt == VT_EMPTY) {
            _processedFileInfo.Attrib = 0;
            _processedFileInfo.Attrib_Defined = false;
        }
        else {
            if (prop.vt != VT_UI4)
                return E_FAIL;
            _processedFileInfo.Attrib = prop.ulVal;
            _processedFileInfo.Attrib_Defined = true;
        }
    }

    RINOK(IsArchiveItemProp(_archiveHandler, index, kpidIsDir, _processedFileInfo.isDir))

    {
        _processedFileInfo.MTime.Clear();
        // Get Modified Time
        NWindows::NCOM::CPropVariant prop;
        RINOK(_archiveHandler->GetProperty(index, kpidMTime, &prop))
        switch (prop.vt) {
            case VT_EMPTY:
                // _processedFileInfo.MTime = _utcMTimeDefault;
                break;
            case VT_FILETIME:
                _processedFileInfo.MTime.Set_From_Prop(prop);
                break;
            default:
                return E_FAIL;
        }
    }
    {
        // Get Size
        NWindows::NCOM::CPropVariant prop;
        RINOK(_archiveHandler->GetProperty(index, kpidSize, &prop))
        try {
            UInt64 newFileSize;
            /* bool newFileSizeDefined = */ ConvertPropVariantToUInt64(prop, newFileSize);
        }
        catch (const std::exception &e) {
            std::cerr << e.what() << '\n';
        }
    }

    {
        // Create folders for file
        int slashPos = _filePath.ReverseFind_PathSepar();
        if (slashPos >= 0)
            NWindows::NFile::NDir::CreateComplexDir(_directoryPath + us2fs(_filePath.Left(slashPos)));
    }

    FString fullProcessedPath = _directoryPath + us2fs(_filePath);
    _diskFilePath = fullProcessedPath;

    if (_processedFileInfo.isDir) {
        NWindows::NFile::NDir::CreateComplexDir(fullProcessedPath);
    }
    else {
        NWindows::NFile::NFind::CFileInfo fi;
        if (fi.Find(fullProcessedPath)) {
            if (!NWindows::NFile::NDir::DeleteFileAlways(fullProcessedPath)) {
                // PrintError("Cannot delete output file", fullProcessedPath);
                return E_ABORT;
            }
        }

        _outFileStreamSpec = new COutFileStream;
        CMyComPtr<ISequentialOutStream> outStreamLoc(_outFileStreamSpec);
        if (!_outFileStreamSpec->Open(fullProcessedPath, CREATE_ALWAYS)) {
            // PrintError("Cannot open output file", fullProcessedPath);
            return E_ABORT;
        }
        _outFileStream = outStreamLoc;
        *outStream = outStreamLoc.Detach();
    }
    return S_OK;
}

Z7_COM7F_IMF(CArchiveExtractCallback::PrepareOperation(Int32 askExtractMode)) {
    _extractMode = false;
    switch (askExtractMode) {
        case NArchive::NExtract::NAskMode::kExtract:
            _extractMode = true;
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

Z7_COM7F_IMF(CArchiveExtractCallback::SetOperationResult(Int32 opRes)) {
    switch (opRes) {
        case NArchive::NExtract::NOperationResult::kOK:
            break;
        default: {
            NumErrors++;
            const char *s = NULL;
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

    if (_outFileStream) {
        if (_processedFileInfo.MTime.Def) {
            CFiTime ft;
            _processedFileInfo.MTime.Write_To_FiTime(ft);
            _outFileStreamSpec->SetMTime(&ft);
        }
        RINOK(_outFileStreamSpec->Close())
    }
    _outFileStream.Release();
    if (_extractMode && _processedFileInfo.Attrib_Defined)
        NWindows::NFile::NDir::SetFileAttrib_PosixHighDetect(_diskFilePath, _processedFileInfo.Attrib);
    return S_OK;
}

Z7_COM7F_IMF(CArchiveExtractCallback::CryptoGetTextPassword(BSTR *password)) {
    if (!PasswordIsDefined) {
        // You can ask real password here from user
        // Password = GetPassword(OutStream);
        // PasswordIsDefined = true;
        // PrintError("Password is not defined");
        return E_ABORT;
    }
    return StringToBstr(Password, password);
}

TEST_F(verify_7zip_command, x) {
    // In Archive
    CMyComPtr<IInArchive> in_archive;
    EXPECT_EQ(CreateObject(&id_format_zip, &IID_IInArchive, reinterpret_cast<void **>(&in_archive)), S_OK);

    // In Stream
    auto regualr_zip = get_test_data("test_data/regular.zip");
    const FString archiveName = us2fs(GetUnicodeString(regualr_zip.c_str()));
    CInFileStream *fileSpec = new CInFileStream;
    CMyComPtr<IInStream> file = fileSpec;

    EXPECT_TRUE(fileSpec->Open(archiveName));

    {
        CArchiveOpenCallback *openCallbackSpec = new CArchiveOpenCallback;
        CMyComPtr<IArchiveOpenCallback> openCallback(openCallbackSpec);
        openCallbackSpec->password_is_defined = false;
        openCallbackSpec->passwd = "";

        const UInt64 scanSize = 1 << 23;
        EXPECT_EQ(in_archive->Open(file, &scanSize, openCallback), S_OK);
    }

    // Extract command
    CArchiveExtractCallback *extractCallbackSpec = new CArchiveExtractCallback;
    CMyComPtr<IArchiveExtractCallback> extractCallback(extractCallbackSpec);
    extractCallbackSpec->Init(in_archive,
                              FString(temp_dir.c_str())); // second parameter is output folder path
    extractCallbackSpec->PasswordIsDefined = false;
    extractCallbackSpec->Password = "";

    const wchar_t *names[] = {L"mt"};
    const unsigned kNumProps = sizeof(names) / sizeof(names[0]);
    NWindows::NCOM::CPropVariant values[kNumProps] = {static_cast<UInt32>(1)};
    CMyComPtr<ISetProperties> setProperties;
    in_archive->QueryInterface(IID_ISetProperties, reinterpret_cast<void **>(&setProperties));
    EXPECT_EQ(setProperties->SetProperties(names, values, kNumProps), S_OK);

    EXPECT_EQ(in_archive->Extract(NULL, static_cast<UInt32>(-1), false, extractCallback), S_OK);
}
