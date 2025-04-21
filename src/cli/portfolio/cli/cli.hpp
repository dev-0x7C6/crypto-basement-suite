#pragma once

#include <CLI/CLI.hpp>

#include "common/configuration.hpp"
#include <expected>

namespace cli {
auto parse(int argc, char **argv) -> std::expected<configuration, int> {
    CLI::App app("portfolio");

    configuration config;

    app.add_flag("-g,--gui", config.show_gui);
    app.add_option("-i,--input", config.balances, "csv format <coin, quantity>")->required()->allow_extra_args()->check(CLI::ExistingFile);
    app.add_option("-t,--track-wallets", config.track_wallets, "csv format <coin, address>");
    app.add_option("-p,--preferred-currency", config.preferred_currency, "show value in currency");
    app.add_option("--blockfrost-api-key", config.blockfrost.key, "https://blockfrost.io/");
    app.add_option("--coingecko-api-key", config.coingecko.key, "https://www.coingecko.com/en/api");
    app.add_flag("--hide-balances", config.hide.balances, "hide balances");
    app.add_flag("--hide-shares", config.hide.shares, "hide shares");
    app.add_option("--cardano-token-registry", config.cardano.token_registry_path, "path to cardano token registry");

    try {
        app.parse(argc, argv);
    } catch (const CLI::ParseError &e) {
        return std::unexpected(app.exit(e));
    }

    return config;
}
} // namespace cli
