#pragma once

#include <chrono>
#include <cmath>
#include <compare>
#include <cstdint>
#include <functional>

#include "types.hpp"
#include <range/v3/all.hpp>

namespace provider {

template <typename type>
concept model = requires(type object) {
    { object.value(types::time_point{}) } -> std::same_as<types::currency>;
    { object.range() } -> std::same_as<types::time_range>;
    { object.range_changed(std::function<void()>()) } -> std::same_as<void>;
};

constexpr auto view_on_price(const ::ranges::range auto &view, const model auto &model) {
    return view | ranges::views::transform([&model](auto &&val) -> float { return model.value(val).price; });
}

} // namespace provider

