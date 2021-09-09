#pragma once

#include <chrono>
#include <cmath>
#include <compare>
#include <cstdint>
#include <functional>

using u32 = std::uint32_t;
using u64 = std::uint64_t;

static_assert(sizeof(std::chrono::time_point<std::chrono::system_clock>) == 8);

namespace cbs {

struct time_prototype {
    u64 timestamp{};

    constexpr time_prototype() = default;
    constexpr time_prototype(const u64 timestamp)
            : timestamp(timestamp) {}

    constexpr auto operator+=(const std::chrono::seconds period) {
        timestamp += period.count();
        return *this;
    }

    constexpr auto operator<=>(const time_prototype &) const = default;
};

using time = time_prototype;
static_assert(sizeof(time) == 8);
} // namespace cbs

struct time_range {
    cbs::time begin;
    cbs::time end;
};

namespace currency::data::provider {

struct currency_details {
    float price;
};

template <typename type>
concept model = requires(type object) {
    { object.value(cbs::time{}) } -> std::same_as<currency_details>;
    { object.range() } -> std::same_as<time_range>;
    { object.range_changed(std::function<void()>()) } -> std::same_as<void>;
};

} // namespace currency::data::provider
