#pragma once

#include "common/configuration.hpp"
#include "common/share.hpp"
#include "libcoingecko/v3/simple/price.hpp"

auto show_gui_charts(int argc, char **argv, const configuration &config, const shares::shares_vec &shares, const coingecko::v3::simple::price::prices &summary) -> int;
