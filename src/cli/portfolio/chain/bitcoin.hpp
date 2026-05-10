#pragma once

#define SIMDJSON_STATIC_REFLECTION 1

#include <nlohmann/json.hpp>
#include <simdjson.h>

#include <spdlog/spdlog.h>
#include <vector>

#include "common/configuration.hpp"
#include "helpers/threading.hpp"
#include "interface/chain.hpp"
#include "rest/requests.hpp"

namespace chain::bitcoin {

enum class error {
    bad_request,
};

auto request(const std::string &url, const std::map<std::string, std::string> &header) //
    -> std::expected<nlohmann::json, error> {
    const auto response = ::curl::request(url, header);
    if (!response) return std::unexpected(error::bad_request);
    return nlohmann::json::parse(response.value());
};

auto balance(const shared_logger &logger, const std::string &addr, const configuration &) -> task<chain::results> {
    return schedule<chain::results>([addr]() -> chain::results {
        try {
            auto response = request(std::format("https://api.blockcypher.com/v1/btc/main/addrs/{}/balance", addr), {});
            if (!response) return std::unexpected<chain::error>(chain::error::unable_to_query);
            return std::map<std::string, double>{{"bitcoin", response->value("final_balance", 0.0) / std::pow(10.0, 8)}};
        } catch (...) {
            return std::unexpected<chain::error>(chain::error::data_unavailable);
        }
    });
};

} // namespace chain::bitcoin
