#pragma once

#include "common/configuration.hpp"
#include "helpers/colors.hpp"

#include <algorithm>
#include <format>
#include <functional>
#include <map>
#include <range/v3/algorithm/sort.hpp>
#include <utility>

namespace format {

auto share(double value, const configuration &cfg) noexcept -> std::string {
    if (cfg.hide.shares)
        return "---%";

    return std::format("{:.2f}%", value);
}

auto price(double value, const configuration &cfg) noexcept -> std::string {
    if (cfg.hide.balances)
        return "---";

    return std::format("{:.2f}", value);
}

auto to_symbol(const std::string &in) -> std::string {
    static const std::map<std::string, std::string> symbols{
        {"btc", "₿"}, //
        {"eur", "€"}, //
        {"usd", "$"}, //
        {"sats", "s₿"}, //
    };

    if (symbols.contains(in))
        return symbols.at(in);

    return in;
};

auto magnitude_ranks() {
    using rank = std::pair<int, std::string>;

    static std::vector<rank> ret{
        {6, "mln"},
        {3, "k"},
    };

    ranges::sort(ret, std::greater<rank>());
    return ret;
}

auto formatted_price(double value, const configuration &cfg, const bool hide_ranks = false, const int decimal = 2) noexcept -> std::string {
    if (cfg.hide.balances)
        return "---";

    auto leading_decimals = [](const double v) {
        if (v < 1.0)
            return 1;

        return static_cast<int>(std::log10(std::abs(v))) + 1;
    };

    const static auto ranks = magnitude_ranks();

    try {
        const auto count = leading_decimals(value);

        if (!hide_ranks)
            for (auto &&[rank, symbol] : ranks)
                if (count > rank)
                    return std::format("{:.{}f} {}", value / std::pow(10, rank), decimal, symbol);

        return std::format("{:.{}f}", value, decimal);
    } catch (...) {}

    return std::format("{:.{}f}", value, decimal);
}

auto percent(double value, double min = -10.0, double max = 10.0) {
    const auto c = std::clamp(value, min, max);
    auto x = hsl_to_rgb({120.0 * (c - min) / (max - min), 1.0, 1.0});
    return ansi_colorize(std::format("{:+.2f}%", value), x.r, x.g, x.b);
};

} // namespace format
