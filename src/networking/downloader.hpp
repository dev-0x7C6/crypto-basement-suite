#pragma once

#include <optional>
#include <string>

namespace network {

auto request(const std::string &url) -> std::optional<std::string>;

} // namespace network
