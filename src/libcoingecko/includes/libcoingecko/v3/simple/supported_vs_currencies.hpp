#pragma once

#include <libcoingecko/v3/options.hpp>

#include <unordered_map>
#include <vector>

namespace coingecko::v3::simple::supported_vs_currencies {

using strings = std::vector<std::string>;

auto query(const options &opts = {}) -> std::expected<strings, error>;

} // namespace coingecko::v3::simple::supported_vs_currencies
