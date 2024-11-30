#pragma once

#include "common/configuration.hpp"
#include "helpers/colors.hpp"

#include <algorithm>
#include <format>

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

auto percent(double value, double min = -10.0, double max = 10.0) {
    const auto c = std::clamp(value, min, max);
    auto x = hsl_to_rgb({120.0 * (c - min) / (max - min), 1.0, 1.0});
    return ansi_colorize(std::format("{:+.2f}%", value), x.r, x.g, x.b);
};

} // namespace format
