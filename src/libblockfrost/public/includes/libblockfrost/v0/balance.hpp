#pragma once

#include <cstdint>
#include <libblockfrost/v0/options.hpp>
#include <optional>
#include <string>
#include <vector>

namespace blockfrost::v0 {

struct asset {
    std::string unit;
    double quantity{};
};

auto address_balance(const std::string &address, const options &opts = {}) -> std::optional<double>;
auto accounts_balance(const std::string &address, const options &opts = {}) -> std::optional<double>;
auto accounts_assets_balance(const std::string &stake_key, const options &opts = {}) -> std::optional<std::vector<asset>>;
}
