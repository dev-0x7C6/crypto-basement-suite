#include "share-chart.hpp"

#include <QtCharts/QPieSeries>
#include <QtCharts>

#include <ranges>

using namespace gui::chart;

auto create_pie_series() -> QPieSeries * {
    auto series = new QPieSeries();
    series->setHoleSize(0.5);
    series->setPieSize(0.8);
    return series;
}

template <typename T>
auto create_chart(T *series, const QString &title = {}) -> QChart * {
    auto chart = new QChart();
    chart->setAnimationOptions(QChart::AllAnimations);
    chart->setTheme(QChart::ChartThemeBlueCerulean);
    chart->legend()->setAlignment(Qt::AlignLeft);
    chart->addSeries(series);
    chart->setTitle(title);
    return chart;
}

auto create_view(auto *chart) -> QChartView * {
    auto view = new QChartView(chart);
    view->setRenderHint(QPainter::Antialiasing);
    return view;
}

auto gui::chart::shares(const shares::shares_vec &shares, const query_price_fn &query_price, double min, double max) -> QChartView * {
    auto series = create_pie_series();
    series->setPieStartAngle(min * 2);

    for (auto &&s : shares) {
        if (s.share < min) continue;
        if (s.share > max) continue;

        series->append(QString::fromStdString(query_price(s)), s.share);
    }

    series->setLabelsVisible(true);
    series->setLabelsPosition(QPieSlice::LabelOutside);
    return create_view(create_chart(series, "shares"));
};

auto gui::chart::bitcoin_altcoin_ratio(const shares::shares_map &shares) -> QChartView * {
    if (!shares.contains("bitcoin"))
        return nullptr;

    auto series = create_pie_series();
    const auto bitcoin_ratio = shares.at("bitcoin").share;
    const auto altcoin_ratio = 100.0 - bitcoin_ratio;

    series->append(QString::fromStdString(std::format("bitcoin {:.2f}%", bitcoin_ratio)), bitcoin_ratio);
    series->append(QString::fromStdString(std::format("altcoin {:.2f}%", altcoin_ratio)), altcoin_ratio);

    series->setLabelsVisible(true);
    series->setLabelsPosition(QPieSlice::LabelOutside);
    return create_view(create_chart(series, "bitcoin / altcoin ratio"));
};

auto gui::chart::day_change(shares::shares_vec shares, const std::map<std::string, double> &day_change) -> QChartView * {
    std::ranges::sort(shares, [&](auto &&l, auto &&r) {
        return day_change.at(l.asset) > day_change.at(r.asset);
    });

    auto series = new QBarSeries();

    auto _24h_sorted = day_change | std::ranges::to<std::vector<std::pair<std::string, double>>>();

    std::ranges::sort(_24h_sorted, [&](auto &&l, auto &&r) { return l.second > r.second; });

    for (auto &&[asset, change] : _24h_sorted) {
        auto values = new QBarSet(QString::fromStdString(std::format("{} {:.2f}%", asset, change)));
        *values << change;
        series->append(values);
    }

    return create_view(create_chart(series, "day change"));
};

auto gui::chart::day_value(shares::shares_vec shares, shares::query_price_fn query_price, const std::map<std::string, double> &day_change) -> QChartView * {
    std::ranges::sort(shares, [&](auto &&l, auto &&r) {
        return day_change.at(l.asset) > day_change.at(r.asset);
    });

    auto series = new QBarSeries();

    auto day_sorted = day_change | std::ranges::to<std::vector<std::pair<std::string, double>>>();
    auto map_shares = shares::to_map(shares);

    std::ranges::sort(day_sorted, [&](auto &&l, auto &&r) { return (l.second * map_shares.at(l.first).share) > (r.second * map_shares.at(r.first).share); });

    for (auto &&[asset, change] : day_sorted) {
        const auto s = map_shares.at(asset);
        const auto [currency, value] = query_price(asset).value();
        const auto valuation = value * s.quantity;
        const auto price_change = valuation - (valuation / (1.00 + (change / 100.0)));

        auto values = new QBarSet(QString::fromStdString(std::format("{} {:+.2f}{}", asset, price_change, currency)));
        *values << (change * map_shares.at(asset).share);
        series->append(values);
    }

    return create_view(create_chart(series, "day value"));
};
