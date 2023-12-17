#include "libblockfrost/v0/balance.hpp"
#include "api.hpp"
#include <fmt/format.h>
#include <string>

auto blockfrost::v0::address_balance(const std::string &address, const options &opts) -> std::optional<double> {
    const auto json = request(fmt::format("addresses/{}", address), opts);
    if (!json) return {};

    auto &&data = *json;
    for (auto &&entry : data["amount"]) {
        if (entry.value("unit", "") == "lovelace")
            return std::stoll(entry.value("quantity", "0"), 0) / 1000000.0;
    }

    return {};
}

auto blockfrost::v0::accounts_balance(const std::string &address, const options &opts) -> std::optional<double> {
    const auto json = request(fmt::format("accounts/{}", address), opts);
    if (!json) return {};
    return std::stoull(json->value("controlled_amount", "0")) / 1000000.0;
}

auto blockfrost::v0::accounts_assets_balance(const std::string &stake_key, const options &opts) -> std::vector<asset> {
    const auto json = request(fmt::format("accounts/{}/addresses/assets", stake_key), opts);
    if (!json) return {};

    std::vector<asset> ret;
    for (auto &&entry : json.value()) {
        const auto unit = entry.value("unit", "");
        const auto quantity = entry.value("quantity", "0");
        ret.emplace_back(asset{
            .unit = unit,
            .quantity = static_cast<double>(std::stoull(quantity)),
        });
    }

    return ret;
}
