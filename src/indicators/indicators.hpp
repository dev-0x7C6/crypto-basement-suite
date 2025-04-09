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
constexpr auto size(std::ranges::range auto v) noexcept {
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
    last // put always as last
};

template <typename T>
concept indicator_concept = requires(T t) {
    typename T::algorithm_type;
    // { t::algorithm_type } -> std::same_as<indicator::type>;
    // { object.compute() } -> std::same_as<types::indicator_value>;
};

static constexpr auto types = std::array<type, static_cast<std::size_t>(type::last)>{
    type::moving_average,
    type::moving_average_convergence_divergence,
    type::exponential_moving_average,
    type::price_velocity,
    type::rate_of_change,
    type::relative_strength_index,
    type::stochastic_oscillator,
};

constexpr auto index(const type value) noexcept {
    return static_cast<std::size_t>(value);
}

class indicator_results {
public:
    auto at(const type value) noexcept -> std::vector<types::indicator_value> & {
        return dataset[static_cast<std::size_t>(value)];
    }

    template <indicator_concept indicator_type, typename... args>
    auto collect(const indicator_type &ind, args &&...values) {
        auto ret = ind.compute(std::forward<args>(values)...);
        collect(ind.algorithm_type, std::move(ret));
    }

    auto collect(const type t, const types::indicator_value &value) noexcept {
        operator[](t).emplace_back(value);
    }

    auto last(const type t) noexcept {
        return operator[](t).back();
    }

    auto operator[](const type value) -> std::vector<types::indicator_value> & {
        return dataset[static_cast<std::size_t>(value)];
    }

private:
    std::array<std::vector<types::indicator_value>, static_cast<std::size_t>(type::last)> dataset;
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
        case type::last: return nullptr;
    }

    return nullptr;
}

// https://starofmysore.com/top-crypto-trading-technical-indicators-to-use-on-primexbt/
// useless by itself, but with different indicators forms a strategy
// Some of his favored technical techniques are moving average divergence/convergence (MACD), on-balance volume and relative strength index (RSI).
struct bollinger_bands {
};

//Fibonacci Retracement
// indicator which works for the whole range , taking maximum and minimum in the window and computing the bands for the price

} // namespace indicator
