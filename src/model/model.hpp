#pragma once

#include <chrono>
#include <cstdint>
#include <cmath>
#include <functional>
#include <concepts>
#include <iostream>

using u32 = std::uint32_t;
using u64 = std::uint64_t;

static_assert(sizeof(std::chrono::time_point<std::chrono::system_clock>) == 8);

namespace cbs {
using time = std::chrono::time_point<std::chrono::system_clock>;
static_assert(sizeof(time) == 8);
}

struct time_range {
    cbs::time begin;
    cbs::time end;
};

namespace currency::data::provider {

struct currency_details {
    double price;
};

template <typename type>
concept model = requires(type object) {
    { object.value(cbs::time{}) } -> std::same_as<currency_details>;
    { object.range() } -> std::same_as<time_range>;
    { object.range_changed(std::function<void()>()) } -> std::same_as<void>;
};

} // namespace currency::data::provider

