#include <QApplication>
#include <QGroupBox>
#include <QtCharts/QBarSeries>
#include <QtCharts/QBarSet>
#include <QtCharts/QPieLegendMarker>
#include <QtCharts/QPieSlice>
#include <QtCharts/QtCharts>

#include "gui/gui.hpp"
#include "gui/share-chart.hpp"
#include "helpers/formatter.hpp"
#include "libcoingecko/v3/simple/price.hpp"

using namespace std;

auto show_gui_charts(int argc, char **argv, const configuration &config, const shares::shares_vec &shares, const coingecko::v3::simple::price::prices &summary) -> int {
    const auto preferred_currency_symbol = format::to_symbol(config.preferred_currency);

    auto price = [&summary](const string &asset, const string &currency) -> optional<currency_quantity> {
        try {
            return currency_quantity{currency, summary.at(asset).at(currency).value};
        } catch (...) {}

        return {};
    };

    QApplication app(argc, argv);
    auto tabs = new QTabWidget();

    auto format_percent_shares = [](const shares::share &share) -> std::string {
        return std::format("{} {:.2f}%", share.asset, share.share);
    };

    auto format_prices_shares = [&](const shares::share &share) -> std::string {
        const auto value = price(share.asset, config.preferred_currency).value().second * share.quantity;
        return std::format("{} {:.0f}{}", share.asset, value, preferred_currency_symbol);
    };

    auto query_price_pref = [&](const string &asset) -> optional<currency_quantity> {
        return price(asset, config.preferred_currency).value();
    };

    tabs->addTab(gui::chart::shares(shares, format_percent_shares), "shares");
    tabs->addTab(gui::chart::shares(shares, format_percent_shares, 0.00, 2.00), "shares small");
    tabs->addTab(gui::chart::shares(shares, format_prices_shares), "prices");
    tabs->addTab(gui::chart::shares(shares, format_prices_shares, 0.00, 2.00), "prices small");
    tabs->addTab(gui::chart::bitcoin_altcoin_ratio(shares::to_map(shares)), "btc/alt ratio");
    // tabs->addTab(gui::chart::day_change(shares, _24h_change), "24h change");
    // tabs->addTab(gui::chart::day_value(shares, query_price_pref, _24h_change), "24h by value");
    tabs->resize(1366, 768);
    tabs->show();

    return app.exec();
}
