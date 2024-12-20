#include "share.hpp"

using namespace shares;

auto shares::calculate(const portfolio &portfolio, query_price_fn &&query_price, double total) -> std::vector<share> {
    std::vector<share> shares;

    for (auto &&[asset, balance] : portfolio) {
        const auto price = query_price(asset);
        if (!price) continue;

        const auto value = balance * price.value_or(0.0);

        shares.push_back({
            .asset = asset,
            .share = value / total * 100.0,
            .quantity = balance,
        });
    }

    return shares;
}
