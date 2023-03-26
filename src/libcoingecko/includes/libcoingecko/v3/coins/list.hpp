#pragma once

#include <string>
#include <unordered_map>
#include <vector>

namespace coingecko::v3::coins {

struct coin {
    std::string id;
    std::string symbol;
    std::string name;
    std::unordered_map<std::string, std::string> platforms;
};

using coins = std::vector<coin>;

auto list(bool include_platform = true) -> coins;
} // namespace coingecko::v3::coins
