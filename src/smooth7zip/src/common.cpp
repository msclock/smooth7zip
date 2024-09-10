#include "common.hpp"

#include <7zip/CPP/Common/StringConvert.h>

namespace smooth7zip {
namespace strings {
std::string to_string(const wchar_t* _wstr_src) {
    if (_wstr_src == nullptr) {
        return {};
    }
    return GetAnsiString(_wstr_src).Ptr();
}

std::string to_string(const std::wstring& _wstr) {
    if (_wstr.empty()) {
        return {};
    }
    return to_string(_wstr.c_str());
}

std::wstring to_wstring(const char* _str_src) {
    if (_str_src == nullptr) {
        return {};
    }
    return GetUnicodeString(_str_src).Ptr();
}

std::wstring to_wstring(const std::string& _str) {
    if (_str.empty()) {
        return {};
    }
    return to_wstring(_str.c_str());
}

} // namespace strings
} // namespace smooth7zip
