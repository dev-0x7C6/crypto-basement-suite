#pragma once

#include <chrono>
#include <cmath>
#include <compare>
#include <cstdint>
#include <functional>

#include <types.hpp>

namespace provider {

template <typename type>
concept model = requires(type object) {
    { object.value(types::time_point{}) } -> std::same_as<types::currency>;
    { object.range() } -> std::same_as<types::time_range>;
    { object.range_changed(std::function<void()>()) } -> std::same_as<void>;
};

} // namespace provider

