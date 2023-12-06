#pragma once

#include <libblockfrost/v0/options.hpp>
#include <optional>
#include <string>

namespace blockfrost::v0 {
auto address_balance(const std::string &address, const options &opts = {}) -> std::optional<double>;
auto accounts_balance(const std::string &address, const options &opts = {}) -> std::optional<double>;
}
