#pragma once

#include <string>
#include <unordered_map>
#include <vector>

using strings = std::vector<std::string>;

namespace coingecko::v3::coins {

auto supported_vs_currencies() -> strings;

} // namespace coingecko::v3::coins
