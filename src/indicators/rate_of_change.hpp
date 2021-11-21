#pragma once

#include "indicators.hpp"

// https://www.investopedia.com/terms/p/pricerateofchange.asp

namespace indicator {
struct rate_of_change {
    static constexpr auto algorithm_type = type::rate_of_change;

    constexpr auto compute(::ranges::range auto &&view, provider::model auto &&model) noexcept -> types::indicator_value {
        auto view_prices = view_on_price(view, model);
        return {(*(view_prices.end() - 1) - *view_prices.begin()) / (*view_prices.begin()) * 100.0f};
    };
};

} // namespace indicator
