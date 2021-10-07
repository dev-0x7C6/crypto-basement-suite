#pragma once

#include "indicators.hpp"

// https://www.investopedia.com/articles/technical/081501.asp

namespace indicator {
struct price_velocity {
    static constexpr auto algorithm_name = "PV";

    constexpr auto compute(::ranges::range auto &&view, provider::model auto &&model) noexcept -> types::indicator_value {
        auto view_prices = view_on_price(view, model);
        return {*(view_prices.end() - 1) - *view_prices.begin()};
    };
};

} // namespace indicator
