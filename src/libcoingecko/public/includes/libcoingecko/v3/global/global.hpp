#pragma once

#include <libcoingecko/v3/options.hpp>

#include <cstdint>
#include <expected>
#include <string>
#include <map>

namespace coingecko::v3::global {
struct data {
    double active_cryptocurrencies{};
    double upcoming_icos{};
    double ongoing_icos{};
    double ended_icos{};
    double markets{};

    std::map<std::string, double> total_market_cap;
    std::map<std::string, double> total_volume;
    std::map<std::string, double> market_cap_percentage;

    double market_cap_change_percentage_24h_usd{};
    std::uint64_t updated_at{};
};

auto list() -> std::expected<data, error>;

} // namespace coingecko::v3::global
