#pragma once

#include <array>
#include <cmath>
#include <cstdint>
#include <numeric>

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
    u32 timestamp{};
    f32 open{std::numeric_limits<f32>::quiet_NaN()};
    f32 high{std::numeric_limits<f32>::quiet_NaN()};
    f32 low{std::numeric_limits<f32>::quiet_NaN()};
    f32 close{std::numeric_limits<f32>::quiet_NaN()};
    volume vol{std::numeric_limits<f32>::quiet_NaN(), std::numeric_limits<f32>::quiet_NaN()};
    u32 trade_count{};

    constexpr auto operator<=>(const sample &) const noexcept = default;
};

constexpr auto make_zeroed_sample() -> sample {
    return {
        .timestamp = 0,
        .open = 0.0f,
        .high = 0.0f,
        .low = 0.0f,
        .close = 0.0f,
        .vol = {0.0f, 0.0f},
        .trade_count = 0,
    };
}

struct statistics {
    u64 empty{};
    u64 missing{};
    u64 unaligned{};
    u64 clones{};

    constexpr auto operator<=>(const statistics &) const noexcept = default;
};

static_assert(sizeof(header) == 68);
static_assert(sizeof(sample) == 32);
