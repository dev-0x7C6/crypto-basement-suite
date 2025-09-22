#include "print-supply.hpp"

#include <format>
#include <ranges>
#include <set>

#include <helpers/formatter.hpp>

using namespace std;
using namespace std::ranges;

namespace printers {

void infinite_supply(const std::vector<coingecko::v3::coins::market_data> &market_data,
    shares::query_price_fn price,
    const configuration &config, std::shared_ptr<spdlog::logger> logger) {
    logger->info("\n+ assets with infinity supply:");

    auto distribution = list_coins_with_infinity_supply(market_data);
    std::ranges::sort(distribution, [](auto &&lhs, auto &&rhs) {
        return circulating_supply_ratio(lhs) > circulating_supply_ratio(rhs);
    });

    for (auto &&asset : distribution) {
        const auto coin_supply_ratio = circulating_supply_ratio(asset);
        const auto fmt_coin_supply_ratio = format::percent(coin_supply_ratio.value_or(0) * 100.00, 0.00, 100.00);

        const auto value = price(asset.id).value().second;
        const auto price = format::price(value, config) + format::to_symbol(config.preferred_currency);
        const auto price_linear_devaluation = format::price(value * coin_supply_ratio.value_or(0), config) + format::to_symbol(config.preferred_currency);

        logger->info(" -> {:>8}, asset: {}, linear devaluation: {} -> {}", fmt_coin_supply_ratio, asset.id, price, price_linear_devaluation);
    }
};

void finite_supply(const std::vector<coingecko::v3::coins::market_data> &market_data,
    shares::query_price_fn price,
    const configuration &config, std::shared_ptr<spdlog::logger> logger) {
    logger->info("\n+ assets with finite supply:");

    auto distribution = list_coins_with_finite_supply(market_data);
    std::ranges::sort(distribution, [](auto &&lhs, auto &&rhs) {
        return circulating_supply_ratio(lhs) > circulating_supply_ratio(rhs);
    });

    for (auto &&asset : distribution) {
        const auto coin_supply_ratio = circulating_supply_ratio(asset);
        const auto fmt_coin_supply_ratio = format::percent(coin_supply_ratio.value_or(0) * 100.00, 0.00, 100.00);

        const auto value = price(asset.id).value().second;
        const auto price = format::price(value, config) + format::to_symbol(config.preferred_currency);
        const auto price_linear_devaluation = format::price(value * coin_supply_ratio.value_or(0), config) + format::to_symbol(config.preferred_currency);

        logger->info(" -> {:>8}, asset: {}, linear devaluation: {} -> {}", fmt_coin_supply_ratio, asset.id, price, price_linear_devaluation);
    }
};

} // namespace printers
