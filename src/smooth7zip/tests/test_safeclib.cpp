#include <gtest/gtest.h>
#include <safeclib/safec.h>
#include <wchar.h>

TEST(verify_safeclib, wcsrtombs) {
    std::setlocale(LC_ALL, "en_US.utf8");
    const wchar_t* wstr = L"z\u00df\u6c34\U0001f34c";
    std::mbstate_t state{};
    size_t len = 1 + wcsrtombs(NULL, &wstr, 0, &state);

    std::string mbstr(len, '\0');
    wcsrtombs(mbstr.data(), &wstr, len, &state);
    GTEST_LOG_(INFO) << "Multibyte string: " << mbstr;
    EXPECT_EQ(11, len);
}

TEST(verify_safeclib, wcsrtombs_s) {
    std::setlocale(LC_ALL, "en_US.utf8");
    const wchar_t* wstr = L"z\u00df\u6c34\U0001f34c";
    std::mbstate_t state{};
    size_t len{};
    wcsrtombs_s(&len, NULL, 0, &wstr, 0, &state);
    EXPECT_EQ(len, 10);
}
