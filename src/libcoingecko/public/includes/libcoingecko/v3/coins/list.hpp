#pragma once

#include <libcoingecko/v3/options.hpp>

#include <string>
#include <unordered_map>
#include <vector>

namespace coingecko::v3::coins::list {

struct settings {
    bool include_platform{true};
};

struct coin {
    std::string id;
    std::string symbol;
    std::string name;
    std::unordered_map<std::string, std::string> platforms;
};

using coins = std::vector<coin>;

auto query(const settings &s = {}, const options &opts = {}) -> std::expected<coins, error>;
} // namespace coingecko::v3::coins::list
