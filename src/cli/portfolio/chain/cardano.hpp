#pragma once

#include <range/v3/to_container.hpp>
#include <range/v3/view/transform.hpp>

#include <libblockfrost/public/includes/libblockfrost/v0/balance.hpp>

#include <spdlog/spdlog.h>

#include "cli/cli.hpp"
#include "helpers/threading.hpp"

namespace chain::cardano {
auto balance(const std::string &address, const configuration &config) -> task<std::vector<std::pair<std::string, double>>> {
    return schedule(std::function{[address, opts{config.blockfrost}]() -> std::vector<std::pair<std::string, double>> {
        spdlog::info("blockfrost::v0: requesting wallet balance {}", address);
        const auto balance = blockfrost::v0::accounts_balance(address, opts);

        if (!balance) {
            spdlog::error("blockfrost::v0: unable to request {}", address);
            return {{"cardano", 0.0}};
        }

        spdlog::info("blockfrost::v0: {}, balance {:.2f}", address, balance.value());
        return {{"cardano", balance.value()}};
    }});
};

auto assets(const std::string &address, const configuration &config) -> task<std::vector<std::pair<std::string, double>>> {
    return schedule(std::function{[address, opts{config.blockfrost}]() -> std::vector<std::pair<std::string, double>> {
        spdlog::info("blockfrost::v0: requesting wallet assets {}", address);
        const auto ret = blockfrost::v0::accounts_assets_balance(address, opts);
        spdlog::info("blockfrost::v0: found {} assets", ret.size());
        return ret | ranges::views::transform([](const blockfrost::v0::asset &v) {
            return std::make_pair(v.unit, v.quantity);
        }) | ranges::to<std::vector>();
    }});
};
} // namespace chain::cardano

namespace chain {
using callback = std::function<decltype(chain::cardano::balance)>;
}
