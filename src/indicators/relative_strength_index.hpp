#pragma once

#include "indicators.hpp"

namespace indicator {
//https://www.investopedia.com/terms/p/pricerateofchange.asp
// additional parameters:
// https://en.wikipedia.org/wiki/Relative_strength_index
// https://www.macroption.com/rsi-calculation/
struct relative_strength_index {
    relative_strength_index() {}

    static constexpr auto algorithm_type = type::relative_strength_index;

    //nature of the RSI demands one more data point than the window size! for example for RSI 20 you will need 21 data points
    auto compute(std::ranges::range auto &&view, provider::model auto &&model) noexcept -> types::indicator_value {
        float ratio_of_change = 0;
        std::optional<float> previous_price;
        float previous_gains_sum = 0;
        float previous_losses_sum = 0;
        for (auto price : view_on_price(view, model) | std::ranges::views::reverse) {
            if (!previous_price.has_value()) {
                previous_price = price;
            } 
            else {
                ratio_of_change = price - previous_price.value();
                if(ratio_of_change > 0) {
                    previous_gains_sum += ratio_of_change; 
                } else {
                    previous_losses_sum += -ratio_of_change;
                }
            }
        }
        if (previous_losses_sum > 0 && size(view) > 0)
                    return {static_cast<float>(100.0-(100.0/(1.0+((previous_gains_sum/static_cast<float>(size(view) - 1))/
                            (previous_losses_sum/static_cast<float>(size(view) - 1))))))};
                else
                    return {100.0};
    }
};

} // namespace indicator
