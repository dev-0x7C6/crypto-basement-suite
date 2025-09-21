#pragma once

#include <common/configuration.hpp>
#include <common/share.hpp>
#include <spdlog/spdlog.h>

namespace printers {

void shares(
    shares::shares_vec shares,
    shares::query_price_fn price,
    shares::query_24h_change_fn get_24h_change,
    const configuration &config,
    std::shared_ptr<spdlog::logger> logger);

}
