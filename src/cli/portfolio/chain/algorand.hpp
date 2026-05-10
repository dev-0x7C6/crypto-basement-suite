#pragma once

#include <expected>
#include <nlohmann/json.hpp>

#include <spdlog/spdlog.h>

#include "common/configuration.hpp"
#include "helpers/threading.hpp"
#include "interface/chain.hpp"
#include "rest/requests.hpp"

namespace chain::algorand {

enum class error {
    bad_request,
};

auto request(const std::string &url, const std::map<std::string, std::string> &header) -> std::expected<nlohmann::json, error> {
    const auto response = ::curl::request(url, header);
    if (!response) return std::unexpected(error::bad_request);
    return nlohmann::json::parse(response.value());
};

auto balance(const shared_logger &logger, const std::string &addr, const configuration &config) -> task<chain::results> {
    return schedule<chain::results>([addr, config]() -> chain::results {
        try {
            std::map<std::string, std::string> header;
            header["X-Algo-API-Token"] = config.scalar_api_key;
            auto response = request(std::format("https://mainnet-api.4160.nodely.dev/v2/accounts/{}", addr), std::move(header));
            if (!response) return std::unexpected<chain::error>(chain::error::unable_to_query);
            const auto balance = response->at("amount").get<double>();
            return std::map<std::string, double>{{std::string{"algorand"}, balance / std::pow(10.0, 6)}};
        } catch (...) {
            return std::unexpected(chain::error::data_unavailable);
        }
    });
};

} // namespace chain::algorand
