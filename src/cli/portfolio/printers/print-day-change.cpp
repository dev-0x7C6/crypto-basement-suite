#include "print-shares.hpp"

#include <format>
#include <ranges>

#include <helpers/formatter.hpp>

using namespace std;
using namespace std::ranges;

namespace printers {

void day_change(
    shares::shares_vec shares,
    shares::query_price_fn price,
    shares::query_24h_change_fn get_24h_change,
    const configuration &config,
    std::shared_ptr<spdlog::logger> logger) {

    std::ranges::sort(shares, [&](auto &&l, auto &&r) {
        return get_24h_change(l.asset) > get_24h_change(r.asset);
    });

    const auto preferred_currency_symbol = format::to_symbol(config.preferred_currency);

    logger->info("\n+ 24h change (sorted):");
    for (auto &&share : shares) {
        const auto percent = format::percent(get_24h_change(share.asset));
        const auto value = price(share.asset).value().second;
        const auto fvalue = format::price(value, {});
        logger->info(" {:>30}: {} [{}{}]",
            share.asset,
            percent,
            fvalue,
            preferred_currency_symbol);
    }
}

} // namespace printers
