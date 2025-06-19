#pragma once

#include <nlohmann/json.hpp>

#include <spdlog/spdlog.h>
#include <vector>

#include "common/configuration.hpp"
#include "helpers/threading.hpp"
#include "rest/requests.hpp"

using shared_logger = std::shared_ptr<spdlog::logger>;

namespace chain::hedera {

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
        try {
            auto response = request(std::format("https://mainnet-public.mirrornode.hedera.com/api/v1/accounts/{}", addr), {});
            if (!response) return std::vector<std::pair<std::string, double>>{};
            const auto balance = response->at("balance").at("balance").get<double>();
            return {{"hedera-hashgraph", balance / std::pow(10.0, 8)}};
        } catch (...) {
            return {{"hedera-hashgraph", 0.0}};
        }
    }});
};

} // namespace chain::hedera
