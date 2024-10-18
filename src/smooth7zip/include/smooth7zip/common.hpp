
#pragma once

#include <string>
#include <system_error>
#include <vector>

#include <7zip/7zip.h>
#include <7zip/CPP/Common/StringConvert.h>
#include <7zip/CPP/Windows/PropVariant.h>

#if _WIN32
#define S7_UNKNOWN_DESTRUCTOR(x)         x
#define S7_UNKNOWN_VIRTUAL_DESTRUCTOR(x) virtual x
#else
#define S7_UNKNOWN_DESTRUCTOR(x)         x override
#define S7_UNKNOWN_VIRTUAL_DESTRUCTOR(x) S7_UNKNOWN_DESTRUCTOR(x)
#endif

#define MY_STDMETHOD_NOEXCEPT(method, ...) auto STDMETHODCALLTYPE method(__VA_ARGS__) noexcept -> HRESULT
#define S7_STDMETHOD(method, ...)          MY_STDMETHOD_NOEXCEPT(method, __VA_ARGS__) override

namespace smooth7zip {
namespace strings {

// Convert a wide string to an ANSI string.
inline std::string to_string(const wchar_t* _wstr_src) {
    if (_wstr_src == nullptr) {
        return {};
    }
    return GetAnsiString(_wstr_src).Ptr();
}

inline std::string to_string(const std::wstring& _wstr) {
    if (_wstr.empty()) {
        return {};
    }
    return to_string(_wstr.c_str());
}

// Convert an ANSI string to a wide string.
inline std::wstring to_wstring(const char* _str_src) {
    if (_str_src == nullptr) {
        return {};
    }
    return GetUnicodeString(_str_src).Ptr();
}

inline std::wstring to_wstring(const std::string& _str) {
    if (_str.empty()) {
        return {};
    }
    return to_wstring(_str.c_str());
}

}; // namespace strings
} // namespace smooth7zip
