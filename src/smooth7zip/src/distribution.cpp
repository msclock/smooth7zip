#include "distribution.hpp"

namespace smooth7zip {
namespace distribution {
bool is_debug() noexcept {
#ifdef _DEBUG
    return true;
#else
    return false;
#endif
}
} // namespace distribution
} // namespace smooth7zip
