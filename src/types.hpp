#pragma once

#include <chrono>
#include <cstdint>
#include <optional>
#include <ranges>
#include <string>

using u32 = std::uint64_t;
using u64 = std::uint64_t;

namespace types {

struct time_point {
    u64 point{};

    constexpr time_point() = default;
    constexpr time_point(const u64 point)
            : point(point) {}

    operator u64() {
        return point;
    }

    constexpr auto operator+=(const std::chrono::seconds period) {
        point += period.count();
        return *this;
    }

    constexpr auto operator<=>(const time_point &) const = default;

    constexpr auto after(const time_point &rhs) const noexcept -> bool {
        return *this < rhs;
    }
};

struct time_range {
    constexpr time_range(const time_point begin, const time_point end)
            : begin(begin)
            , end(end) {}

    time_point begin{};
    time_point end{};

    constexpr auto to_range() {
        return std::ranges::iota_view{begin.point, end.point};
    };
};

struct indicator_settings {
    std::optional<u32> frame_size;
};

struct currency {
    float price;
    time_point time_stamp;
};

struct indicator_value {
    float value{};
};

} // namespace types

static_assert(sizeof(types::time_point) == 8);
static_assert(sizeof(types::time_range) == 16);
