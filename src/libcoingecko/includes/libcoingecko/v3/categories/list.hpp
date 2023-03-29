#pragma once

#include <libcoingecko/v3/options.hpp>

#include <vector>

namespace coingecko::v3::coins::categories {

struct category {
    std::string id;
    std::string name;
};

auto list(const options &opts = {}) -> std::vector<category>;

} // namespace coingecko::v3::coins::categories
