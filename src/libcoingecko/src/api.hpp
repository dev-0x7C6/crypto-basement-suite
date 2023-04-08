#pragma once

#include <libcoingecko/v3/options.hpp>
#include <nlohmann/json.hpp>
#include <unordered_map>

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

template <typename T, typename U>
auto to_map(const nlohmann::json &j, const std::string &key) -> std::unordered_map<T, U> {
    if (!j.contains(key)) return {};
    std::unordered_map<T, U> ret;
    for (auto &&[key, value] : j["key"].items())
        ret[key] = value.get<U>();
    return ret;
}

namespace coingecko::v3 {

auto request(const std::string &query, const options &opts = {}) -> nlohmann::json;

} // namespace coingecko::v3
