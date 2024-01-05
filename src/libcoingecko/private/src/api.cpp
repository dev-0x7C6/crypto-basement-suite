#include "api.hpp"

#include <curlpp/Easy.hpp>
#include <curlpp/Exception.hpp>
#include <curlpp/Options.hpp>
#include <curlpp/cURLpp.hpp>
#include <expected>
#include <nlohmann/json.hpp>

#include <format>
#include <sstream>

using namespace nlohmann;

using error = coingecko::v3::error;

namespace {

namespace network {
auto request(const std::string &url) -> std::expected<std::string, error> {
    using namespace curlpp;
    using namespace curlpp::options;
    try {
        Cleanup cleanup;
        Easy request;
        request.setOpt<Url>(url);

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

auto request(const std::string &url) -> std::expected<nlohmann::json, error> {
    const auto response = network::request(url);
    if (!response) return response;

    return ::json::parse(response.value());
}
} // namespace network::json

} // namespace

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
