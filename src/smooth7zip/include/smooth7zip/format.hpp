#pragma once

#include <algorithm>
#include <bitset>
#include <limits>
#include <map>
#include <string>

#include <smooth7zip/common.hpp>

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
    format_registry() {
        uint32_t count = 0;
        if (S_OK != GetNumberOfFormats(&count)) {
            return;
        }

        for (size_t i = 0; i < count; i++) {
            NWindows::NCOM::CPropVariant prop;
            if (S_OK != GetHandlerProperty2(static_cast<UInt32>(i), NArchive::NHandlerPropID::kName, &prop)) {
                continue;
            }
            auto name = strings::to_string(prop.bstrVal);
            this->formats_.emplace(name, format(static_cast<uint32_t>(i), name));
        }
    }

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

inline auto get_guid(const format& _format) noexcept -> GUID {
    if (_format.index == std::numeric_limits<uint32_t>::max()) {
        return GUID{};
    }
    NWindows::NCOM::CPropVariant prop_clsid;
    if (S_OK
        != GetHandlerProperty2(static_cast<UInt32>(_format.index), NArchive::NHandlerPropID::kClassID, &prop_clsid)) {
        return GUID{};
    }
    return *reinterpret_cast<const GUID*>(prop_clsid.bstrVal);
}

/**
 * @brief Return a space-separated string of format supported extensions.
 */
inline auto get_extension(const format& _format) noexcept -> std::string {
    if (_format.index == std::numeric_limits<uint32_t>::max()) {
        return {};
    }
    std::string ext, addExt;
    NWindows::NCOM::CPropVariant prop_ext;
    if (S_OK
        == GetHandlerProperty2(static_cast<UInt32>(_format.index), NArchive::NHandlerPropID::kExtension, &prop_ext)) {
        ext = strings::to_string(prop_ext.bstrVal);
    }
    NWindows::NCOM::CPropVariant prop_add_ext;

    if (S_OK
        == GetHandlerProperty2(static_cast<UInt32>(_format.index),
                               NArchive::NHandlerPropID::kAddExtension,
                               &prop_add_ext)) {
        addExt = strings::to_string(prop_add_ext.bstrVal);
    }

    return addExt.empty() ? ext : ext + " " + addExt;
}

inline auto get(const std::string& _name) noexcept -> format {
    return format_registry::instance().get(_name);
}

/**
 * @brief Return the format by file extension.
 */
inline auto detect(const std::filesystem::path& input_path) {
    auto input_ext = input_path.extension().string();

    if (input_ext.starts_with('.')) {
        std::transform(input_ext.begin() + 1, input_ext.end(), input_ext.begin(), [](unsigned char c) {
            return std::tolower(c);
        });
        input_ext.erase(input_ext.size() - 1, 1);
    }

    if (input_ext.empty())
        return format{};

    for (auto& format_item : format_registry::instance()) {
        auto exts = get_extension(format_item);
        auto pos = exts.find(input_ext);
        if (pos == std::string::npos)
            continue;

        if (pos > 0 && exts[pos - 1] != ' ')
            continue;

        if (pos + input_ext.size() < exts.size() && exts[pos + input_ext.size()] != ' ')
            continue;

        return format_item;
    }

    // special case for rar("r00" or "r01") and zip("z01" or "z02")
    if ((input_ext[0] == 'r' || input_ext[0] == 'r')
        && (input_ext.size() == 3 && std::isdigit(input_ext[1]) && std::isdigit(input_ext[2]))) {
        return input_ext[0] == 'r' ? get("Rar") : get("zip");
    }

    return format{};
}

/**
 * @brief Return the format by file signature.
 */
inline auto detect(const std::streambuf& input) {
    (void)input;
    // TODO: detect by input data
    return format{};
}
} // namespace format
} // namespace smooth7zip
