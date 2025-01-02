#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <expected>
#include <filesystem>
#include <format>
#include <functional>
#include <iterator>
#include <map>
#include <memory>
#include <optional>
#include <ostream>
#include <range/v3/view/transform.hpp>
#include <ranges>
#include <string>
#include <system_error>
#include <thread>
#include <utility>
#include <vector>

#include <CLI/CLI.hpp>
#include <nlohmann/json.hpp>
#include <range/v3/all.hpp>
#include <spdlog/spdlog.h>

#include <csv.hpp>
#include <types.hpp>

#include <libblockfrost/public/includes/libblockfrost/v0/balance.hpp>
#include <libcoingecko/v3/coins/list.hpp>
#include <libcoingecko/v3/global/global.hpp>
#include <libcoingecko/v3/simple/price.hpp>

#include "chain/cardano.hpp"
#include "cli/cli.hpp"
#include "common/share.hpp"
#include "common/short_scales.hpp"
#include "extensions/btc-halving.hpp"
#include "extensions/cardano-registry-scanner.hpp"
#include "helpers/formatter.hpp"
#include "helpers/threading.hpp"
#include "readers/balances.hpp"
#include "readers/wallets.hpp"

#include <QApplication>
#include <QGroupBox>
#include <QtCharts/QBarSeries>
#include <QtCharts/QBarSet>
#include <QtCharts/QPieLegendMarker>
#include <QtCharts/QPieSlice>
#include <QtCharts/QtCharts>

#include "gui/share-chart.hpp"

using namespace csv;
using namespace coingecko::v3;
using namespace std;
using namespace std::chrono_literals;
using namespace nlohmann;

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

template <typename K, typename V>
auto value_or(const std::map<K, V> &in, const K &key, V &&v) {
    try {
        return in.at(key);
    } catch (...) {}
    return std::forward<V>(v);
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

    std::map<std::string, std::string> coingecko_contract_corrections{
        {"29d222ce763455e3d7a09a665ce554f00ac89d2e99a1a83d267170c6", "29d222ce763455e3d7a09a665ce554f00ac89d2e99a1a83d267170c64d494e"},
        {"1d7f33bd23d85e1a25d87d86fac4f199c3197a2f7afeb662a0f34e1e", "1d7f33bd23d85e1a25d87d86fac4f199c3197a2f7afeb662a0f34e1e776f726c646d6f62696c65746f6b656e"},
        {"e5a42a1a1d3d1da71b0449663c32798725888d2eb0843c4dabeca05a", "e5a42a1a1d3d1da71b0449663c32798725888d2eb0843c4dabeca05a576f726c644d6f62696c65546f6b656e58"},
        {"a0028f350aaabe0545fdcb56b039bfb08e4bb4d8c4d7c3c7d481c235", "a0028f350aaabe0545fdcb56b039bfb08e4bb4d8c4d7c3c7d481c235484f534b59"},
    };

    for (auto &&asset : assets.value())
        for (auto &&[_, contract] : asset.platforms) {
            contract_to_symbol[contract] = asset.id;
            if (coingecko_contract_corrections.contains(contract))
                contract_to_symbol[coingecko_contract_corrections.at(contract)] = asset.id;
        }

    map<string, chain::callback> wallet_balances;
    map<string, chain::callback> wallet_assets;

    wallet_balances["cardano"] = chain::cardano::balance;
    wallet_assets["cardano"] = chain::cardano::assets;

    for (auto &&[blockchain, address] : wallets) {
        if (wallet_balances.contains(blockchain))
            balance_reqs.emplace_back(wallet_balances.at(blockchain)(logger, address, config));

        if (wallet_assets.contains(blockchain))
            assets_reqs.emplace_back(wallet_assets.at(blockchain)(logger, address, config));
    }

    for (auto &&request : balance_reqs) {
        const auto value = request.get();
        if (!value.empty())
            std::move(value.begin(), value.end(), std::back_inserter(balances));
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

            balances.emplace_back(std::make_pair(info, quantity / div));
        }
    }

    auto request_price = schedule(function{[logger, balances, &config]() {
        logger->info("coingecko::v3: requesting prices");
        return repeat(logger, simple::price::query, //
            simple::price::parameters{
                .ids = balances | ::ranges::views::keys | ::ranges::to<std::set<string>>(),
                .vs_currencies = {"usd", "btc", "pln", "eur", config.preferred_currency},
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

    json portfolio_json_array = json::array();

    for (auto &&[asset, ballance] : portfolio) {
        if (!summary.contains(asset)) {
            logger->warn("asset {} not mapped", quote(asset));
            continue;
        }

        const auto &prices = summary.at(asset);
        logger->info("\n+ {} [{}]", asset, format::price(ballance, config));

        json json_valuation_array = json::array();
        for (auto &&[currency, valuation] : prices) {
            const auto value = ballance * valuation.value;
            const auto _24h = valuation.change_24h;
            total[currency] += value;
            _24h_change[asset] = _24h;
            const auto formatted_price = format::price(value, config);
            logger->info(" -> {} {}", formatted_price, currency);
            json_valuation_array.emplace_back(json{
                {"currency", currency},
                {"valuation", value},
            });
        }

        portfolio_json_array.emplace_back(json{
            {"asset", asset},
            {"balance", ballance},
            {"valuation", std::move(json_valuation_array)},
        });
    }

    const auto timestamp_epoch = std::chrono::system_clock::now().time_since_epoch();
    const auto timestamp_ms = std::chrono::duration_cast<std::chrono::milliseconds>(timestamp_epoch).count();

    json total_valuation_array = json::array();
    for (auto &&[currency, valuation] : total)
        total_valuation_array.emplace_back(json{
            {"currency", currency},
            {"valuation", valuation},
        });

    const auto portfolio_json_dump = json{
        {"timestamp", timestamp_ms},
        {"portfolio", portfolio_json_array},
        {"total", total_valuation_array},
    };

    const auto home_dir_path = getenv("HOME");

    std::error_code ec{};
    std::filesystem::create_directories(std::format("{}/.cache/crypto", home_dir_path), ec);
    std::ofstream file(std::format("{}/.cache/crypto/dump_{}.json", home_dir_path, timestamp_ms));
    file << portfolio_json_dump.dump(4);

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

    const auto preferred_currency_symbol = format::to_symbol(config.preferred_currency);

    logger->info("\n+ 24h change (sorted):");
    for (auto &&share : shares) {
        const auto percent = format::percent(_24h_change.at(share.asset));
        const auto value = price(share.asset, config.preferred_currency).value_or(0.0);
        const auto fvalue = format::price(value, {});
        logger->info(" {:>30}: {} [{}{}]",
            share.asset,
            percent,
            fvalue,
            preferred_currency_symbol);
    }

    ::ranges::sort(shares, std::greater<shares::share>());

    logger->info("\n+ shares");
    for (auto &&s : shares) {
        const auto value = price(s.asset, config.preferred_currency).value_or(0.0) * s.quantity;
        const auto percent = format::percent(_24h_change.at(s.asset));
        const auto share = format::share(s.share, config);
        const auto price = format::price(value, config);

        logger->info(" {:>20}: {}, {}{}, 24h: {}",
            s.asset,
            share,
            price,
            preferred_currency_symbol,
            percent);
    }

    const auto next_halving_aprox = bitcoin::halving::trivial_next_halving_approximation().front();
    logger->info("\n+ additional:");
    logger->info(" -> next halving approx.: {}", fmt::to_string(next_halving_aprox));

    logger->info("\n+ global market");
    const auto total_market_cap = fmt::format("{:.3f} T", global_market.total_market_cap.at("usd") / short_scales::trillion);
    const auto total_market_cap_change = format::percent(global_market.market_cap_change_percentage_24h_usd, -5, 5);
    logger->info(" -> {} {}", total_market_cap, total_market_cap_change);

    logger->info("\n+ total");

    std::set<std::string> hide_ranks{"btc"};
    std::map<std::string, int> preferred_decimal_count{{"btc", 8}};

    std::vector<std::pair<std::string, double>> total_sorted;
    auto total_vec = total | ::ranges::to<std::vector<std::pair<std::string, double>>>();

    const static std::map<std::string, int> sort_rank{
        {"btc", 2},
        {config.preferred_currency, 1},
    };

    ::ranges::sort(total_vec, [&](auto &&lhs, auto &&rhs) {
        return value_or(sort_rank, lhs.first, 0) > value_or(sort_rank, rhs.first, 0);
    });

    for (auto &&[currency, valuation] : total_vec) {
        const auto demical = value_or(preferred_decimal_count, currency, 2);
        const auto hide_rank = hide_ranks.contains(currency);
        const auto price = format::formatted_price(valuation, config, hide_rank, demical);
        const auto symbol = format::to_symbol(currency);
        logger->info(" -> {} {}", price, symbol);
    }

    QApplication app(argc, argv);
    auto tabs = new QTabWidget();

    auto format_percent_shares = [](const shares::share &share) -> std::string {
        return std::format("{} {:.2f}%", share.asset, share.share);
    };

    auto format_prices_shares = [&](const shares::share &share) -> std::string {
        const auto value = price(share.asset, config.preferred_currency).value_or(0.0) * share.quantity;
        return std::format("{} {:.0f}{}", share.asset, value, preferred_currency_symbol);
    };

    auto generate_24h_change_chart = [&]() {
        ::ranges::sort(shares, [&](auto &&l, auto &&r) {
            return _24h_change.at(l.asset) > _24h_change.at(r.asset);
        });

        auto series = new QBarSeries();

        auto _24h_sorted = _24h_change | std::ranges::to<std::vector<std::pair<std::string, double>>>();

        ::ranges::sort(_24h_sorted, [&](auto &&l, auto &&r) { return l.second > r.second; });

        for (auto &&[asset, change] : _24h_sorted) {
            auto values = new QBarSet(QString::fromStdString(std::format("{} {:.2f}%", asset, change)));
            *values << change;
            series->append(values);
        }

        auto chart = new QChart();
        chart->setAnimationOptions(QChart::AllAnimations);
        chart->setTheme(QChart::ChartThemeBlueCerulean);
        chart->legend()->setAlignment(Qt::AlignLeft);
        chart->setTitle("24h change");

        auto view = new QChartView(chart);
        view->chart()->addSeries(series);
        view->setRenderHint(QPainter::Antialiasing);
        return view;
    };

    auto generate_24h_value_chart = [&]() {
        ::ranges::sort(shares, [&](auto &&l, auto &&r) {
            return _24h_change.at(l.asset) > _24h_change.at(r.asset);
        });

        auto series = new QBarSeries();

        auto _24h_sorted = _24h_change | std::ranges::to<std::vector<std::pair<std::string, double>>>();
        auto map_shares = shares |
            ::ranges::views::transform([](const shares::share &s) {
                return std::make_pair(s.asset, s);
            }) |
            ::ranges::to<std::map<std::string, shares::share>>();

        ::ranges::sort(_24h_sorted, [&](auto &&l, auto &&r) { return (l.second * map_shares.at(l.first).share) > (r.second * map_shares.at(r.first).share); });

        for (auto &&[asset, change] : _24h_sorted) {
            const auto s = map_shares.at(asset);
            const auto p = price(asset, config.preferred_currency).value_or(0.0) * s.quantity;
            const auto price_change = p - (p / (1.00 + (change / 100.0)));

            auto values = new QBarSet(QString::fromStdString(std::format("{}, {:.2f}% [{:+.2f}{}]", asset, p, price_change, preferred_currency_symbol)));
            *values << (change * map_shares.at(asset).share);
            series->append(values);
        }

        auto chart = new QChart();
        chart->setAnimationOptions(QChart::AllAnimations);
        chart->setTheme(QChart::ChartThemeBlueCerulean);
        chart->legend()->setAlignment(Qt::AlignLeft);
        chart->setTitle("24h portfolio value change");

        auto view = new QChartView(chart);
        view->chart()->addSeries(series);
        view->setRenderHint(QPainter::Antialiasing);
        return view;
    };

    tabs->addTab(gui::chart::shares(shares, format_percent_shares), "shares");
    tabs->addTab(gui::chart::shares(shares, format_percent_shares, 0.00, 2.00), "shares small");
    tabs->addTab(gui::chart::shares(shares, format_prices_shares), "prices");
    tabs->addTab(gui::chart::shares(shares, format_prices_shares, 0.00, 2.00), "prices small");
    tabs->addTab(generate_24h_change_chart(), "24h change");
    tabs->addTab(generate_24h_value_chart(), "24h by value");

    tabs->resize(1366, 768);
    tabs->show();

    return app.exec();
}
