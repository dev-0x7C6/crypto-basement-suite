#include "print-shares.hpp"

#include <format>
#include <ranges>

#include <helpers/formatter.hpp>

using namespace std;
using namespace std::ranges;

namespace printers {

void shares(
    shares::shares_vec shares,
    shares::query_price_fn price,
    shares::query_24h_change_fn get_24h_change,
    const configuration &config,
    std::shared_ptr<spdlog::logger> logger) {

    std::ranges::sort(shares, std::greater<shares::share>());

    logger->info("\n+ shares");
    for (auto &&s : shares) {
        const auto value = price(s.asset).value().second * s.quantity;
        const auto percent = format::percent(get_24h_change(s.asset));
        const auto share = format::share(s.share, config);
        const auto price = format::price(value, config);

        logger->info(" {:>20}: {}, {}{}, 24h: {}",
            s.asset,
            share,
            price,
            format::to_symbol(config.preferred_currency),
            percent);
    }
}

} // namespace printers
