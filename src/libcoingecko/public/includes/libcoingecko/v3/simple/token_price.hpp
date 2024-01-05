#pragma once

#include <libcoingecko/v3/options.hpp>

#include <map>
#include <string>
#include <vector>

namespace coingecko::v3::simple::token_price {

struct parameters {
    std::string id; // assets
    std::vector<std::string> contract_addresses; //
    std::vector<std::string> vs_currencies; // usd
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

using prices = std::map<std::string, std::map<std::string, struct price>>;

auto query(const options &opts = {}) -> std::expected<std::vector<std::string>, error>;

} // namespace coingecko::v3::simple::token_price
