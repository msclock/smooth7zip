#pragma once

#include <filesystem>
#include <fstream>
#include <iomanip>
#include <random>
#include <sstream>
#include <string>

#include <gtest/gtest.h>

#ifndef TESTS_DIR
#error "TESTS_DIR is not defined"
#endif

#define TEST_DATA_DIR_NAME "test_data"
#define TEST_TEMP_DIR_NAME "tmp"

namespace test {
namespace utils {
class rc_dir_test : public ::testing::Test {
protected:
    std::filesystem::path tests_dir_{TESTS_DIR};
    std::filesystem::path test_data_dir_;
    std::filesystem::path system_test_tmp_dir_;

    void SetUp() override {
        this->test_data_dir_ = this->tests_dir_ / TEST_DATA_DIR_NAME;
        std::mt19937_64 gen(std::random_device{}());
        std::uniform_int_distribution<> dis(1000, 9999);
        auto suffix_dir_name = std::to_string(dis(gen)) + std::to_string(dis(gen));
        this->system_test_tmp_dir_ = this->test_data_dir_ / TEST_TEMP_DIR_NAME / suffix_dir_name;
        std::filesystem::create_directories(system_test_tmp_dir_);
    }

    void TearDown() override { std::filesystem::remove_all(system_test_tmp_dir_); }
};

std::string to_string(const GUID& guid) {
    std::stringstream ss;
    ss << std::hex << std::setfill('0');
    ss << "{" << std::setw(8) << guid.Data1 << "-" << std::setw(4) << guid.Data2 << "-" << std::setw(4) << guid.Data3
       << "-";
    for (size_t i = 0; i < sizeof(guid.Data4); i++) {
        ss << std::setw(2) << static_cast<unsigned int>(guid.Data4[i]);
    }
    ss << "}";
    return ss.str();
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
