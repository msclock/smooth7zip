#include "format.hpp"

#include <filesystem>

#include <7zip/CPP/Windows/PropVariant.h>

namespace smooth7zip {

namespace format {

format_registry::format_registry() {
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

auto get_guid(const format& _format) noexcept -> GUID {
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

auto get_update(const format& _format) noexcept -> bool {
    if (_format.index == std::numeric_limits<uint32_t>::max()) {
        return false;
    }
    NWindows::NCOM::CPropVariant prop_update;
    if (S_OK
        != GetHandlerProperty2(static_cast<UInt32>(_format.index), NArchive::NHandlerPropID::kUpdate, &prop_update)) {
        return false;
    }
    return prop_update.boolVal != VARIANT_FALSE;
}

auto get_extension(const format& _format) noexcept -> std::string {
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

auto detect(const std::string& input_path) noexcept -> format {
    std::filesystem::path input_path_normalized = std::filesystem::absolute(input_path);
    auto input_ext = input_path_normalized.extension().string();

    if (input_path_normalized.has_extension() && input_ext[0] == ('.')) {
        std::transform(input_ext.begin() + 1, input_ext.end(), input_ext.begin(), [](unsigned char c) {
            return std::tolower(c);
        });
        input_ext.erase(input_ext.size() - 1, 1);
    }

    if (input_ext.empty())
        return format{};

    // special case for rar("r00" or "r01") and zip("z01" or "z02")
    if ((input_ext[0] == 'r' || input_ext[0] == 'r')
        && (input_ext.size() == 3 && std::isdigit(input_ext[1]) && std::isdigit(input_ext[2]))) {
        return input_ext[0] == 'r' ? get("Rar") : get("zip");
    }

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

    return format{};
}

auto detect(const std::streambuf& input) noexcept -> format {
    (void)input;
    // TODO: detect by input data
    return format{};
}

} // namespace format

} // namespace smooth7zip
