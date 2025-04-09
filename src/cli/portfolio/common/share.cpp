#include "share.hpp"

#include <ranges>

using namespace shares;

auto shares::calculate(const portfolio &portfolio, query_price_fn &&query_price, double total) -> std::expected<shares_vec, error> {
    shares_vec ret;

    for (auto &&[asset, balance] : portfolio) {
        const auto price = query_price(asset);
        if (!price) return std::unexpected(error::unknown_asset_price);

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

constexpr auto to_pair(const shares::share &share) -> std::pair<std::string, shares::share> {
    return std::make_pair(share.asset, share);
}

auto shares::to_map(const shares_vec &shares) -> shares_map {
    namespace rng = std::ranges;

    return //
        shares //
        | rng::views::transform(to_pair) //
        | rng::to<shares_map>();
}
