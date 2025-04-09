#pragma once

#include <cmath>
#include <functional>
#include <ranges>

#include "types.hpp"

namespace provider {

template <typename type>
concept model = requires(type object) {
    { object.value(types::time_point{}) } -> std::same_as<types::currency>;
    { object.range() } -> std::same_as<types::time_range>;
    { object.range_changed(std::function<void()>()) } -> std::same_as<void>;
};

constexpr auto view_on_price(const std::ranges::range auto &view, const model auto &model) {
    return view | std::ranges::views::transform([&model](auto &&val) -> float { return model.value(val).price; });
}

} // namespace provider

