#include "handler.hpp"

namespace smooth7zip {

auto archive_update_callback::SetRatioInfo(const UInt64 *inSize, const UInt64 *outSize) noexcept -> LONG {
    if (inSize != nullptr && outSize != nullptr && handler.callbacks().ratio)
        handler.callbacks().ratio(*inSize, *outSize);
    return S_OK;
}

} // namespace smooth7zip
