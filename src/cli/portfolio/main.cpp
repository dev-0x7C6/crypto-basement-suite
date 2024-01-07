#include <CLI/CLI.hpp>

#include <algorithm>
#include <expected>
#include <functional>
#include <iterator>
#include <map>
#include <optional>
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

using namespace ranges;
using namespace std::chrono_literals;

static auto logger = []() {
    auto console = spdlog::stdout_color_mt("portfolio");
    console->set_pattern("%v");
    return spdlog::get("portfolio");
}();

struct share {
    std::string asset;
    double share{};
    double quantity{};
};

auto as_btc(const std::map<std::string, struct coingecko::v3::simple::price::price> &prices) -> std::optional<double> {
    if (!prices.contains("btc")) return {};
    return prices.at("btc").value;
}

template <typename Callable, typename... Ts>
auto repeat(Callable &callable, Ts &&...values) {
    for (;;) {
        auto ret = callable(std::forward<Ts>(values)...);
        if (ret)
            return ret;
        logger->info("retry, waiting 1min for coingecko");
        std::this_thread::sleep_for(1min);
    }
}

auto main(int argc, char **argv) -> int {
    auto expected_config = cli::parse(argc, argv);
    if (!expected_config)
        return expected_config.error();

    using namespace coingecko::v3;

    const auto config = std::move(expected_config.value());

    auto balances = readers::balances_from_csv(config.balances);
    auto wallets = readers::wallets_from_csv(config.track_wallets);

    std::vector<task<std::vector<std::pair<std::string, double>>>> balance_reqs;
    std::vector<task<std::vector<std::pair<std::string, double>>>> assets_reqs;
    std::map<std::string, std::string> contract_to_symbol;

    using namespace coingecko::v3;

    const auto assets = repeat(coins::list::query, coins::list::settings{}, config.coingecko);
    for (auto &&asset : assets.value())
        for (auto &&[_, contract] : asset.platforms)
            contract_to_symbol[contract] = asset.id;

    std::map<std::string, chain::callback> wallet_balances;
    std::map<std::string, chain::callback> wallet_assets;

    wallet_balances["cardano"] = chain::cardano::balance;
    wallet_assets["cardano"] = chain::cardano::assets;

    for (auto &&[coin, address] : wallets) {
        if (wallet_balances.contains(coin))
            balance_reqs.emplace_back(wallet_balances.at(coin)(address, config));

        if (wallet_assets.contains(coin))
            assets_reqs.emplace_back(wallet_assets.at(coin)(address, config));
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

    auto request_price = schedule(std::function{[balances, &config]() {
        logger->info("coingecko::v3: requesting prices");
        return repeat(simple::price::query, //
            simple::price::parameters{
                .ids = balances | ::ranges::views::transform([](auto &&p) { return p.first; }) | ::ranges::to<std::vector<std::string>>(),
                .vs_currencies = {"usd", "btc", "pln", "sats", "eur", config.preferred_currency},
            },
            config.coingecko);
    }});

    auto request_global_stats = schedule(std::function{[&config]() {
        logger->info("coingecko::v3: requesting global market data");
        return repeat(global::list, config.coingecko);
    }});

    if (!request_price.get() || !request_global_stats.get()) {
        logger->error("invalid coingecko data");
        return 1;
    }

    const auto summary = request_price.get().value();
    const auto global_market = request_global_stats.get().value();

    std::map<std::string, double> portfolio;
    for (auto [symbol, balance] : balances)
        portfolio[symbol] += balance;

    std::map<std::string, double> total;
    std::map<std::string, double> _24h_change;
    double _24h_min{};
    double _24h_max{};

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
            _24h_min = std::min(_24h_min, _24h);
            _24h_max = std::max(_24h_max, _24h);
            const auto formatted_price = format::price(value, config);

            logger->info(" -> {} {}", formatted_price, currency);
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

    ::ranges::sort(shares, [&](auto &&l, auto &&r) {
        return _24h_change.at(l.asset) > _24h_change.at(r.asset);
    });

    logger->info("\n+ 24h change (sorted):");
    for (auto &&share : shares) {
        const auto &change = _24h_change.at(share.asset);
        const auto p = format::percent(change, _24h_min, _24h_max);
        const auto x = summary.at(share.asset).at(config.preferred_currency).value;
        logger->info(" {:>20}: {} [{:.2f} {}]", share.asset, p, x, config.preferred_currency);
    }

    ::ranges::sort(shares, [](auto &&l, auto &&r) {
        return l.share > r.share;
    });

    logger->info("\n+ shares");
    for (auto &&share : shares) {
        const auto &prices = summary.at(share.asset);
        const auto &change = _24h_change.at(share.asset);
        const auto value = prices.at(config.preferred_currency).value * share.quantity;
        const auto p = format::percent(change, _24h_min, _24h_max);
        const auto formatted_share = format::share(share.share, config);
        const auto formatted_price = format::price(value, config);
        logger->info(" {:>20}: {}, {} {}, 24h: {}", share.asset, formatted_share, formatted_price, config.preferred_currency,
            p);
    }

    logger->info("\n+ global market");
    const auto total_market_cap = fmt::format("{:.3f} T", global_market.total_market_cap.at("usd") / 1000 / 1000 / 1000 / 1000);
    const auto total_market_cap_change = format::percent(global_market.market_cap_change_percentage_24h_usd, -5, 5);
    logger->info(" -> {} {}", total_market_cap, total_market_cap_change);

    logger->info("\n+ total");
    for (auto &&[currency, valuation] : total)
        logger->info(" -> {} {}", format::price(valuation, config), currency);

    spdlog::shutdown();

    return 0;
}
