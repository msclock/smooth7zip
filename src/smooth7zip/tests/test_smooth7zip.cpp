#include <gtest/gtest.h>
#include "smooth7zip/smooth7zip.hpp"

TEST(smooth7zip, version) {
    const auto version = git_ProjectVersion();
    EXPECT_STRNE(version, "");
}

TEST(smooth7zip, distribution) {
    const auto is_debug = smooth7zip::distribution::is_debug();
#ifdef _DEBUG
    EXPECT_TRUE(is_debug);
#else
    EXPECT_FALSE(is_debug);
#endif
}
