#pragma once

#include <expected>
#include <functional>
#include <map>
#include <optional>
#include <ranges>
#include <string>
#include <utility>
#include <vector>

#include <libcoingecko/v3/coins/markets.hpp>

using portfolio = std::map<std::string, double>;
using currency_quantity = std::pair<std::string, double>;

template <typename K, typename V>
auto value_or(const std::map<K, V> &in, const K &key, V &&v) {
    try {
        return in.at(key);
    } catch (...) {}
    return std::forward<V>(v);
}

namespace symbol {
constexpr auto eur = "eur";
constexpr auto pln = "pln";
constexpr auto usd = "usd";
constexpr auto btc = "btc";
} // namespace symbol

constexpr auto circulating_supply_ratio(const coingecko::v3::coins::market_data &details) -> std::optional<double> {
    if (details.circulating_supply && details.max_supply)
        return *details.circulating_supply / *details.max_supply;

    if (details.circulating_supply && details.total_supply)
        return *details.circulating_supply / *details.total_supply;

    return {};
}

constexpr auto list_coins_with_infinity_supply(const std::vector<coingecko::v3::coins::market_data> &coins) {
    using namespace coingecko::v3::coins;
    using namespace std::ranges::views;
    using namespace std::ranges;

    auto condition = [](auto &&data) -> bool {
        return !data.max_supply.has_value();
    };

    return coins | std::ranges::views::filter(std::move(condition)) | to<std::vector<market_data>>();
}

constexpr auto list_coins_with_finite_supply(const std::vector<coingecko::v3::coins::market_data> &coins) {
    using namespace coingecko::v3::coins;
    using namespace std::ranges::views;
    using namespace std::ranges;

    auto condition = [](auto &&data) -> bool {
        return data.max_supply.has_value();
    };

    return coins | filter(std::move(condition)) | to<std::vector<market_data>>();
}

namespace shares {

enum class error {
    unknown_asset_price,
};

struct share {
    std::string asset;
    double share{};
    double quantity{};
};

constexpr auto operator<=>(const share &lhs, const share &rhs) noexcept {
    return lhs.share > rhs.share;
}

using shares_vec = std::vector<share>;
using shares_map = std::map<std::string, share>;

using query_price_fn = std::function<std::optional<currency_quantity>(const std::string &asset)>;
using query_24h_change_fn = std::function<double(const std::string &asset)>;

auto calculate(const portfolio &portfolio, query_price_fn &&query_price, double total) -> std::expected<shares_vec, error>;
auto to_map(const shares_vec &) -> shares_map;
} // namespace shares
