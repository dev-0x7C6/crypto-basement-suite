#pragma once

#include <expected>
#include <nlohmann/json.hpp>

#include <spdlog/spdlog.h>
#include <string>
#include <vector>

#include "common/configuration.hpp"
#include "helpers/threading.hpp"
#include "interface/chain.hpp"
#include "rest/requests.hpp"

namespace chain::hedera {

enum class error {
    bad_request,
};

auto request(const std::string &url, const std::map<std::string, std::string> &header) -> std::expected<nlohmann::json, error> {
    const auto response = ::curl::request(url, header);
    if (!response) return std::unexpected(error::bad_request);
    return nlohmann::json::parse(response.value());
};

auto balance(const shared_logger &logger, const std::string &addr, const configuration &) -> task<chain::results> {
    return schedule<chain::results>([addr]() -> chain::results {
        try {
            auto response = request(std::format("https://mainnet-public.mirrornode.hedera.com/api/v1/accounts/{}", addr), {});
            if (!response) return std::unexpected(chain::error::unable_to_query);
            const auto balance = response->at("balance").at("balance").get<double>();
            return std::map<std::string, double>{{"hedera-hashgraph", balance / std::pow(10.0, 8)}};
        } catch (...) {
            return std::unexpected(chain::error::data_unavailable);
        }
    });
};

} // namespace chain::hedera
