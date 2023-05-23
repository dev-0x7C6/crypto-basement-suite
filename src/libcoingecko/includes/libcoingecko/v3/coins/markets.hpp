#pragma once

#include <libcoingecko/v3/options.hpp>

#include <string>
#include <unordered_map>
#include <vector>

namespace coingecko::v3::coins::markets {

enum class order {
    undefined,
    market_cap_asc,
    market_cap_desc,
    volume_asc,
    volume_desc,
    id_asc,
    id_desc
};

struct settings {
    std::string vs_currency;
    std::vector<std::string> ids;
    std::string category;
    std::string order;
    int per_page{100};
    int page{1};
    bool sparkline{true};
    std::vector<std::string> price_change_percentage;
    std::vector<std::string> locale;
};

struct info {
    std::string id;
    std::string symbol;
    std::string name;
    std::string image;
    double current_price{};
    double market_cap{};
    int market_cap_rank{};
    std::uint64_t fully_diluted_valuation{};
    std::uint64_t total_volume{};
    std::uint64_t high_24h{};
    std::uint64_t low_24h{};
    double price_change_24h{};
    double price_change_percentage_24h{};
    double market_cap_change_24h{};
};

auto markets(settings query = {}, const options &opts = {}) -> std::expected<info, error>;
} // namespace coingecko::v3::coins::markets
