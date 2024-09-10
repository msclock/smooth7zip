#pragma once

#include <algorithm>
#include <bitset>
#include <filesystem>
#include <limits>
#include <map>
#include <string>

#include "common.hpp"

namespace smooth7zip {
namespace format {
struct format {
    explicit format(uint32_t _index = std::numeric_limits<uint32_t>::max(), const std::string_view _name = "") noexcept
        : index(_index),
          name(_name) {}

    format(const format&) = default;

    format& operator=(const format&) = default;

    auto operator==(const format& _other) const noexcept -> bool { return name == _other.name; }

    auto operator!=(const format& _other) const noexcept -> bool { return !(*this == _other); }

    auto is_valid() const noexcept -> bool { return index != std::numeric_limits<uint32_t>::max() && !name.empty(); }

    uint32_t index;
    std::string name;
};

class format_registry {
private:
    format_registry();

    ~format_registry() = default;

    std::map<std::string, format> formats_;

public:
    struct iterator {
        using value_type = format;
        using difference_type = ptrdiff_t;
        using pointer = const value_type*;
        using reference = const value_type&;
        using iterator_category = std::forward_iterator_tag;

        explicit iterator(const std::map<std::string, format>::const_iterator index) noexcept : index_(index) {}

        auto operator*() const noexcept -> reference { return index_.operator*().second; }

        auto operator++() noexcept -> iterator& {
            std::advance(this->index_, 1);
            return *this;
        }

        auto operator++(int) noexcept -> iterator {
            auto copy = *this;
            std::advance(this->index_, 1);
            return copy;
        }

        auto operator==(const iterator& _other) const noexcept -> bool { return this->index_ == _other.index_; }

        auto operator!=(const iterator& _other) const noexcept -> bool { return !(*this == _other); }

    private:
        std::map<std::string, format>::const_iterator index_;
    };

    auto begin() const noexcept -> iterator { return iterator(this->formats_.begin()); }

    auto end() const noexcept -> iterator { return iterator(this->formats_.end()); }

    auto get(const std::string& _name) const noexcept -> format {
        auto it = this->formats_.find(_name);
        if (it == this->formats_.end()) {
            return format{};
        }
        return it->second;
    }

    auto get(const uint32_t _index) const noexcept -> format {
        if (_index >= this->formats_.size()) {
            return format{};
        }
        auto it = std::find_if(this->formats_.begin(), this->formats_.end(), [_index](const auto& format) {
            return format.second.index == _index;
        });
        return it != this->formats_.end() ? it->second : format{};
    }

    auto count() const noexcept -> size_t { return this->formats_.size(); }

    static auto instance() noexcept -> format_registry& {
        static format_registry registry;
        return registry;
    }

private:
    format_registry(const format_registry&) = delete;
    format_registry& operator=(const format_registry&) = delete;
};

auto get_guid(const format& _format) noexcept -> GUID;

/**
 * @brief Return true if the format supports updating.
 */
auto get_update(const format& _format) noexcept -> bool;
/**
 * @brief Return a space-separated string of format supported extensions.
 */
auto get_extension(const format& _format) noexcept -> std::string;

auto get(const std::string& _name) noexcept -> format {
    return format_registry::instance().get(_name);
}

/**
 * @brief Return the format by file extension.
 */
auto detect(const std::string& input_path) noexcept -> format;

/**
 * @brief Return the format by file signature.
 */
auto detect(const std::streambuf& input) noexcept -> format;

} // namespace format
} // namespace smooth7zip
