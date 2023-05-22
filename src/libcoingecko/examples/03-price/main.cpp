#include <iostream>

#include <libcoingecko/v3/coins/price.hpp>

auto main(int argc, char **argv) -> int {
    const auto result = coingecko::v3::coins::price({
        .ids = {"bitcoin", "ethereum", "cardano", "polkadot", "cosmos"},
        .vs_currencies = {"usd", "btc", "sats"},
    });

    if (!result) {
        std::cerr << "coingecko: unable to fetch prices" << std::endl;
        return 1;
    }

    for (auto &&[asset, valuation] : result.value()) {
        std::cout << "asset: " << asset << std::endl;
        for (auto &&[currency, info] : valuation)
            std::cout << " -> " << currency << ": " << info.value << std::endl;
        std::cout << std::endl;
    }

    return {};
}
