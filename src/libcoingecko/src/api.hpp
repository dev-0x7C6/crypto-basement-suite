#pragma once

#include <curlpp/Easy.hpp>
#include <curlpp/Options.hpp>
#include <curlpp/cURLpp.hpp>
#include <nlohmann/json.hpp>

#include <sstream>

using namespace nlohmann;

template <typename T>
auto get(const json &j, const std::string &key) -> std::optional<T> {
    try {
        return j[key].get<T>();
    } catch (...) {};
    return {};
}

template <typename T>
auto set(const json &j, const std::string &key, T &out) -> void {
    out = get<T>(j, key).value_or(T{});
}

namespace coingecko::v3 {

constexpr auto api = "https://api.coingecko.com/api/v3";

auto request(const std::string &url) -> nlohmann::json;
} // namespace coingecko::v3
