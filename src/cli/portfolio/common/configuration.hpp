#pragma once

#include <libblockfrost/public/includes/libblockfrost/v0/options.hpp>
#include <libcoingecko/v3/options.hpp>

#include <string>
#include <vector>

struct configuration {
    blockfrost::v0::options blockfrost;
    coingecko::v3::options coingecko;

    std::vector<std::string> ballances;
    std::vector<std::string> track_wallets;
    std::string preferred_currency{"usd"};

    struct {
        bool balances{false};
        bool shares{false};
    } hide;
};
