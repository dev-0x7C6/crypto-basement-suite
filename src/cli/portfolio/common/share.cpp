#include "share.hpp"

#include <ranges>

using namespace shares;

auto shares::calculate(const portfolio &portfolio, query_price_fn &&query_price, double total) -> shares_vec {
    shares_vec ret;

    for (auto &&[asset, balance] : portfolio) {
        const auto price = query_price(asset);
        if (!price) continue;

        const auto [currency, valution] = price.value();
        const auto value = balance * valution;

        ret.push_back({
            .asset = asset,
            .share = value / total * 100.0,
            .quantity = balance,
        });
    }

    return ret;
}

auto shares::to_map(const shares_vec &shares) -> shares_map {
    return shares |
        std::ranges::views::transform([](const shares::share &s) {
            return std::make_pair(s.asset, s);
        }) |
        std::ranges::to<std::map<std::string, shares::share>>();
}
