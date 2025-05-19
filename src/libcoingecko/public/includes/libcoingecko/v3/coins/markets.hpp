#pragma once

#include <libcoingecko/v3/options.hpp>

#include <cstdint>
#include <map>
#include <optional>
#include <set>
#include <string>
#include <vector>

namespace coingecko::v3::coins {

enum class order {
    undefined,
    market_cap_asc,
    market_cap_desc,
    volume_asc,
    volume_desc,
    id_asc,
    id_desc
};

struct markets_query {
    std::string vs_currency;
    std::set<std::string> ids;
    std::string category;
    std::string order;
    int per_page{100};
    int page{1};
    bool sparkline{true};
    std::vector<std::string> price_change_percentage;
    std::vector<std::string> locale;
};

struct market_data {
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
    double market_cap_change_percentage_24h{};
    std::optional<double> circulating_supply{};
    std::optional<double> total_supply{};
    std::optional<double> max_supply{};
    double ath{};
    double ath_change_percentage{};
    std::string ath_date;
    double atl{};
    double atl_change_percentage{};
    std::optional<double> roi;
    std::string last_updated;

    // additional
    double supply_ratio{};
};

auto markets(const markets_query &query = {}, const options &opts = {}) -> std::expected<std::vector<market_data>, error>;

} // namespace coingecko::v3::coins

auto to_pairs(const std::vector<coingecko::v3::coins::market_data> &) -> std::vector<std::pair<std::string, coingecko::v3::coins::market_data>>;
auto to_map(const std::vector<coingecko::v3::coins::market_data> &) -> std::map<std::string, coingecko::v3::coins::market_data>;
