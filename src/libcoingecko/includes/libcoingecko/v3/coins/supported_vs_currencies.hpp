#pragma once

#include <libcoingecko/v3/options.hpp>

#include <unordered_map>
#include <vector>

using strings = std::vector<std::string>;

namespace coingecko::v3::coins {

auto supported_vs_currencies(const options &opts = {}) -> strings;

} // namespace coingecko::v3::coins
