#pragma once

#include <nlohmann/json.hpp>

#include <spdlog/spdlog.h>
#include <vector>

#include "common/configuration.hpp"
#include "helpers/threading.hpp"
#include "rest/requests.hpp"

using shared_logger = std::shared_ptr<spdlog::logger>;

namespace chain::bitcoin {

enum class error {
    bad_request,
};

auto request(const std::string &url, const std::map<std::string, std::string> &header) -> std::expected<nlohmann::json, error> {
    const auto response = ::curl::request(url, header);
    if (!response) return std::unexpected(error::bad_request);
    return nlohmann::json::parse(response.value());
};

auto balance(const shared_logger &logger, const std::string &addr, const configuration &) -> task<std::vector<std::pair<std::string, double>>> {
    return schedule(std::function<std::vector<std::pair<std::string, double>>()>{[addr]() -> std::vector<std::pair<std::string, double>> {
        auto response = request(std::format("https://api.blockcypher.com/v1/btc/main/addrs/{}/balance", addr), {});
        if (!response) return std::vector<std::pair<std::string, double>>{};
        return {{"bitcoin", response->value("final_balance", 0.0) / std::pow(10.0, 8)}};
    }});
};

} // namespace chain::bitcoin
