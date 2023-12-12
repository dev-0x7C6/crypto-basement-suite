#include <CLI/CLI.hpp>
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <fmt/format.h>
#include <numeric>

#include <libblockfrost/public/includes/libblockfrost/v0/balance.hpp>
#include <range/v3/view/filter.hpp>
#include <range/v3/view/join.hpp>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

#include <map>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include <csv.hpp>
#include <types.hpp>

#include <libcoingecko/v3/simple/price.hpp>

using namespace csv;

using namespace ranges;
using namespace std::chrono_literals;

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

struct share {
    std::string asset;
    double share{};
    double quantity{};
};

auto as_btc(const std::map<std::string, struct coingecko::v3::simple::price::price> &prices) -> std::optional<double> {
    if (!prices.contains("btc")) return {};
    return prices.at("btc").value;
}

auto main(int argc, char **argv) -> int {
    CLI::App app("portfolio");

    std::vector<std::string> quantity_csv_files;
    std::vector<std::string> wallet_csv_files;
    std::string preferred_currency{"usd"};

    app.add_option("-i,--input,input", quantity_csv_files, "csv format <coin, quantity>")->required()->allow_extra_args()->check(CLI::ExistingFile);
    app.add_option("-w,--wallet-addresses,wallet-addresses", wallet_csv_files, "csv format <coin, address>");
    app.add_option("-p,--preferred-currency,preferred-currency", preferred_currency, "show value in currency");
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
        .vs_currencies = {"usd", "btc", "pln", "sats", "eur", preferred_currency},
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
    std::map<std::string, double> _24h_change;
    double _24h_min{};
    double _24h_max{};

    for (auto &&[asset, ballance] : portfolio) {
        const auto &prices = summary.at(asset);
        spdlog::info("\n+ {} [{:f}]", asset, ballance);

        for (auto &&[currency, valuation] : prices) {
            const auto value = ballance * valuation.value;
            const auto _24h = valuation.change_24h;
            total[currency] += value;
            _24h_change[asset] = _24h;
            _24h_min = std::min(_24h_min, _24h);
            _24h_max = std::max(_24h_max, _24h);

            spdlog::info(" -> /{}: {:f}", currency, value);
        }
    }

    std::vector<share> shares;
    const auto total_in_btc = total["btc"];

    for (auto &&[asset, ballance] : portfolio) {
        if (!summary.contains(asset)) continue;

        const auto &prices = summary.at(asset);
        const auto value = ballance * as_btc(prices).value_or(0.0);

        shares.push_back({
            .asset = asset,
            .share = value / total_in_btc * 100.0,
            .quantity = ballance,
        });
    }

    ranges::sort(shares, [&](auto &&l, auto &&r) {
        return _24h_change.at(l.asset) > _24h_change.at(r.asset);
    });

    auto colorize = [](auto &&text, std::uint8_t r, std::uint8_t g, std::uint8_t b) {
        return fmt::format("\x1b[38;2;{};{};{}m{}\033[m", r, g, b, text);
    };

    auto colorized_percent = [&](double value, double min, double max) {
        constexpr auto _max = +2.5;
        constexpr auto _min = -2.5;
        const auto c = std::clamp(value, _min, _max);
        const auto g = 255 * (c - _min) / (_max - _min);
        const auto r = 255 - g;
        return colorize(std::format("{:+.2f}%", value), r, g, 0);
    };

    spdlog::info("\n+ 24h change (sorted):");
    for (auto &&share : shares) {
        const auto &change = _24h_change.at(share.asset);
        const auto p = colorized_percent(change, _24h_min, _24h_max);
        spdlog::info(" {:>20}: {}", share.asset, p);
    }

    ranges::sort(shares, [](auto &&l, auto &&r) {
        return l.share > r.share;
    });

    spdlog::info("\n+ shares");
    for (auto &&share : shares) {
        const auto &prices = summary.at(share.asset);
        const auto &change = _24h_change.at(share.asset);
        const auto value = prices.at(preferred_currency).value * share.quantity;
        const auto p = colorized_percent(change, _24h_min, _24h_max);
        spdlog::info(" {:>20}: {:.2f}%, {:.2f} {}, 24h: {}", share.asset, share.share, value, preferred_currency,
            p);
    }

    spdlog::info("+ total");
    for (auto &&[currency, valuation] : total)
        spdlog::info(" -> /{}: {:.2f}", currency, valuation);

    return 0;
}
