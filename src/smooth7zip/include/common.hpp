
#pragma once

#include <string>
#include <system_error>
#include <vector>

#include <7zip/7zip.h>

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
std::string to_string(const wchar_t* _wstr_src);

std::string to_string(const std::wstring& _wstr);

// Convert an ANSI string to a wide string.
std::wstring to_wstring(const char* _str_src);

std::wstring to_wstring(const std::string& _str);

}; // namespace strings
} // namespace smooth7zip
