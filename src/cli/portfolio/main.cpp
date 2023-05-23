#include <CLI/CLI.hpp>
#include <fmt/format.h>
#include <nlohmann/json.hpp>
#include <range/v3/view/join.hpp>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

#include <chrono>
#include <optional>
#include <sstream>
#include <unordered_map>
#include <vector>

#include <libcoingecko/v3/all.hpp>
#include <networking/downloader.hpp>
#include <types.hpp>

using namespace ranges;
using namespace std::chrono_literals;
using namespace nlohmann;

auto main(int argc, char **argv) -> int {
    CLI::App app("portfolio");
    std::vector<std::string> inputs;
    auto input_file_opt = app.add_option("-i,--input,input", inputs, "input json");
    input_file_opt->allow_extra_args()->check(CLI::ExistingFile);
    CLI11_PARSE(app, argc, argv);

    auto console = spdlog::stdout_color_mt("console");
    spdlog::set_pattern("%v");

    const auto req = coingecko::v3::coins::price::query({
        .ids = {"bitcoin", "cardano", "polkadot", "cosmos", "avalanche-2", "near", "algorand", "solana"},
        .vs_currencies = {"usd", "btc", "pln", "sats"},
    });

    if (!req) {
        spdlog::error("invalid coingecko data");
        return 1;
    }

    auto &&summary = req.value();

    // test input
    std::vector<std::pair<std::string, double>> input{
        {"bitcoin", 0.12},
        {"bitcoin", 0.24},
        {"bitcoin", 0.02},
        {"cardano", 1203.50},
        {"cardano", 1929.29},
        {"cardano", 2384.03},
        {"cosmos", 15.22},
        {"cosmos", 20.21},
        {"cosmos", 41.98},
        {"polkadot", 129.23},
        {"polkadot", 21.02},
        {"polkadot", 99.99},
    };

    std::map<std::string, double> portfolio;
    for (auto [symbol, balance] : input)
        portfolio[symbol] += balance;

    std::map<std::string, double> total;

    for (auto &&[portfolio_asset, portfolio_ballance] : portfolio) {
        spdlog::info("+ {} [{:f}]", portfolio_asset, portfolio_ballance);
        for (auto &&[asset, prices] : summary)
            if (portfolio_asset == asset)
                for (auto &&[currency, valuation] : prices) {
                    const auto value = portfolio_ballance * valuation.value;
                    spdlog::info(" -> /{}: {:f}", currency, value);
                    total[currency] += value;
                }
        spdlog::info("");
    }

    spdlog::info("+ shares");
    for (auto &&[portfolio_asset, portfolio_ballance] : portfolio) {
        spdlog::info("  + {}", portfolio_asset);
        for (auto &&[asset, prices] : summary)
            if (portfolio_asset == asset) {
                const auto value = portfolio_ballance * prices.at("btc").value;
                spdlog::info("   -> {:.2f}% [{:f}]", value / total["btc"] * 100.0, portfolio_ballance);
            }

        spdlog::info("");
    }

    spdlog::info("+ total");
    for (auto &&[currency, valuation] : total)
        spdlog::info(" -> /{}: {:.2f}", currency, valuation);

    return 0;
}
