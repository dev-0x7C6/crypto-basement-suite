#pragma once

#include "indicators.hpp"

namespace indicator {

struct moving_average {
    static constexpr auto algorithm_type = type::moving_average;

    // needs outside moving data provide i.e.
    // (time range) | (skip every 60 sec) | (create buffers that sliding data by 1 in 25 window size)
    // model_range  | views::stride(60)   | views::sliding(25)
    // pros: multiple algos might benefit from same local data
    //
    // should we introduce struct that will enforce special type on caller side?
    // i.e. struct sliding_range{ range member }
    // in order to enforce some awareness on caller side
    //

    constexpr auto compute(::ranges::forward_range auto &&view, provider::model auto &&model) noexcept -> types::indicator_value {
        const auto sum = ::ranges::accumulate(view_on_price(view, model), 0.0f);
        return {sum / size(view)};
    }
};

} // namespace indicator
