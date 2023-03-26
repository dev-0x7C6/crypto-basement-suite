#pragma once

#include <optional>
#include <string>
#include <unordered_map>

namespace coingecko::v3::global {
struct data {
    double active_cryptocurrencies{};
    double upcoming_icos{};
    double ongoing_icos{};
    double ended_icos{};
    double markets{};

    std::unordered_map<std::string, double> total_market_cap;
    std::unordered_map<std::string, double> total_volume;
    std::unordered_map<std::string, double> market_cap_percentage;

    double market_cap_change_percentage_24h_usd{};
    uint64_t updated_at{};
};

auto list() -> std::optional<data>;

} // namespace coingecko::v3::global
