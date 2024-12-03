#pragma once

#include <filesystem>
#include <map>
#include <nlohmann/json.hpp>
#include <string>

namespace cardano::registry {

namespace token {
struct details {
    double divisor{1.0};
};

using database = std::map<std::string, token::details>;
} // namespace token

auto scan(const std::filesystem::path &path) -> token::database;

} // namespace cardano::registry
