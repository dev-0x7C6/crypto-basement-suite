#pragma once

#include <ctime>
#include <cstdint>
#include <concepts>
#include <iterator>

using u32 = std::uint32_t;
using u64 = std::uint64_t;

namespace time {
using point = u64;

struct span {
    point begin;
    point end;
};

} // namespace time
