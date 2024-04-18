#pragma once

#include <string>
#include <vector>

#include <7zip/7zip.h>
#include <safeclib/safec.h>

namespace std {
inline std::string to_string(const GUID& guid) {
    char buffer[36];
    snprintf(buffer,
             sizeof(buffer),
#if defined(_WIN64)
             "%08lX-%04X-%04X-%02X%02X%02X%02X%02X%02X%02X%02X",
#else
             "%08X-%04X-%04X-%02X%02X%02X%02X%02X%02X%02X%02X",
#endif
             guid.Data1,
             guid.Data2,
             guid.Data3,
             guid.Data4[0],
             guid.Data4[1],
             guid.Data4[2],
             guid.Data4[3],
             guid.Data4[4],
             guid.Data4[5],
             guid.Data4[6],
             guid.Data4[7]);
    return std::string(buffer);
}
} // namespace std

namespace smooth7zip {
namespace string {
inline std::string wstring_to_string(const std::wstring& wide_str) {
    std::string ret;
    std::mbstate_t state{};
    const wchar_t* src = wide_str.c_str();
    std::size_t len = std::wcsrtombs(nullptr, &src, 0, &state);
    if (static_cast<std::size_t>(-1) != len) {
        std::vector<char> buff(len);
        len = std::wcsrtombs(&buff[0], &src, len, &state);
        if (static_cast<std::size_t>(-1) != len) {
            ret.assign(&buff[0], len);
        }
    }
    return ret;
}

inline std::wstring string_to_wstring(const std::string& str) {
    std::wstring ret;
    std::mbstate_t state{};
    const char* src = str.data();
    std::size_t len = std::mbsrtowcs(nullptr, &src, 0, &state);
    if (static_cast<std::size_t>(-1) != len) {
        std::vector<wchar_t> buff(len);
        len = std::mbsrtowcs(&buff[0], &src, len, &state);
        if (static_cast<std::size_t>(-1) != len) {
            ret.assign(&buff[0], len);
        }
    }
    return ret;
}
} // namespace string
} // namespace smooth7zip
