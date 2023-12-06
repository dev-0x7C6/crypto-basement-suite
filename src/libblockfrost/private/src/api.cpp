#include "api.hpp"

#include <curlpp/Easy.hpp>
#include <curlpp/Exception.hpp>
#include <curlpp/Options.hpp>
#include <curlpp/cURLpp.hpp>
#include <nlohmann/json.hpp>

#include <fmt/format.h>
#include <format>
#include <sstream>
#include <string>

using namespace nlohmann;

using error = blockfrost::v0::error;

namespace {

namespace network {
auto request(const std::string &url, const std::string &header) -> std::expected<std::string, error> {
    using namespace curlpp;
    using namespace curlpp::options;
    try {
        Cleanup cleanup;
        Easy request;
        request.setOpt<Url>(url);
        if (!header.empty())
            request.setOpt(new curlpp::options::HttpHeader({fmt::format("project_id: {}", header)}));

        std::stringstream stream;
        stream << request;
        return std::string(stream.str());
    }

    catch (RuntimeError &e) {
        std::cout << e.what() << std::endl;
        return std::unexpected(error::generic_error);
    }

    catch (LogicError &e) {
        std::cout << e.what() << std::endl;
        return std::unexpected(error::generic_error);
    }

    return std::unexpected(error::generic_error);
}
} // namespace network

namespace network::json {

auto request(const std::string &url, const std::string &header) -> std::expected<nlohmann::json, error> {
    const auto response = network::request(url, header);
    if (!response) return response;
    return ::json::parse(response.value());
}
} // namespace network::json

} // namespace

namespace blockfrost::v0 {

auto request(const std::string &query, const options &opts) -> std::expected<nlohmann::json, error> {
    const auto response = network::json::request(std::format("{}/{}", opts.provider, query), opts.key);
    return response;
}
} // namespace blockfrost::v0
