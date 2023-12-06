#pragma once

#include <libcoingecko/v3/options.hpp>

#include <unordered_map>
#include <vector>

using strings = std::vector<std::string>;

namespace coingecko::v3::simple::price {

struct parameters {
    strings ids; // assets
    strings vs_currencies; // usd
    bool include_market_cap{true};
    bool include_24hr_vol{true};
    bool include_24hr_change{true};
    bool include_last_updated_at{true};
    uint precision{}; // 0 - 18;
};

struct price {
    double value{};
    double market_cap{};
    double volume_24h{};
    double change_24h{};
};

using prices = std::unordered_map<std::string, std::unordered_map<std::string, struct price>>;

auto query(const parameters & = {}, const options & = {}) -> std::expected<prices, error>;

} // namespace coingecko::v3::simple::price
