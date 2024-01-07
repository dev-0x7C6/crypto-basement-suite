#include <CLI/CLI.hpp>

#include <algorithm>
#include <expected>
#include <functional>
#include <iterator>
#include <map>
#include <memory>
#include <optional>
#include <spdlog/logger.h>
#include <string>
#include <thread>
#include <utility>
#include <vector>

#include <csv.hpp>
#include <types.hpp>

#include <libblockfrost/public/includes/libblockfrost/v0/balance.hpp>
#include <libcoingecko/v3/coins/list.hpp>
#include <libcoingecko/v3/global/global.hpp>
#include <libcoingecko/v3/simple/price.hpp>

#include <range/v3/view/filter.hpp>
#include <range/v3/view/join.hpp>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

#include "chain/cardano.hpp"
#include "cli/cli.hpp"
#include "helpers/formatter.hpp"
#include "helpers/threading.hpp"

#include "readers/balances.hpp"
#include "readers/wallets.hpp"

using namespace csv;
using namespace coingecko::v3;
using namespace std;
using namespace std::chrono_literals;

struct share {
    string asset;
    double share{};
    double quantity{};
};

auto as_btc(const map<string, struct simple::price::price> &prices) -> optional<double> {
    if (!prices.contains("btc")) return {};
    return prices.at("btc").value;
}

template <typename Callable, typename... Ts>
auto repeat(const shared_ptr<spdlog::logger> &logger, Callable &callable, Ts &&...values) {
    for (;;) {
        auto ret = callable(std::forward<Ts>(values)...);
        if (ret)
            return ret;
        logger->info("retry, waiting 1min for coingecko");
        this_thread::sleep_for(1min);
    }
}

using portfolio = map<string, double>;

namespace shares {
auto calculate(const portfolio &portfolio,
    function<optional<double>(const string &asset)> &&query_price,
    double total) -> vector<share> {
    vector<share> shares;

    for (auto &&[asset, balance] : portfolio) {
        const auto price = query_price(asset);

        if (!price) continue;

        const auto value = balance * price.value_or(0.0);

        shares.push_back({
            .asset = asset,
            .share = value / total * 100.0,
            .quantity = balance,
        });
    }

    return shares;
}
} // namespace shares

auto main(int argc, char **argv) -> int {
    auto logger = spdlog::stdout_color_mt("portfolio");
    logger->set_pattern("%v");

    auto expected_config = cli::parse(argc, argv);
    if (!expected_config)
        return expected_config.error();

    const auto config = std::move(expected_config.value());

    auto balances = readers::balances_from_csv(config.balances);
    auto wallets = readers::wallets_from_csv(config.track_wallets);

    vector<task<vector<pair<string, double>>>> balance_reqs;
    vector<task<vector<pair<string, double>>>> assets_reqs;
    map<string, string> contract_to_symbol;

    using namespace coingecko::v3;

    const auto assets = repeat(logger, coins::list::query, coins::list::settings{}, config.coingecko);
    for (auto &&asset : assets.value())
        for (auto &&[_, contract] : asset.platforms)
            contract_to_symbol[contract] = asset.id;

    map<string, chain::callback> wallet_balances;
    map<string, chain::callback> wallet_assets;

    wallet_balances["cardano"] = chain::cardano::balance;
    wallet_assets["cardano"] = chain::cardano::assets;

    for (auto &&[coin, address] : wallets) {
        if (wallet_balances.contains(coin))
            balance_reqs.emplace_back(wallet_balances.at(coin)(logger, address, config));

        if (wallet_assets.contains(coin))
            assets_reqs.emplace_back(wallet_assets.at(coin)(logger, address, config));
    }

    for (auto &&request : balance_reqs) {
        const auto value = request.get();
        if (!value.empty())
            std::move(value.begin(), value.end(), std::back_inserter(balances));
    }

    for (auto &&request : assets_reqs) {
        const auto assets = request.get();
        for (auto &&[coin, quantity] : assets) {
            if (!contract_to_symbol.contains(coin)) continue;
            const auto &info = contract_to_symbol[coin];
            logger->info("found coin asset {}", info);
            // input.emplace_back(std::make_pair(info, quantity));
        }
    }

    auto request_price = schedule(function{[logger, balances, &config]() {
        logger->info("coingecko::v3: requesting prices");
        return repeat(logger, simple::price::query, //
            simple::price::parameters{
                .ids = balances | ::ranges::views::transform([](auto &&p) { return p.first; }) | ::ranges::to<vector<string>>(),
                .vs_currencies = {"usd", "btc", "pln", "sats", "eur", config.preferred_currency},
            },
            config.coingecko);
    }});

    auto request_global_stats = schedule(function{[logger, &config]() {
        logger->info("coingecko::v3: requesting global market data");
        return repeat(logger, global::list, config.coingecko);
    }});

    if (!request_price.get() || !request_global_stats.get()) {
        logger->error("invalid coingecko data");
        return 1;
    }

    const auto summary = request_price.get().value();
    const auto global_market = request_global_stats.get().value();

    portfolio portfolio;
    for (auto [symbol, balance] : balances)
        portfolio[symbol] += balance;

    map<string, double> total;
    map<string, double> _24h_change;

    for (auto &&[asset, ballance] : portfolio) {
        const auto &prices = summary.at(asset);

        if (config.hide.balances)
            logger->info("\n+ {} [---]", asset);
        else
            logger->info("\n+ {} [{:f}]", asset, ballance);

        for (auto &&[currency, valuation] : prices) {
            const auto value = ballance * valuation.value;
            const auto _24h = valuation.change_24h;
            total[currency] += value;
            _24h_change[asset] = _24h;
            const auto formatted_price = format::price(value, config);

            logger->info(" -> {} {}", formatted_price, currency);
        }
    }

    auto price = [&summary](const string &asset, const string &currency) -> optional<double> {
        try {
            return summary.at(asset).at(currency).value;
        } catch (...) {}

        return {};
    };

    auto shares = shares::calculate(
        portfolio, [&](const string &asset) -> optional<double> {
            return price(asset, "btc");
        },
        total["btc"]);

    ::ranges::sort(shares, [&](auto &&l, auto &&r) {
        return _24h_change.at(l.asset) > _24h_change.at(r.asset);
    });

    logger->info("\n+ 24h change (sorted):");
    for (auto &&share : shares) {
        const auto percent = format::percent(_24h_change.at(share.asset));
        const auto value = price(share.asset, config.preferred_currency).value_or(0.0);
        logger->info(" {:>20}: {} [{:.2f} {}]",
            share.asset,
            percent,
            value,
            config.preferred_currency);
    }

    ::ranges::sort(shares, [](auto &&l, auto &&r) {
        return l.share > r.share;
    });

    logger->info("\n+ shares");
    for (auto &&s : shares) {
        const auto value = price(s.asset, config.preferred_currency).value_or(0.0) * s.quantity;
        const auto percent = format::percent(_24h_change.at(s.asset));
        const auto share = format::share(s.share, config);
        const auto price = format::price(value, config);
        logger->info(" {:>20}: {}, {} {}, 24h: {}",
            s.asset,
            share,
            price,
            config.preferred_currency,
            percent);
    }

    logger->info("\n+ global market");
    const auto total_market_cap = fmt::format("{:.3f} T", global_market.total_market_cap.at("usd") / 1000 / 1000 / 1000 / 1000);
    const auto total_market_cap_change = format::percent(global_market.market_cap_change_percentage_24h_usd, -5, 5);
    logger->info(" -> {} {}", total_market_cap, total_market_cap_change);

    logger->info("\n+ total");
    for (auto &&[currency, valuation] : total)
        logger->info(" -> {} {}", format::price(valuation, config), currency);

    return 0;
}
