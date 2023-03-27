#pragma once

#include <string>
#include <vector>

namespace coingecko::v3::coins::categories {

struct category {
    std::string id;
    std::string name;
};

auto list() -> std::vector<category>;

} // namespace coingecko::v3::coins::categories
