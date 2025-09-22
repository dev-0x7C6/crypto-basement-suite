#include <CLI/CLI.hpp>
#include <csv.hpp>
#include <memory>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>
#include <types.hpp>

#include <libblockfrost/public/includes/libblockfrost/v0/balance.hpp>
#include <libcoingecko/v3/coins/list.hpp>
#include <libcoingecko/v3/global/global.hpp>
#include <libcoingecko/v3/simple/price.hpp>
#include <rest/requests.hpp>

#include "chain/algorand.hpp"
#include "chain/bitcoin.hpp"
#include "chain/cardano.hpp"
#include "chain/hedera.hpp"
#include "cli/cli.hpp"
#include "common/configuration.hpp"
#include "common/share.hpp"
#include "common/short_scales.hpp"
#include "extensions/btc-halving.hpp"
#include "extensions/cardano-registry-scanner.hpp"
#include "gui/gui.hpp"
#include "helpers/formatter.hpp"
#include "helpers/threading.hpp"
#include "libcoingecko/v3/coins/markets.hpp"
#include "printers/print-day-change.hpp"
#include "printers/print-shares.hpp"
#include "printers/print-summary.hpp"
#include "printers/print-supply.hpp"
#include "readers/balances.hpp"
#include "readers/wallets.hpp"
#include "storage/storage.hpp"

using namespace csv;
using namespace coingecko::v3;
using namespace std;
using namespace std::chrono_literals;
using namespace nlohmann;

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

namespace cardano::token {
struct details {
    double divisor{1.0};
};

} // namespace cardano::token

auto quote(auto &&value) -> std::string {
    return std::format("'{}'", std::forward<decltype(value)>(value));
}

auto average(std::ranges::range auto rng) {
    const auto count = std::ranges::distance(rng);
    const auto value = std::accumulate(std::begin(rng), std::end(rng), 0);
    return value / count;
}

auto main(int argc, char **argv) -> int {
    auto logger = spdlog::stdout_color_mt("portfolio");
    logger->set_pattern("%v");

    auto expected_config = cli::parse(argc, argv);
    if (!expected_config)
        return expected_config.error();

    const auto config = std::move(expected_config.value());

    const auto cardano_token_registry = cardano::registry::scan(config.cardano.token_registry_path);

    auto balances = readers::balances_from_csv(config.balances);
    auto wallets = readers::wallets_from_csv(config.track_wallets);

    vector<task<vector<pair<string, double>>>> balance_reqs;
    vector<task<vector<pair<string, double>>>> assets_reqs;
    map<string, string> contract_to_symbol;

    using namespace coingecko::v3;

    const auto assets = repeat(logger, coins::list::query, coins::list::settings{}, config.coingecko);

    const map<string, chain::callback> wallet_balances{
        {"cardano", chain::cardano::balance},
        {"bitcoin", chain::bitcoin::balance},
        {"hedera-hashgraph", chain::hedera::balance},
        {"algorand", chain::algorand::balance}};

    const map<string, chain::callback> wallet_assets{
        {"cardano", chain::cardano::assets},
    };

    for (auto &&[blockchain, address] : wallets) {
        if (wallet_balances.contains(blockchain))
            balance_reqs.emplace_back(wallet_balances.at(blockchain)(logger, address, config));

        if (wallet_assets.contains(blockchain))
            assets_reqs.emplace_back(wallet_assets.at(blockchain)(logger, address, config));
    }

    for (auto &&request : balance_reqs) {
        const auto value = request.get();
        if (!value.empty())
            move(value.begin(), value.end(), back_inserter(balances));
    }

    for (auto &&request : assets_reqs) {
        const auto assets = request.get();
        for (auto &&[contract, quantity] : assets) {
            if (!contract_to_symbol.contains(contract)) continue;
            if (!cardano_token_registry.contains(contract)) continue;

            const auto &info = contract_to_symbol[contract];
            const auto div = cardano_token_registry.at(contract).divisor;

            logger->info("found coin asset {}", quote(info));
            logger->info("  contract: {}", quote(contract));
            logger->info("  quantity: {}", quote(quantity));
            logger->info("   divisor: {}", quote(div));

            balances.emplace_back(make_pair(info, quantity / div));
        }
    }

    namespace rng = std::ranges;
    namespace view = std::ranges::views;

    auto request_price = schedule(function{[logger, balances, &config]() {
        logger->info("coingecko::v3: requesting prices");
        return repeat(logger, simple::price::query, //
            simple::price::parameters{
                .ids = balances | view::keys | rng::to<set<string>>(),
                .vs_currencies = {symbol::usd, symbol::btc, symbol::eur, symbol::pln, config.preferred_currency},
            },
            config.coingecko);
    }});

    auto request_markets_data = schedule(function{[logger, balances, &config]() {
        logger->info("coingecko::v3: requesting market data");
        return repeat(logger, coins::markets, //
            coins::markets_query{
                .vs_currency = symbol::btc,
                .ids = balances | view::keys | rng::to<set<string>>(),
            },
            config.coingecko);
    }});

    auto request_global_stats = schedule(function{[logger, &config]() {
        logger->info("coingecko::v3: requesting global market data");
        return repeat(logger, global::list, config.coingecko);
    }});

    if (!request_price.get() || !request_global_stats.get() || !request_markets_data.get()) {
        logger->error("invalid coingecko data");
        return 1;
    }

    const auto summary = request_price.get().value();
    const auto global_market = request_global_stats.get().value();
    const auto coin_list_with_market_data = request_markets_data.get().value();

    portfolio portfolio;
    for (auto [symbol, balance] : balances)
        portfolio[symbol] += balance;

    map<string, double> total;
    map<string, double> _24h_change;

    for (auto &&[asset, ballance] : portfolio) {
        if (!summary.contains(asset)) {
            logger->warn("asset {} not mapped", quote(asset));
            continue;
        }

        const auto &prices = summary.at(asset);
        logger->info("\n+ {} [{}]", asset, format::price(ballance, config));

        for (auto &&[currency, valuation] : prices) {
            const auto value = ballance * valuation.value;
            const auto _24h = valuation.change_24h;
            total[currency] += value;
            _24h_change[asset] = _24h;
            const auto formatted_price = format::price(value, config);
            logger->info(" -> {} {}", formatted_price, currency);
        }
    }

    storage::save(portfolio, summary);

    auto price = [&summary](const string &asset, const string &currency) -> optional<currency_quantity> {
        try {
            return currency_quantity{currency, summary.at(asset).at(currency).value};
        } catch (...) {}

        return {};
    };

    auto price_in_btc = [&price](const string &asset) {
        return price(asset, symbol::btc);
    };

    auto shares = shares::calculate(portfolio, price_in_btc, total[symbol::btc]).value();

    auto get_24h_change = [&](const string &asset) {
        return _24h_change.at(asset);
    };

    auto calculate_portfolio_change = [](const map<string, double> &change_provider, const shares::shares_vec &shares, set<string> filter = {}) {
        auto change{0.0};
        auto count{0};

        for (auto &&share : shares) {
            if (filter.contains(share.asset))
                continue;

            change += share.share * change_provider.at(share.asset);
            count++;
        }

        return change / count;
    };

    printers::day_change(shares, price_in_btc, get_24h_change, config, logger);
    printers::shares(shares, price_in_btc, get_24h_change, config, logger);
    printers::finite_supply(coin_list_with_market_data, price_in_btc, config, logger);
    printers::infinite_supply(coin_list_with_market_data, price_in_btc, config, logger);

    const auto bitcoin_24h_change = _24h_change["bitcoin"];
    const auto next_halving_aprox = bitcoin::halving::trivial_next_halving_approximation().front();
    const auto portfolio_24h_change = calculate_portfolio_change(_24h_change, shares);
    const auto portfolio_24h_change_altcoins = calculate_portfolio_change(_24h_change, shares, {"bitcoin"});

    const auto fmt_next_halving = fmt::to_string(next_halving_aprox);
    const auto fmt_portfolio_24h_altcoin_gains = format::percent(portfolio_24h_change_altcoins - bitcoin_24h_change, -15, 15);
    const auto fmt_portfolio_24h_bitcoin_gains = format::percent(bitcoin_24h_change - portfolio_24h_change_altcoins, -15, 15);
    const auto fmt_portfolio_24h_change = format::percent(portfolio_24h_change, -5, 5);
    const auto fmt_portfolio_24h_gain_vs_total_tmk = format::percent(portfolio_24h_change - global_market.market_cap_change_percentage_24h_usd, -5, 5);
    const auto fmt_total_market_cap = fmt::format("{:.3f} T", global_market.total_market_cap.at(symbol::usd) / short_scales::trillion);
    const auto fmt_total_market_cap_24h_change = format::percent(global_market.market_cap_change_percentage_24h_usd, -5, 5);

    logger->info("\n+ additional:");
    logger->info(" -> next halving approx.: {}", fmt_next_halving);

    logger->info("\n+ 24h gains");
    logger->info(" -> altcoins vs bitcoin {}", fmt_portfolio_24h_altcoin_gains);
    logger->info(" -> bitcoin vs altcoins {}", fmt_portfolio_24h_bitcoin_gains);

    logger->info("\n+ 24h change");
    logger->info(" -> portfolio change {}", fmt_portfolio_24h_change);

    logger->info("\n+ global market");
    logger->info(" -> {} {}", fmt_total_market_cap, fmt_total_market_cap_24h_change);

    logger->info("\n+ portfolio vs global market");
    logger->info(" -> {}", fmt_portfolio_24h_gain_vs_total_tmk);

    printers::summary(total, config, logger);

    if (config.show_gui)
        return show_gui_charts(argc, argv, config, shares, summary);

    return 0;
}
