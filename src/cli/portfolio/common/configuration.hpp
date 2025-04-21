#pragma once

#include <libblockfrost/public/includes/libblockfrost/v0/options.hpp>
#include <libcoingecko/v3/options.hpp>

#include <filesystem>
#include <string>
#include <vector>

struct configuration {
    blockfrost::v0::options blockfrost;
    coingecko::v3::options coingecko;

    std::vector<std::string> balances;
    std::vector<std::string> track_wallets;
    std::string preferred_currency{"usd"};

    struct {
        std::filesystem::path token_registry_path;
    } cardano;

    struct {
        bool balances{false};
        bool shares{false};
    } hide;

    bool show_gui{false};
};
