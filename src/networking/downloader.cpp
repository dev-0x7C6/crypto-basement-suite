#include "downloader.hpp"

#include <curl/curl.h>
#include <sstream>

static size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp) {
    ((std::string *)userp)->append((char *)contents, size * nmemb);
    return size * nmemb;
}

namespace network {
auto request(const std::string &url) -> std::optional<std::string> {
    std::string content;

    const auto curl = curl_easy_init();
    if (!curl)
        return {};

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &content);
    const auto res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);

    return content;
}
} // namespace network
