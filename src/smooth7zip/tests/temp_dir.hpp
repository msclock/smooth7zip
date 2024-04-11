#pragma once

#include <gtest/gtest.h>
#include <filesystem>
#include <random>

class temp_dir_test : public ::testing::Test {
protected:
    std::filesystem::path temp_dir;

    void SetUp() override {
        std::random_device rd;
        std::mt19937_64 gen(rd());

        std::uniform_int_distribution<> dis(1000, 9999);
        auto suffix_dir_name = std::to_string(dis(gen)) + '_' + std::to_string(dis(gen));
        temp_dir = std::filesystem::temp_directory_path() / suffix_dir_name;
        std::filesystem::create_directory(temp_dir);
    }

    void TearDown() override { std::filesystem::remove_all(temp_dir); }
};
