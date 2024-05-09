#pragma once

#include <bitset>
#include <limits>
#include <map>
#include <string>

#ifndef NOMINMAX
#undef max
#undef min
#endif

#include <7zip/7zip.h>
#include "7zip/CPP/Windows/PropVariant.h"

#include "smooth7zip/details/traits.hpp"

namespace smooth7zip {
namespace format {
struct format {
    explicit format(uint32_t index_ = std::numeric_limits<uint32_t>::max(), const std::string_view name_ = "") noexcept
        : index(index_),
          name(name_) {}

    auto operator==(const format& other) const noexcept -> bool { return name == other.name; }

    auto operator!=(const format& other) const noexcept -> bool { return !(*this == other); }

    auto is_valid() const noexcept -> bool { return index != std::numeric_limits<uint32_t>::max() && !name.empty(); }

    const uint32_t index;
    const std::string name;
};

class format_registry : public detail::enable_copy_move<false, false> {
private:
    format_registry() {
        uint32_t count = 0;
        GetNumberOfFormats(&count);
        for (size_t i = 0; i < count; i++) {
            NWindows::NCOM::CPropVariant prop_name;
            GetHandlerProperty2(static_cast<UInt32>(i), NArchive::NHandlerPropID::kName, &prop_name);
            auto name = us2fs(prop_name.bstrVal);
            this->_formats.emplace(name, format(static_cast<uint32_t>(i), static_cast<const char*>(name)));
        }
        this->_format_count = count;
    }

    ~format_registry() = default;

    uint32_t _format_count = 0;
    std::map<std::string, format> _formats;

public:
    struct iterator {
        using value_type = format;
        using difference_type = ptrdiff_t;
        using pointer = const value_type*;
        using reference = const value_type&;
        using iterator_category = std::forward_iterator_tag;

        explicit iterator(const std::map<std::string, format>::const_iterator index) noexcept : _index(index) {}

        auto operator*() const noexcept -> reference { return _index.operator*().second; }

        auto operator++() noexcept -> iterator& {
            std::advance(this->_index, 1);
            return *this;
        }

        auto operator++(int) noexcept -> iterator {
            auto copy = *this;
            std::advance(this->_index, 1);
            return copy;
        }

        auto operator==(const iterator& other) const noexcept -> bool { return this->_index == other._index; }

        auto operator!=(const iterator& other) const noexcept -> bool { return !(*this == other); }

    private:
        std::map<std::string, format>::const_iterator _index;
    };

    auto begin() const noexcept -> iterator { return iterator(this->_formats.begin()); }

    auto end() const noexcept -> iterator { return iterator(this->_formats.end()); }

    auto get(const std::string& format_name) const noexcept -> format {
        auto it = this->_formats.find(format_name);
        if (it == this->_formats.end()) {
            return format{};
        }
        return it->second;
    }

    auto get(const uint32_t index) const noexcept -> format {
        if (index >= this->_format_count) {
            return format{};
        }
        auto it = std::find_if(this->_formats.begin(), this->_formats.end(), [index](const auto& format) {
            return format.second.index == index;
        });
        return it != this->_formats.end() ? it->second : format{};
    }

    auto format_count() const noexcept -> uint32_t { return this->_format_count; }

    static auto instance() noexcept -> format_registry& {
        static format_registry registry;
        return registry;
    }
};

inline auto get_format_guid(const format& f) noexcept -> GUID {
    if (f.index == std::numeric_limits<uint32_t>::max()) {
        return GUID{};
    }
    NWindows::NCOM::CPropVariant prop_clsid;
    GetHandlerProperty2(static_cast<UInt32>(f.index), NArchive::NHandlerPropID::kClassID, &prop_clsid);
    return *reinterpret_cast<const GUID*>(prop_clsid.bstrVal);
}

inline auto get(const std::string& format_name) noexcept -> format {
    return format_registry::instance().get(format_name);
}
} // namespace format
} // namespace smooth7zip
