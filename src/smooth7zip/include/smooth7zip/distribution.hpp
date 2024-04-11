#pragma once

namespace smooth7zip {
namespace distribution {
inline bool is_debug() noexcept {
#ifdef _DEBUG
    return true;
#else
    return false;
#endif
}
} // namespace distribution
} // namespace smooth7zip
