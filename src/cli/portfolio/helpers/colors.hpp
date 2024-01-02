#pragma once

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <format>

struct hsv {
    double h{};
    double s{};
    double v{};
};

struct rgb {
    std::uint8_t r{};
    std::uint8_t g{};
    std::uint8_t b{};
};

constexpr auto is_valid(const hsv v) -> bool {
    return (v.h >= 0.0 && v.h <= 360.0) &&
        (v.s >= 0.0 && v.s <= 1.0) &&
        (v.v >= 0.0 && v.v <= 1.0);
}

constexpr auto hsl_to_rgb(const hsv v) -> rgb {
    if (!is_valid(v))
        return {};

    const auto pc = v.v * v.s;
    const auto px = pc * (1 - std::fabs(std::fmod(v.h / 60.0, 2) - 1));

    const auto c = static_cast<std::uint8_t>(std::clamp(pc * 255.0, 0.0, 255.0));
    const auto x = static_cast<std::uint8_t>(std::clamp(px * 255.0, 0.0, 255.0));

    if (v.h >= 0 && v.h < 60) {
        return {c, x, 0};
    } else if (v.h >= 60 && v.h < 120) {
        return {x, c, 0};
    } else if (v.h >= 120 && v.h < 180) {
        return {0, c, x};
    } else if (v.h >= 180 && v.h < 240) {
        return {0, x, c};
    } else if (v.h >= 240 && v.h < 300) {
        return {x, 0, c};
    }

    return {c, 0, x};
}

constexpr auto ansi_colorize(auto &&text, std::uint8_t r, std::uint8_t g, std::uint8_t b) {
    return std::format("\x1b[38;2;{};{};{}m{}\033[m", r, g, b, text);
};
