#pragma once

#include <libcoingecko/v3/options.hpp>

#include <vector>

namespace coingecko::v3::coins::categories {

struct category {
    std::string id;
    std::string name;
};

using categories = std::vector<category>;

auto list(const options &opts = {}) -> std::expected<categories, error>;

} // namespace coingecko::v3::coins::categories
