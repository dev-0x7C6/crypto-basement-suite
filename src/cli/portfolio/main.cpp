#include <CLI/CLI.hpp>

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <expected>
#include <functional>
#include <iterator>
#include <map>
#include <memory>
#include <numeric>
#include <optional>
#include <ostream>
#include <range/v3/algorithm/fold_left.hpp>
#include <range/v3/numeric/accumulate.hpp>
#include <range/v3/view/slice.hpp>
#include <spdlog/logger.h>
#include <string>
#include <thread>
#include <utility>
#include <vector>

#include <csv.hpp>
#include <types.hpp>

#include <ranges>

#include <libblockfrost/public/includes/libblockfrost/v0/balance.hpp>
#include <libcoingecko/v3/coins/list.hpp>
#include <libcoingecko/v3/global/global.hpp>
#include <libcoingecko/v3/simple/price.hpp>

#include <range/v3/all.hpp>
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
        logger->info("retry, waiting 1min 30secs for coingecko");
        this_thread::sleep_for(1min + 30s);
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

/*
0 (Bitcoin launch)	3rd Jan 2009	0 (Genesis Block)	50 BTC	50%	N/A
1st halving	28th Nov 2012	210,000	25 BTC	75%	$12.35
2nd halving	9th Jul 2016	420,000	12.5 BTC	87.5%	$650.53
3rd halving	11th May 2020	630,000	6.25 BTC	93.75%	$8,571.67
4th halving	19th April 2024	840,000	3.125 BTC	96.875%	$63,842.56
*/

template <>
struct fmt::formatter<std::chrono::year_month_day> {
    template <typename ParseContext>
    auto parse(ParseContext &ctx) { return ctx.begin(); }

    template <typename FormatContext>
    auto format(const std::chrono::year_month_day &ymd, FormatContext &ctx) -> decltype(ctx.out()) {
        const auto y = static_cast<std::int32_t>(ymd.year());
        const auto m = static_cast<std::uint32_t>(ymd.month());
        const auto d = static_cast<std::uint32_t>(ymd.day());
        return fmt::format_to(ctx.out(), "{:04}-{:02}-{:02}", y, m, d);
    }
};

auto main(int argc, char **argv) -> int {
    auto logger = spdlog::stdout_color_mt("portfolio");
    logger->set_pattern("%v");

    using namespace std::chrono;

    const std::vector<year_month_day> btc_halving_date_table{
        {2009y / January / 3},
        {2012y / November / 28},
        {2016y / July / 9},
        {2020y / May / 11},
        {2024y / April / 19}};

    std::vector<int> btc_halving_day_count;

    for (auto &&compare : btc_halving_date_table | std::views::slide(2)) {
        const auto start = sys_days(compare.front());
        const auto end = sys_days(compare.back());
        btc_halving_day_count.push_back(days(end - start).count());
    }

    const auto sum = ::ranges::fold_left(btc_halving_day_count, int(0), std::plus<int>());
    const auto avg = sum / btc_halving_day_count.size();

    const auto last_halving = sys_days(btc_halving_date_table.back());
    const auto next_halving = year_month_day(last_halving + days(avg));

    logger->info("next halving approximation: {}", fmt::to_string(next_halving));

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
            // balances.emplace_back(std::make_pair(info, quantity / 1000000));
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
