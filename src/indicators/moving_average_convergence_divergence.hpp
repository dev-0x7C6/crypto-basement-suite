#pragma once

#include "indicators/indicators.hpp"

namespace indicator {
// MACD
//  for MACD we need two EMA indicators,
//  signal lines are created by computing for example moving average of indicator
//  in this example: MA from MACD => signal line
//   when MACD is above signal line, it means bullish market, if below - bearish market
// https://www.investopedia.com/terms/s/signal_line.asp
struct ma_convergence_divergence {
    ma_convergence_divergence(types::indicator_settings settings = {})
            : percentage(settings.macd_ema_percentage.value_or(0.6)) {}

    static constexpr auto algorithm_type = type::moving_average_convergence_divergence;

    auto compute(std::ranges::range auto &&view, provider::model auto &&model) noexcept -> types::indicator_value {
        int short_ema_start = size(view) - static_cast<float>(size(view)) * percentage;
        float k_param_long = (2.0 / (1.0 + size(view)));
        float k_param_short = (2.0 / (1.0 + static_cast<float>(size(view)) * percentage));
        float long_ema_window_sum = 0;
        float short_ema_window_sum = 0;
        float ema_start_counter = 0;
        for (auto price : view_on_price(view, model) | std::ranges::views::reverse) {
            long_ema_window_sum = price * k_param_long + long_ema_window_sum * (1 - k_param_long);
            if (ema_start_counter >= short_ema_start) {
                short_ema_window_sum = price * k_param_short + short_ema_window_sum * (1 - k_param_short);
            }
            ema_start_counter++;
        }
        return {static_cast<float>(short_ema_window_sum - long_ema_window_sum)};
    }

private:
    const float percentage;
};

} // namespace indicator
