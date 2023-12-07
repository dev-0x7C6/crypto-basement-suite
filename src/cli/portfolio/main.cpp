#include <CLI/CLI.hpp>
#include <fmt/format.h>
#include <libblockfrost/public/includes/libblockfrost/v0/balance.hpp>
#include <nlohmann/json.hpp>
#include <range/v3/view/filter.hpp>
#include <range/v3/view/join.hpp>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

#include <map>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include <libcoingecko/v3/all.hpp>
#include <networking/downloader.hpp>
#include <types.hpp>

#include <csv.hpp>

using namespace csv;

using namespace ranges;
using namespace std::chrono_literals;
using namespace nlohmann;

auto read_input_files(const std::vector<std::string> &files) -> std::vector<std::pair<std::string, double>> {
    std::vector<std::pair<std::string, double>> ret;
    for (auto &&file : files) {
        spdlog::info("reading data from {}", file);
        CSVReader reader(file);
        for (auto &&row : reader)
            ret.emplace_back(std::make_pair(row[0].get<std::string>(), row[1].get<double>()));
    }

    ranges::sort(ret, [](auto &&l, auto &&r) {
        return l.first < r.first;
    });

    return ret;
}

auto read_wallet_files(const std::vector<std::string> &files) -> std::vector<std::pair<std::string, std::string>> {
    std::vector<std::pair<std::string, std::string>> ret;
    for (auto &&file : files) {
        spdlog::info("reading data from {}", file);
        CSVReader reader(file);
        for (auto &&row : reader)
            ret.emplace_back(std::make_pair(row[0].get<std::string>(), row[1].get<std::string>()));
    }
    return ret;
}

auto main(int argc, char **argv) -> int {
    CLI::App app("portfolio");
    std::vector<std::string> quantity_csv_files;
    std::vector<std::string> wallet_csv_files;
    auto input_file_opt = app.add_option("-i,--input,input", quantity_csv_files, "csv format <coin, quantity>")->required();
    auto wallet_addresses_opt = app.add_option("-w,--wallet-addresses,wallet-addresses", wallet_csv_files, "csv format <coin, address>");
    input_file_opt->allow_extra_args()->check(CLI::ExistingFile);
    CLI11_PARSE(app, argc, argv);

    auto console = spdlog::stdout_color_mt("console");
    spdlog::set_pattern("%v");

    auto input = read_input_files(quantity_csv_files);
    auto wallets = read_wallet_files(wallet_csv_files);

    for (auto &&[coin, address] : wallets)
        if (coin == "cardano")
            input.emplace_back(std::make_pair(coin, blockfrost::v0::accounts_balance(address).value_or(0.0)));

    const auto req = coingecko::v3::simple::price::query({
        .ids = input | ranges::views::transform([](auto &&p) { return p.first; }) | ranges::to<std::vector<std::string>>(),
        .vs_currencies = {"usd", "btc", "pln", "sats", "eur"},
    });

    if (!req) {
        spdlog::error("invalid coingecko data");
        return 1;
    }

    auto &&summary = req.value();

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
