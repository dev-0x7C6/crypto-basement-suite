#pragma once

#include <libblockfrost/v0/options.hpp>
#include <optional>
#include <string>

namespace blockfrost::v0 {
auto balance(const std::string &address, const options &opts = {}) -> std::optional<double>;
}
