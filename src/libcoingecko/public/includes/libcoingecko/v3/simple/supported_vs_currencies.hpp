#pragma once

#include <libcoingecko/v3/options.hpp>

#include <set>
#include <string>

namespace coingecko::v3::simple::supported_vs_currencies {

auto query(const options &opts = {}) -> std::expected<std::set<std::string>, error>;

} // namespace coingecko::v3::simple::supported_vs_currencies
