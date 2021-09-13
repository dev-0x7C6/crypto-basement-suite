#pragma once

#include <chrono>
#include <cmath>
#include <compare>
#include <cstdint>
#include <functional>

#include <model/types.hpp>

namespace provider {

template <typename type>
concept model = requires(type object) {
    { object.value(types::time_point{}) } -> std::same_as<types::currency>;
    { object.range() } -> std::same_as<types::time_range>;
    { object.range_changed(std::function<void()>()) } -> std::same_as<void>;
};

} // namespace provider

namespace indicator {
template <typename type>
concept indicator_model = requires(type object) {
    { object.compute_value(types::time_point{}) } -> std::same_as<types::indicator_value>;
    { object.configure(std::list<std::tuple<std::string, std::string>>()) } -> std::same_as<void>;
    { object.load_data(types::currency{}) } -> std::same_as<void>;
};
} // namespace indicator
