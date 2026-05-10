#include "rest/requests.hpp"

#include <format>
#include <ranges>

#include <curl/curl.h>

namespace {

auto is_request_ok(const CURLcode status) {
    return CURLE_OK == status;
}

auto to_slist(const ::curl::vec_headers &headers) -> ::curl_slist * {
    ::curl_slist *ret{nullptr};
    for (auto &&header : headers)
        ret = curl_slist_append(ret, header.c_str());
    return ret;
}

auto curl_append_stdstring(void *content, size_t size, size_t blocks, void *userp) -> size_t {
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
    raii::curl request;
    if (!request)
        return std::unexpected(CURLE_FAILED_INIT);

    std::string content{};
    content.reserve(4096);

    curl_easy_setopt(request, CURLOPT_URL, url.c_str());
    curl_easy_setopt(request, CURLOPT_WRITEFUNCTION, curl_append_stdstring);
    curl_easy_setopt(request, CURLOPT_WRITEDATA, &content);
    curl_easy_setopt(request, CURLOPT_USERAGENT, "Mozilla/5.0 (X11; Linux x86_64; rv:146.0) Gecko/20100101 Firefox/146.0");
    curl_easy_setopt(request, CURLOPT_HTTPHEADER, to_slist(headers));

    const auto status = curl_easy_perform(request);

    if (!is_request_ok(status))
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
