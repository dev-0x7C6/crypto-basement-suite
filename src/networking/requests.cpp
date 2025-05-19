#include "rest/requests.hpp"

#include <format>
#include <ranges>

#include <curl/curl.h>

namespace {

size_t curl_append_stdstring(void *content, size_t size, size_t blocks, void *userp) {
    const auto content_size = size * blocks;
    auto data = reinterpret_cast<std::string *>(userp);
    data->append(reinterpret_cast<const char *>(content), content_size);
    return content_size;
}

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
} // namespace

namespace curl {

auto request(const std::string &url, const ::curl::vec_headers &headers) noexcept -> ::curl::content {
    std::string content;

    raii::curl request;
    if (!request)
        return std::unexpected(CURLE_FAILED_INIT);

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
        return std::unexpected(status);

    return content;
}

auto request(const std::string &url, const ::curl::map_headers &headers) noexcept -> ::curl::content {
    using namespace std::ranges;
    using namespace std::ranges::views;
    return request(url,
        headers | transform([](auto &&pair) { return std::format("{}: {}", pair.first, pair.second); }) | to<std::vector<std::string>>());
}

auto request(const std::string &url) noexcept -> ::curl::content {
    return request(url, ::curl::vec_headers{});
}

} // namespace curl
