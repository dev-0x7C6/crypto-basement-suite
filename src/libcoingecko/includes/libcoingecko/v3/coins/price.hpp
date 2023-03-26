#pragma once

#include <string>
#include <unordered_map>
#include <vector>

using strings = std::vector<std::string>;

namespace coingecko::v3::coins {

struct price_query {
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

using prices = std::unordered_map<std::string, std::unordered_map<std::string, price>>;

auto price(const price_query & = {}) -> prices;

} // namespace coingecko::v3::coins
