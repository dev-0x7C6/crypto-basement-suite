#pragma once

#include <range/v3/to_container.hpp>
#include <range/v3/view/transform.hpp>

#include <libblockfrost/public/includes/libblockfrost/v0/balance.hpp>

#include <spdlog/spdlog.h>

#include "common/configuration.hpp"
#include "helpers/threading.hpp"

namespace chain::cardano {
auto balance(const std::string &addr, const configuration &config) -> task<std::vector<std::pair<std::string, double>>> {
    return schedule(std::function{[addr, opts{config.blockfrost}]() -> std::vector<std::pair<std::string, double>> {
        spdlog::info("blockfrost::v0: {}: requesting wallet balance", addr);
        const auto balance = blockfrost::v0::accounts_balance(addr, opts);

        if (!balance) {
            spdlog::error("blockfrost::v0: {}: unable to request", addr);
            return {{"cardano", 0.0}};
        }

        spdlog::info("blockfrost::v0: {}: balance {:.2f}", addr, balance.value());
        return {{"cardano", balance.value()}};
    }});
};

auto assets(const std::string &addr, const configuration &config) -> task<std::vector<std::pair<std::string, double>>> {
    return schedule(std::function{[addr, opts{config.blockfrost}]() -> std::vector<std::pair<std::string, double>> {
        spdlog::info("blockfrost::v0: {}: requesting wallet assets", addr);
        const auto ret = blockfrost::v0::accounts_assets_balance(addr, opts);
        spdlog::info("blockfrost::v0: {}: found {} assets", addr, ret.size());
        auto conversion = ret | ranges::views::transform([](const blockfrost::v0::asset &v) {
            return std::make_pair(v.unit, v.quantity);
        }) | ranges::to<std::vector>();

        for (auto &&[asset, quantity] : conversion)
            spdlog::info("blockfrost::v0: {}: asset {}, quantity {}", addr, asset, quantity);

        return conversion;
    }});
};
} // namespace chain::cardano

namespace chain {
using callback = std::function<decltype(chain::cardano::balance)>;
}
