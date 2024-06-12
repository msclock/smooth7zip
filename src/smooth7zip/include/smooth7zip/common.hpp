
#pragma once

#include <string>
#include <system_error>
#include <vector>

#include <7zip/7zip.h>
#include <7zip/CPP/Common/StringConvert.h>
#include <7zip/CPP/Windows/PropVariant.h>

#include <fmt/format.h>

#if S7_CPP_STANDARD < 23
#include <tl/expected.hpp>
#else
#include <expected>
#endif

#if (defined(_MSVC_LANG) && _MSVC_LANG >= 202002L) || (defined(__cplusplus) && __cplusplus >= 202002L)
#define S7_CPP_STANDARD 20
#elif (defined(_MSVC_LANG) && _MSVC_LANG >= 201703L) || (defined(__cplusplus) && __cplusplus >= 201703L)
#define S7_CPP_STANDARD 17
#elif (defined(_MSVC_LANG) && _MSVC_LANG >= 201402L) || (defined(__cplusplus) && __cplusplus >= 201402L)
#define S7_CPP_STANDARD 14
#else
#define S7_CPP_STANDARD 11
#endif

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

#if S7_CPP_STANDARD < 23
template <class T, class E>
using expected = tl::expected<T, E>;
template <class E>
using unexpected = tl::unexpected<E>;

template <class E>
unexpected<typename std::decay<E>::type> make_unexpected(E&& e) {
    return tl::make_unexpected(std::forward<E>(e));
}

#else
template <class T, class E>
using expected = std::expected<T, E>;
template <class E>
using unexpected = std::unexpected<E>;

template <class E>
unexpected<typename std::decay<E>::type> make_unexpected(E&& e) {
    return std::make_unexpected(std::forward<E>(e));
}
#endif

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
