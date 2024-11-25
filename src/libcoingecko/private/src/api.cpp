#include "api.hpp"

#include <nlohmann/json.hpp>
#include <rest/requests.hpp>

using namespace nlohmann;

using error = coingecko::v3::error;

namespace network::json {

auto request(const std::string &url) -> std::expected<nlohmann::json, error> {
    const auto response = curl::request(url);
    if (!response) return std::unexpected(error::bad_request);

    return ::json::parse(response.value());
}
} // namespace network::json

namespace coingecko::v3 {

auto request(const std::string &query, const options &opts) -> std::expected<nlohmann::json, error> {
    const auto response = network::json::request(std::format("{}/{}", opts.provider, query));
    if (!response) return response;

    auto &&json = response.value();

    if (json.contains("status") && json["status"].contains("error_code"))
        return std::unexpected(static_cast<error>(json["status"].value("error_code", 0)));

    return json;
}
} // namespace coingecko::v3
