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

auto main(int argc, char **argv) -> int {
    CLI::App app("portfolio");
    std::vector<std::string> files;
    auto input_file_opt = app.add_option("-i,--input,input", files, "input json")->required();
    input_file_opt->allow_extra_args()->check(CLI::ExistingFile);
    CLI11_PARSE(app, argc, argv);

    auto console = spdlog::stdout_color_mt("console");
    spdlog::set_pattern("%v");

    const auto input = read_input_files(files);

    std::ofstream output("output.csv");
    auto writer = csv::make_csv_writer(output);
    writer << std::vector<std::string>({"coin", "value"});
    for (auto &&[coin, count] : input) {
        writer << std::make_tuple(coin, count);
    }

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
