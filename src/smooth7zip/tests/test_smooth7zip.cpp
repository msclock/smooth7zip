#include "gtest/gtest.h"

#include "smooth7zip/smooth7zip.hpp"

#include "utils.hpp"

constexpr auto id_format_apm = GUID{0x23170F69, 0x40C1, 0x278A, {0x10, 0x00, 0x00, 0x01, 0x10, 0xD4, 0x00, 0x00}};

TEST(smooth7zip, format) {
    // Test to load the format registry correctly
    EXPECT_EQ(smooth7zip::format::format_registry::instance().format_count(), 55);

    // Test to get a format by name
    auto format = smooth7zip::format::format_registry::instance().get("APM");
    EXPECT_TRUE(format.is_valid());

    // Test to get a format by index
    auto index_0_format = smooth7zip::format::format_registry::instance().get(0);
    EXPECT_EQ(index_0_format.name, "APM");

    // Test iterate over all formats
    for (auto f : smooth7zip::format::format_registry::instance()) {
        EXPECT_TRUE(f.is_valid());
    }

    // Test get the GUID of a format
    auto format_guid = smooth7zip::format::get_format_guid(format);
    EXPECT_EQ(format_guid, id_format_apm);

    // Test invalid GUID for invalid format
    auto invalid_guid = smooth7zip::format::get_format_guid(smooth7zip::format::format{});
    EXPECT_EQ(invalid_guid, GUID{});
}
