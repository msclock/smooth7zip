#pragma once

#include <filesystem>
#include <fstream>
#include <iomanip>
#include <random>
#include <sstream>
#include <string>

#include <7zip/7zip.h>
#include <gtest/gtest.h>
#include <cmrc/cmrc.hpp>

CMRC_DECLARE(test_data);

namespace test {
namespace utils {
class rc_dir_test : public ::testing::Test {
protected:
    std::filesystem::path temp_dir;

    void SetUp() override {
        std::mt19937_64 gen(std::random_device{}());

        std::uniform_int_distribution<> dis(1000, 9999);
        auto suffix_dir_name = std::to_string(dis(gen)) + '_' + std::to_string(dis(gen));
        temp_dir = std::filesystem::temp_directory_path() / suffix_dir_name;
        std::filesystem::create_directory(temp_dir);
    }

    auto get_test_data(const std::string& data_path) const -> std::string {
        auto rc_path = cmrc::test_data::get_filesystem();
        auto rc_data = rc_path.open(data_path);
        auto test_data_path = temp_dir / data_path;
        auto data_dir = test_data_path.parent_path();
        if (!std::filesystem::exists(data_dir))
            std::filesystem::create_directories(data_dir);
        {
            std::ofstream ofs(test_data_path, std::ios::binary);
            ofs.write(rc_data.begin(), static_cast<std::streamsize>(rc_data.size()));
        }
        EXPECT_TRUE(std::filesystem::exists(test_data_path));
        return test_data_path.string();
    }

    void TearDown() override { std::filesystem::remove_all(temp_dir); }
};

inline std::string to_string(const GUID& guid) {
    std::stringstream ss;
    ss << std::hex << std::setfill('0');
    ss << "{" << std::setw(8) << guid.Data1 << "-" << std::setw(4) << guid.Data2 << "-" << std::setw(4) << guid.Data3
       << "-" << std::setw(2) << static_cast<unsigned int>(guid.Data4[0]) << std::setw(2)
       << static_cast<unsigned int>(guid.Data4[1]) << "-" << std::setw(2) << static_cast<unsigned int>(guid.Data4[2])
       << std::setw(2) << static_cast<unsigned int>(guid.Data4[3]) << std::setw(2)
       << static_cast<unsigned int>(guid.Data4[4]) << std::setw(2) << static_cast<unsigned int>(guid.Data4[5])
       << std::setw(2) << static_cast<unsigned int>(guid.Data4[6]) << std::setw(2)
       << static_cast<unsigned int>(guid.Data4[7]) << "}";
    return ss.str();
}

inline std::string wstring_to_string(const std::wstring& wide_str) {
    std::string ret;
    std::mbstate_t state{};
    const wchar_t* src = wide_str.c_str();
    std::size_t len = std::wcsrtombs(nullptr, &src, 0, &state);
    if (static_cast<std::size_t>(-1) != len) {
        std::vector<char> buff(len);
        len = std::wcsrtombs(&buff[0], &src, len, &state);
        if (static_cast<std::size_t>(-1) != len) {
            ret.assign(&buff[0], len);
        }
    }
    return ret;
}

inline std::wstring string_to_wstring(const std::string& str) {
    std::wstring ret;
    std::mbstate_t state{};
    const char* src = str.data();
    std::size_t len = std::mbsrtowcs(nullptr, &src, 0, &state);
    if (static_cast<std::size_t>(-1) != len) {
        std::vector<wchar_t> buff(len);
        len = std::mbsrtowcs(&buff[0], &src, len, &state);
        if (static_cast<std::size_t>(-1) != len) {
            ret.assign(&buff[0], len);
        }
    }
    return ret;
}

std::string hex_to_str(const char* data, const std::size_t length, const std::size_t space_pos) {
    std::stringstream ss;
    ss << std::hex << std::uppercase << std::setfill('0');
    std::size_t insert_space_counter = 0;
    for (size_t i = 0; i < length; i++) {
        ss << std::setw(2) << static_cast<int>(static_cast<std::uint8_t>(data[i]));
        insert_space_counter++;
        if (insert_space_counter == space_pos) {
            ss << " ";
            insert_space_counter = 0;
        }
    }
    return ss.str();
}
} // namespace utils
} // namespace test
