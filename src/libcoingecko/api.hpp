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

namespace network {

auto request(const std::string &url) -> std::optional<std::string> {
    using namespace curlpp;
    using namespace curlpp::options;
    try {
        Cleanup cleanup;
        Easy request;
        request.setOpt<Url>(url);

        std::stringstream stream;
        stream << request;
        return stream.str();
    }

    catch (RuntimeError &e) {
        std::cout << e.what() << std::endl;
        return {};
    }

    catch (LogicError &e) {
        std::cout << e.what() << std::endl;
        return {};
    }

    return {};
}

} // namespace network

namespace network::json {

nlohmann::json request(const std::string &url) {
    const auto response = network::request(url);
    if (!response) return {};

    return ::json::parse(response.value());
}
} // namespace network::json

namespace coingecko::v3 {

constexpr auto api = "https://api.coingecko.com/api/v3";

nlohmann::json request(const std::string &url) {
    const auto json = network::json::request(url);
    if (json.empty()) return {};

    if (json.contains("status") && json["status"].contains("error_code"))
        return {};

    return json;
}
} // namespace coingecko::v3
