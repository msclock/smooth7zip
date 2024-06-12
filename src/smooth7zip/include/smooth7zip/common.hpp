
#pragma once

#include <array>

#include <7zip/7zip.h>
#include <7zip/CPP/Common/StringConvert.h>
#include <7zip/CPP/Windows/PropVariant.h>

#include "types.hpp"

namespace smooth7zip {

namespace strings {

inline std::string to_string(const wchar_t* src) {
    // return GetAnsiString(src).Ptr();
#ifdef _WIN32
    return fs2fas(src).Ptr();
#else
    return us2fs(src).Ptr();
#endif
}

inline std::string to_string(const std::wstring& wide_str) {
    return to_string(wide_str.c_str());
}

inline std::wstring to_wstring(const char* src) {
    return GetUnicodeString(src).Ptr();
}

inline std::wstring to_wstring(const std::string& str) {
    return to_wstring(str.c_str());
}

}; // namespace strings

}; // namespace smooth7zip
