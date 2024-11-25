#pragma once

#include <expected>
#include <map>
#include <string>
#include <vector>

#include <curl/curl.h>

namespace curl {

using content = std::expected<std::string, CURLcode>;

using vec_headers = std::vector<std::string>;
using map_headers = std::map<std::string, std::string>;

auto request(const std::string &url) noexcept -> content;
auto request(const std::string &url, const vec_headers &headers) noexcept -> content;
auto request(const std::string &url, const map_headers &headers) noexcept -> content;

} // namespace curl
