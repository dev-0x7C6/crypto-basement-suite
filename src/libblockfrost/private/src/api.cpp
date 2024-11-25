#include "api.hpp"

#include <rest/requests.hpp>

using namespace nlohmann;

using error = blockfrost::v0::error;

namespace network::json {

auto request(const std::string &url, const std::map<std::string, std::string> &header) -> std::expected<nlohmann::json, error> {
    const auto response = ::curl::request(url, header);
    if (!response) return std::unexpected(error::bad_request);
    return ::json::parse(response.value());
}
} // namespace network::json

namespace blockfrost::v0 {

auto request(const std::string &query, const options &opts) -> std::expected<nlohmann::json, error> {
    const auto response = network::json::request(std::format("{}/{}", opts.provider, query), std::map<std::string, std::string>{{"project_id", opts.key}});
    return response;
}
} // namespace blockfrost::v0
