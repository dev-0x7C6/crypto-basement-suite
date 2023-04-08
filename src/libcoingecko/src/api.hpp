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

template <typename key_type, typename value_type>
auto to_map(const nlohmann::json &j, const std::string &key) -> std::unordered_map<key_type, value_type> {
    if (!j.contains(key)) return {};
    std::unordered_map<key_type, value_type> ret;
    for (auto &&[key, value] : j[key].items())
        ret[key] = value.template get<value_type>();
    return ret;
}

template <>
inline auto set<std::unordered_map<std::string, double>>(const nlohmann::json &j, const std::string &key, std::unordered_map<std::string, double> &out) -> void {
    out = to_map<std::string, double>(j, key);
}

template <>
inline auto set<std::unordered_map<std::string, std::string>>(const nlohmann::json &j, const std::string &key, std::unordered_map<std::string, std::string> &out) -> void {
    out = to_map<std::string, std::string>(j, key);
}

namespace coingecko::v3 {

auto request(const std::string &query, const options &opts = {}) -> nlohmann::json;

} // namespace coingecko::v3
