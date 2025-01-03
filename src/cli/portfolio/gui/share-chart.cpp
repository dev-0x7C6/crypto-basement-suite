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

auto create_chart(QPieSeries *series, const QString &title = {}) -> QChart * {
    series->setLabelsVisible(true);
    series->setLabelsPosition(QPieSlice::LabelOutside);

    auto chart = new QChart();
    chart->setAnimationOptions(QChart::AllAnimations);
    chart->setTheme(QChart::ChartThemeBlueCerulean);
    chart->legend()->setAlignment(Qt::AlignLeft);
    chart->legend()->setReverseMarkers(true);
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

    for (auto &&s : shares | std::ranges::views::reverse) {
        if (s.share < min) continue;
        if (s.share > max) continue;

        series->append(QString::fromStdString(query_price(s)), s.share);
    }

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

    return create_view(create_chart(series, "bitcoin / altcoin ratio"));
};
