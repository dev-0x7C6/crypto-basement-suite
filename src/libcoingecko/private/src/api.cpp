#include "api.hpp"

#include <format>
#include <nlohmann/json.hpp>
#include <rest/requests.hpp>
#include <string>

using namespace nlohmann;

using error = coingecko::v3::error;

namespace coingecko::v3 {

auto raw_request(const std::string &url, const options &opts) -> std::expected<nlohmann::json, error> {
    curl::map_headers headers{
        {std::format("x-cg-{}-api-key", opts.demo ? "demo" : "pro"), opts.key},
    };

    const auto response = curl::request(url, std::move(headers));
    if (!response) return std::unexpected(error::bad_request);

    return ::json::parse(response.value());
}

auto request(const std::string &query, const options &opts) -> std::expected<nlohmann::json, error> {
    const auto response = raw_request(std::format("{}/{}", opts.provider, query), opts);
    if (!response) return response;

    auto &&json = response.value();

    if (json.contains("status") && json["status"].contains("error_code"))
        return std::unexpected(static_cast<error>(json["status"].value("error_code", 0)));

    return json;
}
} // namespace coingecko::v3
