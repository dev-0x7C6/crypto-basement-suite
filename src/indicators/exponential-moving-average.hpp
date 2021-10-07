#pragma once

#include "indicators.hpp"

namespace indicator {
struct exponential_moving_average {
    static constexpr auto algorithm_name = "EMA";

    constexpr auto compute(::ranges::range auto &&view, provider::model auto &&model) noexcept -> types::indicator_value {
        float current_window_ema_sum = 0;
        double k_param_ref = (2.0 / (static_cast<int>(view.size()) + 1));
        for (auto it : view_on_price(view, model) | ranges::views::reverse | ranges::views::transform([&](auto &&price) -> float {
                                                                                            current_window_ema_sum = price * k_param_ref + current_window_ema_sum * (1 - k_param_ref);
                                                                                            return current_window_ema_sum; })) {}
        return {current_window_ema_sum};
    };
};

} // namespace indicator
