#pragma once

#include <bitset>
#include <limits>
#include <map>
#include <string>

#include "smooth7zip/common.hpp"
#include "smooth7zip/details/traits.hpp"

namespace smooth7zip {
namespace format {
struct format {
    explicit format(uint32_t _index = std::numeric_limits<uint32_t>::max(), const std::string_view _name = "") noexcept
        : index(_index),
          name(_name) {}

    auto operator==(const format& _other) const noexcept -> bool { return name == _other.name; }

    auto operator!=(const format& _other) const noexcept -> bool { return !(*this == _other); }

    auto is_valid() const noexcept -> bool { return index != std::numeric_limits<uint32_t>::max() && !name.empty(); }

    const uint32_t index;
    const std::string name;
};

class format_registry : public detail::enable_copy_move<false, false> {
private:
    format_registry() {
        uint32_t count = 0;
        if (S_OK != GetNumberOfFormats(&count)) {
            return;
        }

        for (size_t i = 0; i < count; i++) {
            NWindows::NCOM::CPropVariant prop_name;
            if (S_OK != GetHandlerProperty2(static_cast<UInt32>(i), NArchive::NHandlerPropID::kName, &prop_name)) {
                continue;
            }
            auto name = strings::to_string(prop_name.bstrVal);
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
};

inline auto get_format_guid(const format& _format) noexcept -> GUID {
    if (_format.index == std::numeric_limits<uint32_t>::max()) {
        return GUID{};
    }
    NWindows::NCOM::CPropVariant prop_clsid;
    GetHandlerProperty2(static_cast<UInt32>(_format.index), NArchive::NHandlerPropID::kClassID, &prop_clsid);
    return *reinterpret_cast<const GUID*>(prop_clsid.bstrVal);
}

inline auto get(const std::string& _name) noexcept -> format {
    return format_registry::instance().get(_name);
}
} // namespace format
} // namespace smooth7zip
