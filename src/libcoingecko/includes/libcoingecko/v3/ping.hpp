#pragma once

#include <libcoingecko/v3/options.hpp>

namespace coingecko::v3 {
auto ping(const options &opts = {}) -> bool;
}
