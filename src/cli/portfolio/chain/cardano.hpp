#pragma once

#include <cstddef>
#include <memory>
#include <ranges>

#include <libblockfrost/public/includes/libblockfrost/v0/balance.hpp>

#include <spdlog/async_logger.h>
#include <spdlog/logger.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>
#include <vector>

#include "common/configuration.hpp"
#include "helpers/threading.hpp"

using shared_logger = std::shared_ptr<spdlog::logger>;

namespace chain::cardano {

namespace log::request {
namespace balance {
auto requested(const shared_logger &logger, const std::string &addr) -> void {
    logger->info("blockfrost::v0: {}: requesting wallet balance", addr);
}

auto failed(const shared_logger &logger, const std::string &addr) -> void {
    logger->error("blockfrost::v0: {}: unable to request", addr);
}

auto success(const shared_logger &logger, const std::string &addr, const double balance) -> void {
    logger->info("blockfrost::v0: {}: balance {:.2f}", addr, balance);
}
} // namespace balance

namespace assets {
auto requested(const shared_logger &logger, const std::string &addr) -> void {
    logger->info("blockfrost::v0: {}: requesting wallet assets", addr);
}

auto failed(const shared_logger &logger, const std::string &addr) -> void {
    logger->error("blockfrost::v0: {}: unable to request", addr);
}

auto success(const shared_logger &logger, const std::string &addr, const std::size_t count) -> void {
    logger->info("blockfrost::v0: {}: found {} assets", addr, count);
}
} // namespace assets
} // namespace log::request

auto balance(const shared_logger &logger, const std::string &addr, const configuration &config) -> task<std::vector<std::pair<std::string, double>>> {
    return schedule(std::function{[logger, addr, opts{config.blockfrost}]() -> std::vector<std::pair<std::string, double>> {
        log::request::balance::requested(logger, addr);
        const auto balance = blockfrost::v0::accounts_balance(addr, opts);

        if (!balance) {
            log::request::balance::failed(logger, addr);
            return {{"cardano", 0.0}};
        }

        log::request::balance::success(logger, addr, balance.value());
        return {{"cardano", balance.value()}};
    }});
};

auto assets(const shared_logger &logger, const std::string &addr, const configuration &config) -> task<std::vector<std::pair<std::string, double>>> {
    namespace rng = std::ranges;

    return schedule(std::function{[logger, addr, opts{config.blockfrost}]() -> std::vector<std::pair<std::string, double>> {
        log::request::assets::requested(logger, addr);

        const auto ret = blockfrost::v0::accounts_assets_balance(addr, opts);

        if (!ret) {
            log::request::assets::failed(logger, addr);
            return {};
        }

        const auto &assets = ret.value();

        log::request::assets::success(logger, addr, assets.size());

        auto conversion = assets | rng::views::transform([](const blockfrost::v0::asset &v) {
            return std::make_pair(v.unit, v.quantity);
        }) | rng::to<std::vector>();

        for (auto &&[asset, quantity] : conversion)
            logger->info("blockfrost::v0: {}: asset {}, quantity {}", addr, asset, quantity);

        return conversion;
    }});
};
} // namespace chain::cardano

namespace chain {
using callback = std::function<decltype(chain::cardano::balance)>;
}
