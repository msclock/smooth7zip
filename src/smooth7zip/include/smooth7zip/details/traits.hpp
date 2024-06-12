#pragma once

#include <type_traits>

namespace smooth7zip {
namespace detail {

struct copyable_movable {};

class noncopyable {
protected:
    noncopyable() = default;
    ~noncopyable() = default;

    noncopyable(noncopyable const&) = delete;
    noncopyable& operator=(noncopyable const&) = delete;
};

struct movable {
    constexpr movable() noexcept = default;
    ~movable() noexcept = default;

    movable(movable&&) noexcept = default;
    movable& operator=(movable&&) noexcept = default;
    movable(const movable&) = delete;
    movable& operator=(const movable&) = delete;
};

struct noncopyable_nonmovable {
    constexpr noncopyable_nonmovable() noexcept = default;
    ~noncopyable_nonmovable() noexcept = default;

    noncopyable_nonmovable(noncopyable_nonmovable&&) = delete;
    noncopyable_nonmovable& operator=(noncopyable_nonmovable&&) = delete;
    noncopyable_nonmovable(const noncopyable_nonmovable&) = delete;
    noncopyable_nonmovable& operator=(const noncopyable_nonmovable&) = delete;
};

template <bool Copy, bool Move>
using enable_copy_move =
    std::conditional_t<Copy, copyable_movable, std::conditional_t<Move, movable, noncopyable_nonmovable>>;

} // namespace detail
} // namespace smooth7zip
