#pragma once

#include <array>
#include <string>

namespace moon {
enum class phase {
    new_moon,
    waxing_crescent_moon,
    first_quarter_moon,
    waxing_gibbous_moon,
    full_moon,
    waning_gibbous_moon,
    last_quarter_moon,
    waning_crescent_moon,
};

constexpr auto phases = std::array<phase, 9>{
    phase::new_moon,
    phase::waxing_crescent_moon,
    phase::first_quarter_moon,
    phase::waxing_gibbous_moon,
    phase::full_moon,
    phase::waning_gibbous_moon,
    phase::last_quarter_moon,
    phase::waning_crescent_moon,
    phase::new_moon,
};

namespace symbol {
constexpr auto new_moon = "🌑";
constexpr auto waxing_crescent_moon = "🌒";
constexpr auto first_quarter_moon = "🌓";
constexpr auto waxing_gibbous_moon = "🌔";
constexpr auto full_moon = "🌕";
constexpr auto waning_gibbous_moon = "🌖";
constexpr auto last_quarter_moon = "🌗";
constexpr auto waning_crescent_moon = "🌘";
} // namespace symbol

auto to_string(const phase value) -> std::string {
    switch (value) {
        case phase::new_moon: return symbol::new_moon;
        case phase::waxing_crescent_moon: return symbol::waxing_crescent_moon;
        case phase::first_quarter_moon: return symbol::first_quarter_moon;
        case phase::waxing_gibbous_moon: return symbol::waxing_gibbous_moon;
        case phase::full_moon: return symbol::full_moon;
        case phase::waning_gibbous_moon: return symbol::waning_gibbous_moon;
        case phase::last_quarter_moon: return symbol::last_quarter_moon;
        case phase::waning_crescent_moon: return symbol::waning_crescent_moon;
    }

    return "?";
}

constexpr auto full_cycle_in_days = 29.5;
} // namespace moon
