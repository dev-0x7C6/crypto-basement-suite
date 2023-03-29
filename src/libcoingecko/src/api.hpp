#pragma once

#include <libcoingecko/v3/options.hpp>
#include <nlohmann/json.hpp>

template <typename T>
auto get(const nlohmann::json &j, const std::string &key) -> std::optional<T> {
    try {
        return j[key].get<T>();
    } catch (...) {};
    return {};
}

template <typename T>
auto set(const nlohmann::json &j, const std::string &key, T &out) -> void {
    out = get<T>(j, key).value_or(T{});
}

namespace coingecko::v3 {

auto request(const std::string &query, const options &opts = {}) -> nlohmann::json;

} // namespace coingecko::v3
