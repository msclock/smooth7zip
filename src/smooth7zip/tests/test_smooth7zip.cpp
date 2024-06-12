#include <gtest/gtest.h>

#include <smooth7zip/smooth7zip.hpp>

#include <7zip/CPP/Windows/FileName.h>        // NormalizeDirPathPrefix
#include <7zip/CPP/Windows/PropVariantConv.h> // ConvertPropVariantToShortString

#include <stack>
#include "utils.hpp"

constexpr auto id_format_apm = GUID{0x23170F69, 0x40C1, 0x278A, {0x10, 0x00, 0x00, 0x01, 0x10, 0xD4, 0x00, 0x00}};

TEST(smooth7zip, format) {
    // Test to load the format registry correctly
    EXPECT_EQ(smooth7zip::format::format_registry::instance().count(), 55);

    // Test to get a format by name
    auto format = smooth7zip::format::format_registry::instance().get("APM");
    EXPECT_TRUE(format.is_valid());

    // Test to get a format by index
    auto apm_format_from_index = smooth7zip::format::format_registry::instance().get(format.index);
    EXPECT_EQ(apm_format_from_index.name, "APM");

    // Test iterate over all formats
    for (const auto &f : smooth7zip::format::format_registry::instance()) {
        EXPECT_TRUE(f.is_valid());
    }

    // Test get the GUID of a format
    auto format_id = smooth7zip::format::get_guid(format);
    EXPECT_EQ(format_id, id_format_apm);

    // Test invalid GUID for invalid format
    auto invalid_guid = smooth7zip::format::get_guid(smooth7zip::format::format{});
    EXPECT_EQ(invalid_guid, GUID{});

    // Test get all the extensions of the format zip
    auto exts = smooth7zip::format::get_extension(smooth7zip::format::get("zip"));
    EXPECT_EQ(exts, "zip z01 zipx jar xpi odt ods docx xlsx epub ipa apk appx");

    // Test detect format by extension
    EXPECT_EQ(smooth7zip::format::detect(std::filesystem::path("detect.zip")), smooth7zip::format::get("zip"));
    EXPECT_EQ(smooth7zip::format::detect(std::filesystem::path("detect.appx")), smooth7zip::format::get("zip"));
    EXPECT_EQ(smooth7zip::format::detect(std::filesystem::path("detect.z01")), smooth7zip::format::get("zip"));
    EXPECT_EQ(smooth7zip::format::detect(std::filesystem::path("detect.r00")), smooth7zip::format::get("Rar"));
    EXPECT_EQ(smooth7zip::format::detect(std::filesystem::path("detect.r01")), smooth7zip::format::get("Rar"));
    EXPECT_FALSE(smooth7zip::format::detect(std::filesystem::path("detect.invalid")).is_valid());
}

using smooth7zip_command = test::utils::rc_dir_test;

// Create a zip archive from the current directory
TEST_F(smooth7zip_command, a) {
    NWindows::NFile::NFind::CFileInfo fi;
    CObjectVector<NWindows::NFile::NFind::CFileInfo> to_archive_items;

    auto include_path = std::filesystem::directory_entry(this->test_data_dir_ / ".." / ".." / "include");
    std::stack<std::filesystem::directory_entry> to_archive_stack({include_path});
    do {
        auto cur_archive_path = to_archive_stack.top();
        to_archive_stack.pop();
        for (const auto &to_archive_entry : std::filesystem::directory_iterator(cur_archive_path)) {
            if (to_archive_entry.is_regular_file()) {
                ASSERT_TRUE(fi.Find(to_archive_entry.path().c_str()));
                to_archive_items.Add(fi);
            }
            else if (to_archive_entry.is_directory()) {
                to_archive_stack.push(to_archive_entry);
            }
        }
    }
    while (!to_archive_stack.empty());

    auto format_zip = smooth7zip::format::get("zip");
    auto format_id = smooth7zip::format::get_guid(format_zip);
    auto handler = smooth7zip::archive_handler();

    // Update callback
    auto *update_callback_spec = new smooth7zip::archive_update_callback(handler);
    update_callback_spec->dir_items = &to_archive_items;
    update_callback_spec->path_prefix = FString(include_path.path().c_str());
    CMyComPtr<IArchiveUpdateCallback2> update_callback(update_callback_spec);

    // Out Stream
    const auto archive_path = this->test_system_tmp_dir_ / "temp.zip";
    auto archive_name = FString(archive_path.c_str());
    auto *out_file_stream_spec = new COutFileStream;
    ASSERT_TRUE(out_file_stream_spec->Create(archive_name, True));
    CMyComPtr<IOutStream> out_stream(out_file_stream_spec);

    // Out Archive
    CMyComPtr<IOutArchive> out_archive;
    EXPECT_EQ(CreateObject(&format_id, &IID_IOutArchive, reinterpret_cast<void **>(&out_archive)), S_OK);

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

TEST_F(smooth7zip_command, l) {
    auto format_zip = smooth7zip::format::get("zip");
    auto format_id = smooth7zip::format::get_guid(format_zip);
    auto handler = smooth7zip::archive_handler();
    // In Archive
    CMyComPtr<IInArchive> in_archive;
    EXPECT_EQ(CreateObject(&format_id, &IID_IInArchive, reinterpret_cast<void **>(&in_archive)), S_OK);

    // In Stream
    auto regualr_zip = this->test_data_dir_ / "regular.zip";
    EXPECT_TRUE(std::filesystem::exists(regualr_zip));
    auto list_file_name = FString(regualr_zip.c_str());
    CInFileStream *in_file_stream_spec = new CInFileStream;

    EXPECT_TRUE(in_file_stream_spec->Open(list_file_name));
    CMyComPtr<IInStream> in_stream(in_file_stream_spec);

    {
        auto *open_callback_spec = new smooth7zip::archive_open_callback(handler);
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

TEST_F(smooth7zip_command, x) {
    auto format_zip = smooth7zip::format::get("zip");
    auto format_id = smooth7zip::format::get_guid(format_zip);
    auto handler = smooth7zip::archive_handler();

    // In Archive
    CMyComPtr<IInArchive> in_archive;
    EXPECT_EQ(CreateObject(&format_id, &IID_IInArchive, reinterpret_cast<void **>(&in_archive)), S_OK);

    // In Stream
    auto regualr_zip = this->test_data_dir_ / "regular.zip";
    const FString archive_name = FString(regualr_zip.c_str());
    CInFileStream *in_file_stream_spec = new CInFileStream;
    CMyComPtr<IInStream> in_stream(in_file_stream_spec);
    EXPECT_TRUE(in_file_stream_spec->Open(archive_name));

    {
        auto *open_callback_spec = new smooth7zip::archive_open_callback(handler);
        CMyComPtr<IArchiveOpenCallback> open_callback(open_callback_spec);
        open_callback_spec->passwd = ""; // empty password
        const UInt64 scan_size = 1 << 23;
        EXPECT_EQ(in_archive->Open(in_stream, &scan_size, open_callback), S_OK);
    }

    // Extract command
    auto *extract_callback_spec = new smooth7zip::archive_extract_callback(handler);
    auto extract_out_dir = FString(this->test_system_tmp_dir_.c_str());
    NWindows::NFile::NName::NormalizeDirPathPrefix(extract_out_dir);
    extract_callback_spec->in_archive = in_archive;
    extract_callback_spec->extract_out_dir = extract_out_dir;

    const wchar_t *names[] = {L"mt"};
    const unsigned num_props = sizeof(names) / sizeof(names[0]);
    NWindows::NCOM::CPropVariant values[num_props] = {static_cast<UInt32>(1)};
    CMyComPtr<ISetProperties> set_properties;
    in_archive->QueryInterface(IID_ISetProperties, reinterpret_cast<void **>(&set_properties));
    EXPECT_EQ(set_properties->SetProperties(names, values, num_props), S_OK);

    CMyComPtr<IArchiveExtractCallback> extract_callback(extract_callback_spec);
    EXPECT_EQ(in_archive->Extract(nullptr, static_cast<UInt32>(-1), false, extract_callback), S_OK);
    EXPECT_FALSE(std::filesystem::is_empty(this->test_system_tmp_dir_));
}

TEST(smooth7zip, archive) {
    // Test large page mode
    EXPECT_TRUE(smooth7zip::set_large_page_mode());
}

TEST_F(smooth7zip_command, archive_x) {
    auto zip_file = this->test_data_dir_ / "regular.zip";
    auto archive = smooth7zip::archive(zip_file, smooth7zip::archive::open_mode::read);
    archive.extract_to(this->test_system_tmp_dir_);
    EXPECT_FALSE(std::filesystem::is_empty(this->test_system_tmp_dir_));
}

TEST_F(smooth7zip_command, archive_l) {
    auto zip_file = this->test_data_dir_ / "regular.zip";
    auto archive = smooth7zip::archive(zip_file, smooth7zip::archive::open_mode::read);
    EXPECT_EQ(archive.get_count(), 3);
}

TEST_F(smooth7zip_command, archive_a) {
    auto zip_file = this->test_data_dir_ / "regular.zip";
    auto archive = smooth7zip::archive(zip_file, smooth7zip::archive::open_mode::write);
    auto include_path = std::filesystem::directory_entry(this->test_data_dir_ / ".." / ".." / "include");
    auto saved_path = this->test_system_tmp_dir_ / "temp.zip";
    archive.add_directory(include_path.path());
    archive.compress_to(saved_path);
    EXPECT_TRUE(std::filesystem::exists(saved_path));
}
