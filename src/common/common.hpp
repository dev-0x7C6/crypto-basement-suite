#pragma once

#include <array>
#include <cmath>
#include <cstdint>

using s32 = std::int32_t;
using s64 = std::int64_t;
using f32 = std::float_t;
using u16 = std::uint16_t;
using u32 = std::uint32_t;
using u64 = std::uint64_t;
using volume = std::pair<float, float>;

struct header {
    u16 magic{0xfeca};
    u16 version{1};
    std::array<char, 64> symbols{};

    constexpr auto operator<=>(const header &) const noexcept = default;
};

struct sample {
    u32 timestamp;
    f32 open;
    f32 high;
    f32 low;
    f32 close;
    volume vol;
    u32 trade_count;

    constexpr auto operator<=>(const sample &) const noexcept = default;
};

struct statistics {
    u64 empty{};
    u64 missing{};
    u64 unaligned{};
    u64 clones{};

    constexpr auto operator<=>(const statistics &) const noexcept = default;
};

static_assert(sizeof(header) == 68);
static_assert(sizeof(sample) == 32);
