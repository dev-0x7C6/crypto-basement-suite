#pragma once

#include <common/configuration.hpp>
#include <common/share.hpp>
#include <libcoingecko/v3/coins/markets.hpp>
#include <spdlog/spdlog.h>

namespace printers {

void infinite_supply(const std::vector<coingecko::v3::coins::market_data> &market_data,
    shares::query_price_fn price,
    const configuration &config, std::shared_ptr<spdlog::logger> logger);

void finite_supply(const std::vector<coingecko::v3::coins::market_data> &market_data,
    shares::query_price_fn price,
    const configuration &config, std::shared_ptr<spdlog::logger> logger);

} // namespace printers
