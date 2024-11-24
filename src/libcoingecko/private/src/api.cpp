#include "api.hpp"

#include <curl/curl.h>
#include <nlohmann/json.hpp>

#include <expected>
#include <format>
#include <sstream>

using namespace nlohmann;

using error = coingecko::v3::error;

static size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp) {
    ((std::string *)userp)->append((char *)contents, size * nmemb);
    return size * nmemb;
}

namespace {

namespace network {
auto request(const std::string &url) -> std::expected<std::string, error> {
    std::string content;

    const auto curl = curl_easy_init();
    if (!curl)
        return std::unexpected(error::generic_error);

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &content);
    const auto res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);

    return content;
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
