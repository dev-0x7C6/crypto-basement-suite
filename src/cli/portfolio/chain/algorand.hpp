#pragma once

#include <nlohmann/json.hpp>

#include <spdlog/spdlog.h>
#include <vector>

#include "common/configuration.hpp"
#include "helpers/threading.hpp"
#include "rest/requests.hpp"

using shared_logger = std::shared_ptr<spdlog::logger>;

namespace chain::algorand {

enum class error {
    bad_request,
};

auto request(const std::string &url, const std::map<std::string, std::string> &header) -> std::expected<nlohmann::json, error> {
    const auto response = ::curl::request(url, header);
    if (!response) return std::unexpected(error::bad_request);
    return nlohmann::json::parse(response.value());
};

auto balance(const shared_logger &logger, const std::string &addr, const configuration &config) -> task<std::vector<std::pair<std::string, double>>> {
    return schedule(std::function<std::vector<std::pair<std::string, double>>()>{[addr, config]() -> std::vector<std::pair<std::string, double>> {
        try {
            std::map<std::string, std::string> header;
            header["X-Algo-API-Token"] = config.scalar_api_key;
            auto response = request(std::format("https://mainnet-api.4160.nodely.dev/v2/accounts/{}", addr), std::move(header));
            if (!response) return std::vector<std::pair<std::string, double>>{};
            const auto balance = response->at("amount").get<double>();
            return {{"algorand", balance / std::pow(10.0, 6)}};
        } catch (...) {
            return {{"algorand", 0.0}};
        }
    }});
};

} // namespace chain::algorand
