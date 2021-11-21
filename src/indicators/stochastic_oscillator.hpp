#pragma once

#include "indicators.hpp"
#include <limits>
#include <ranges>
#include <iostream>

namespace indicator {
// https://www.investopedia.com/terms/s/stochasticoscillator.asp
struct stochastic_oscillator {
    stochastic_oscillator() {}

    static constexpr auto algorithm_type = type::stochastic_oscillator;

    auto compute(::ranges::range auto &&view, provider::model auto &&model) noexcept -> types::indicator_value {
        auto view_prices = view_on_price(view, model);
        float period_maximum_price = 0;
        float period_minimum_price = std::numeric_limits<float>::max();
        for (auto price : view_prices | ranges::views::reverse) {
            if(period_maximum_price < price)
                period_maximum_price = price;
            if(period_minimum_price > price)
                period_minimum_price = price;
        }

        if (period_maximum_price - period_minimum_price != 0 && size(view_prices) > 0)
            return {static_cast<float>(100.0*((*(view_prices.end()-1)-period_minimum_price)/(period_maximum_price - period_minimum_price)))};
        else
            return {100.0};
    }
};

} // namespace indicator
