#pragma once

#include <model/model.hpp>

#include <list>
#include <memory>
#include <vector>

template <typename type>
concept container_with_size = requires(type object) {
    { object.size() } -> std::same_as<std::size_t>;
};

constexpr auto is_empty(container_with_size auto x) noexcept {
    return x.size() == 0;
}

template <typename return_type = std::size_t>
constexpr auto size(::ranges::range auto v) noexcept {
    return static_cast<return_type>(static_cast<std::size_t>(v.size()));
}

namespace indicator {

enum class type {
    moving_average, // MA
    moving_average_convergence_divergence, // MACD
    exponential_moving_average, // EMA

    price_velocity, // PV
    rate_of_change, // ROC
    relative_strength_index, // RSI
    stochastic_oscillator, // SO
};

constexpr auto to_string(const type value) noexcept -> const char * {
    switch (value) {
        case type::moving_average: return "MA";
        case type::moving_average_convergence_divergence: return "MACD";
        case type::exponential_moving_average: return "EMA";
        case type::price_velocity: return "PV";
        case type::rate_of_change: return "ROC";
        case type::relative_strength_index: return "RSI";
        case type::stochastic_oscillator: return "SO";
    }
}

template <typename type>
concept indicator_concept = requires(type object) {
    { object.algorithm_name } -> std::convertible_to<std::string>;
    { object.compute() } -> std::same_as<types::indicator_value>;
};

// https://starofmysore.com/top-crypto-trading-technical-indicators-to-use-on-primexbt/
// useless by itself, but with different indicators forms a strategy
// Some of his favored technical techniques are moving average divergence/convergence (MACD), on-balance volume and relative strength index (RSI).
struct bollinger_bands {
};


//Fibonacci Retracement
// indicator which works for the whole range , taking maximum and minimum in the window and computing the bands for the price

} // namespace indicator
