#pragma once

#include <libcoingecko/v3/options.hpp>

#include <unordered_map>
#include <vector>

namespace coingecko::v3::coins {

using strings = std::vector<std::string>;

auto supported_vs_currencies(const options &opts = {}) -> std::expected<strings, error>;

} // namespace coingecko::v3::coins
