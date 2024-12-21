#include "share-chart.hpp"

#include <QtCharts/QPieSeries>
#include <QtCharts>

#include <ranges>

using namespace gui::chart;

auto gui::chart::shares(const shares::shares &shares, const query_price_fn &query_price, double min, double max) -> QChartView * {
    auto series = new QPieSeries();
    series->setHoleSize(0.5);
    series->setPieSize(0.8);
    series->setPieStartAngle(min * 2);

    for (auto &&s : shares | std::ranges::views::reverse) {
        if (s.share < min) continue;
        if (s.share > max) continue;

        auto slice = series->append(QString::fromStdString(query_price(s)), s.share);
        series->setLabelsVisible(true);
        series->setLabelsPosition(QPieSlice::LabelOutside);
    }

    auto chart = new QChart();
    chart->setAnimationOptions(QChart::AllAnimations);
    chart->setTheme(QChart::ChartThemeBlueCerulean);
    chart->legend()->setAlignment(Qt::AlignLeft);
    chart->legend()->setReverseMarkers(true);
    chart->setTitle("shares");
    chart->addSeries(series);

    auto view = new QChartView(chart);
    view->setRenderHint(QPainter::Antialiasing);
    return view;
};
