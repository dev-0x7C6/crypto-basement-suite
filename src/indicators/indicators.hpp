#pragma once

#include <model/model.hpp>

#include <list>
#include <memory>
#include <vector>

template <typename type>
concept container_with_size = requires(type object) {
    { object.size() } -> std::same_as<std::size_t>;
};

auto is_empty(container_with_size auto x) noexcept {
    return x.size() == 0;
}

namespace indicator {

template <typename type>
concept indicator_model = requires(type object) {
    { object.compute_value(types::time_point{}) } -> std::same_as<types::indicator_value>;
    { object.configure(std::list<std::tuple<std::string, std::string>>()) } -> std::same_as<void>;
    { object.load_data(types::currency{}) } -> std::same_as<void>;
};

// https://starofmysore.com/top-crypto-trading-technical-indicators-to-use-on-primexbt/
// useless by itself, but with different indicators forms a strategy
// Some of his favored technical techniques are moving average divergence/convergence (MACD), on-balance volume and relative strength index (RSI).
struct bollinger_bands {
};

// indicator with memory, need to rethink how to use it with current approach
// RSI 
struct relative_strength_index {
};

// similat to the above but without memory
struct stochastic_oscilator {
// https://www.investopedia.com/terms/s/stochasticoscillator.asp
}

//MACD 
struct ma_convergence_divergence {

}

//Fibonacci Retracement
// indicator which works for the whole range , taking maximum and minimum in the window and computing the bands for the price

} // namespace indicator
