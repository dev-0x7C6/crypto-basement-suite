#include "api.hpp"

#include <curl/curl.h>
#include <fmt/format.h>
#include <format>
#include <nlohmann/json.hpp>
#include <ranges>
#include <string>

using namespace nlohmann;

using error = blockfrost::v0::error;

static size_t curl_append_stdstring(void *content, size_t size, size_t blocks, void *userp) {
    const auto content_size = size * blocks;
    auto data = reinterpret_cast<std::string *>(userp);
    data->append(reinterpret_cast<const char *>(content), content_size);
    return content_size;
}

namespace {

namespace raii {
class curl {
public:
    curl()
            : handle(curl_easy_init()) {}
    ~curl() {
        if (is_valid())
            curl_easy_cleanup(handle);
    }

    constexpr auto is_valid() const -> bool { return handle != nullptr; }
    operator CURL *() const noexcept { return handle; }
    operator bool() const noexcept { return is_valid(); }

private:
    CURL *handle = nullptr;
};
} // namespace raii

namespace network {

auto request(const std::string &url, const std::vector<std::string> &headers = {}) -> std::expected<std::string, error> {
    std::string content;

    raii::curl request;
    if (!request)
        return std::unexpected(error::generic_error);

    curl_easy_setopt(request, CURLOPT_URL, url.c_str());
    curl_easy_setopt(request, CURLOPT_WRITEFUNCTION, curl_append_stdstring);
    curl_easy_setopt(request, CURLOPT_WRITEDATA, &content);

    if (!headers.empty()) {
        struct curl_slist *list = nullptr;
        for (auto &&header : headers)
            list = curl_slist_append(list, header.c_str());

        curl_easy_setopt(request, CURLOPT_HTTPHEADER, list);
    }

    const auto status = curl_easy_perform(request);

    if (CURLE_OK != status)
        return std::unexpected(error::generic_error);

    return content;
}

auto request(const std::string &url, const std::map<std::string, std::string> &header = {}) -> std::expected<std::string, error> {
    return request(url,
        header | std::ranges::views::transform([](auto &&pair) { return std::format("{}: {}", pair.first, pair.second); }) | std::ranges::to<std::vector<std::string>>());
}

} // namespace network

namespace network::json {

auto request(const std::string &url, const std::map<std::string, std::string> &header) -> std::expected<nlohmann::json, error> {
    const auto response = network::request(url, header);
    if (!response) return response;
    return ::json::parse(response.value());
}
} // namespace network::json

} // namespace

namespace blockfrost::v0 {

auto request(const std::string &query, const options &opts) -> std::expected<nlohmann::json, error> {
    const auto response = network::json::request(std::format("{}/{}", opts.provider, query), std::map<std::string, std::string>{{"project_id", opts.key}});
    return response;
}
} // namespace blockfrost::v0
