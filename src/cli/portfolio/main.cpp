#include "libblockfrost/v0/options.hpp"
#include "libcoingecko/v3/options.hpp"
#include <CLI/CLI.hpp>

#include <algorithm>
#include <cstdint>
#include <map>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include <csv.hpp>
#include <types.hpp>

#include <CLI/CLI.hpp>

#include <libcoingecko/v3/coins/list.hpp>
#include <libcoingecko/v3/global/global.hpp>
#include <libcoingecko/v3/simple/price.hpp>

#include <libblockfrost/public/includes/libblockfrost/v0/balance.hpp>
#include <range/v3/view/filter.hpp>
#include <range/v3/view/join.hpp>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

#include "colors.hpp"
#include "threading.hpp"

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

struct configuration {
    blockfrost::v0::options blockfrost;
    coingecko::v3::options coingecko;

    struct {
        bool balances{false};
        bool shares{false};
    } hide;
};

namespace format {

auto share(double value, const configuration &cfg) noexcept -> std::string {
    if (cfg.hide.balances)
        return "---%";

    return fmt::format("{:.2f}%", value);
}

auto price(double value, const configuration &cfg) noexcept -> std::string {
    if (cfg.hide.balances)
        return "---";

    return fmt::format("{:.2f}", value);
}

} // namespace format

auto main(int argc, char **argv) -> int {
    CLI::App app("portfolio");

    std::vector<std::string> ballances;
    std::vector<std::string> track_wallets;
    std::string preferred_currency{"usd"};

    configuration config;

    app.add_option("-i,--input", ballances, "csv format <coin, quantity>")->required()->allow_extra_args()->check(CLI::ExistingFile);
    app.add_option("-t,--track-wallets", track_wallets, "csv format <coin, address>");
    app.add_option("-p,--preferred-currency", preferred_currency, "show value in currency");
    app.add_option("--blockfrost-api-key", config.blockfrost.key, "https://blockfrost.io/");
    app.add_option("--coingecko-api-key", config.coingecko.key, "https://www.coingecko.com/en/api");
    app.add_flag("--hide-balances", config.hide.balances, "hide balances");
    app.add_flag("--hide-shares", config.hide.shares, "hide shares");
    CLI11_PARSE(app, argc, argv);

    auto console = spdlog::stdout_color_mt("console");
    spdlog::set_pattern("%v");

    auto input = read_input_files(ballances);
    auto wallets = read_wallet_files(track_wallets);

    std::vector<task<std::optional<std::pair<std::string, double>>>> request_wallet_ballances;
    std::vector<task<std::vector<blockfrost::v0::asset>>> request_wallet_assets;
    std::map<std::string, std::string> contract_to_symbol;

    const auto assets = coingecko::v3::coins::list::query({}, config.coingecko);
    for (auto &&asset : assets.value())
        for (auto &&[_, contract] : asset.platforms)
            contract_to_symbol[contract] = asset.id;

    for (auto &&[coin, address] : wallets) {
        request_wallet_ballances.emplace_back(schedule(std::function{[coin, address, &config]() -> std::optional<std::pair<std::string, double>> {
            if (coin != "cardano") return {};
            spdlog::info("blockfrost::v0: requesting wallet balance {}", address);
            const auto balance = blockfrost::v0::accounts_balance(address, config.blockfrost);

            if (!balance) {
                spdlog::error("blockfrost::v0: unable to request {}", address);
                return std::make_pair(coin, 0.0);
            }

            spdlog::info("blockfrost::v0: {}, balance {:.2f}", address, balance.value());
            return std::make_pair(coin, balance.value());
        }}));

        request_wallet_assets.emplace_back(schedule(std::function{[address, coin, &config]() -> std::vector<blockfrost::v0::asset> {
            if (coin != "cardano") return {};
            spdlog::info("blockfrost::v0: requesting wallet assets {}", address);
            auto ret = blockfrost::v0::accounts_assets_balance(address, config.blockfrost);
            spdlog::info("blockfrost::v0: found {} assets", ret.size());
            return ret;
        }}));
    }

    for (auto &&request : request_wallet_ballances) {
        const auto value = request.get();
        if (value)
            input.emplace_back(std::move(value.value()));
    }

    for (auto &&request : request_wallet_assets) {
        const auto assets = request.get();
        for (auto &&asset : assets) {
            if (!contract_to_symbol.contains(asset.unit)) continue;
            const auto &info = contract_to_symbol[asset.unit];
            spdlog::info("found coin asset {}", info);
            // input.emplace_back(std::make_pair(info, asset.quantity));
        }
    }

    auto request_price = schedule(std::function{[input, preferred_currency]() {
        spdlog::info("coingecko::v3: requesting prices");
        return coingecko::v3::simple::price::query({
            .ids = input | ranges::views::transform([](auto &&p) { return p.first; }) | ranges::to<std::vector<std::string>>(),
            .vs_currencies = {"usd", "btc", "pln", "sats", "eur", preferred_currency},
        });
    }});

    auto request_global_stats = schedule(std::function{[input, preferred_currency]() {
        spdlog::info("coingecko::v3: requesting global market data");
        return coingecko::v3::global::list();
    }});

    if (!request_price.get() || !request_global_stats.get()) {
        spdlog::error("invalid coingecko data");
        return 1;
    }

    const auto summary = request_price.get().value();
    const auto global_market = request_global_stats.get().value();

    std::map<std::string, double> portfolio;
    for (auto [symbol, balance] : input)
        portfolio[symbol] += balance;

    std::map<std::string, double> total;
    std::map<std::string, double> _24h_change;
    double _24h_min{};
    double _24h_max{};

    for (auto &&[asset, ballance] : portfolio) {
        const auto &prices = summary.at(asset);

        if (config.hide.balances)
            spdlog::info("\n+ {} [---]", asset);
        else
            spdlog::info("\n+ {} [{:f}]", asset, ballance);

        for (auto &&[currency, valuation] : prices) {
            const auto value = ballance * valuation.value;
            const auto _24h = valuation.change_24h;
            total[currency] += value;
            _24h_change[asset] = _24h;
            _24h_min = std::min(_24h_min, _24h);
            _24h_max = std::max(_24h_max, _24h);
            const auto formatted_price = format::price(value, config);

            spdlog::info(" -> {} {}", formatted_price, currency);
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

    auto colorized_percent = [&](double value, double min, double max) {
        constexpr auto _max = +10.0;
        constexpr auto _min = -10.0;
        const auto c = std::clamp(value, _min, _max);
        auto x = hsl_to_rgb({120.0 * (c - _min) / (_max - _min), 1.0, 1.0});
        return ansi_colorize(std::format("{:+.2f}%", value), x.r, x.g, x.b);
    };

    spdlog::info("\n+ 24h change (sorted):");
    for (auto &&share : shares) {
        const auto &change = _24h_change.at(share.asset);
        const auto p = colorized_percent(change, _24h_min, _24h_max);
        const auto x = summary.at(share.asset).at(preferred_currency).value;
        spdlog::info(" {:>20}: {} [{:.2f} {}]", share.asset, p, x, preferred_currency);
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
        const auto formatted_share = format::share(share.share, config);
        const auto formatted_price = format::price(value, config);
        spdlog::info(" {:>20}: {}, {} {}, 24h: {}", share.asset, formatted_share, formatted_price, preferred_currency,
            p);
    }

    spdlog::info("\n+ global market");
    const auto total_market_cap = fmt::format("{:.3f} T", global_market.total_market_cap.at("usd") / 1000 / 1000 / 1000 / 1000);
    const auto total_market_cap_change = colorized_percent(global_market.market_cap_change_percentage_24h_usd, -5, 5);
    spdlog::info(" -> {} {}", total_market_cap, total_market_cap_change);

    spdlog::info("\n+ total");
    for (auto &&[currency, valuation] : total)
        spdlog::info(" -> {} {}", format::price(valuation, config), currency);

    return 0;
}
