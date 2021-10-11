#pragma once

#include "indicators.hpp"

namespace indicator {
struct exponential_moving_average {
    static constexpr auto algorithm_name = "EMA";

    constexpr auto compute(::ranges::range auto &&view, provider::model auto &&model) noexcept -> types::indicator_value {
        auto current_window_ema_sum = 0.0f;
        auto k_param_ref = (2.0f / (size(view) + 1));

        for (auto price : view_on_price(view, model) | ranges::views::reverse)
            current_window_ema_sum = price * k_param_ref + current_window_ema_sum * (1 - k_param_ref);

        return {current_window_ema_sum};
    };
};

} // namespace indicator
