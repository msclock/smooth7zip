#include <gtest/gtest.h>

#include "smooth7zip.hpp"

#include "utils.hpp"

constexpr auto id_format_apm = GUID{0x23170F69, 0x40C1, 0x278A, {0x10, 0x00, 0x00, 0x01, 0x10, 0xD4, 0x00, 0x00}};

TEST(smooth7zip, format) {
    // Test to load the format registry correctly
    auto& reg = smooth7zip::format::format_registry::instance();
    EXPECT_EQ(reg.count(), 55);

    // Test to get a format by name
    auto format = reg.get("APM");
    EXPECT_TRUE(format.is_valid());

    // Test to get a format by index
    auto apm_format_from_index = reg.get(format.index);
    EXPECT_EQ(apm_format_from_index.name, "APM");

    // Test iterate over all formats
    for (const auto& f : reg) {
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

TEST(smooth7zip, archive) {
    // Test large page mode
    EXPECT_TRUE(smooth7zip::set_large_page_mode());
}

using smooth7zip_command = test::utils::rc_dir_test;

TEST_F(smooth7zip_command, archive_x) {
    auto zip_file = this->test_data_dir_ / "regular.zip";
    auto archive = smooth7zip::archive(zip_file, smooth7zip::archive::open_mode::read);
    archive.extract_to(this->system_test_tmp_dir_);
    EXPECT_FALSE(std::filesystem::is_empty(this->system_test_tmp_dir_));
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
    auto saved_path = this->system_test_tmp_dir_ / "temp.zip";
    archive.add_directory(include_path.path());
    archive.compress_to(saved_path);
    EXPECT_TRUE(std::filesystem::exists(saved_path));
}

TEST(smooth7zip, distribution) {
    const auto is_debug = smooth7zip::distribution::is_debug();
#ifdef _DEBUG
    EXPECT_TRUE(is_debug);
#else
    EXPECT_FALSE(is_debug);
#endif
}
