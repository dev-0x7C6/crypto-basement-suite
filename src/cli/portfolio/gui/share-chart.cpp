#include "share-chart.hpp"

#include <QtCharts/QPieSeries>
#include <QtCharts>

using namespace gui::chart;

auto gui::chart::shares(const shares::shares &shares, const query_price_fn &query_price, double min, double max) -> QChartView * {
    auto series = new QPieSeries();
    series->setHoleSize(0.2);

    for (auto &&s : shares) {
        if (s.share < min) continue;
        if (s.share > max) continue;

        auto slice = series->append(QString::fromStdString(query_price(s)), s.share);
        slice->setLabelArmLengthFactor(0.3);

        series->setLabelsVisible(true);
        series->setLabelsPosition(QPieSlice::LabelOutside);
    }

    auto chart = new QChart();
    chart->setAnimationOptions(QChart::AllAnimations);
    chart->setTheme(QChart::ChartThemeDark);
    chart->legend()->setAlignment(Qt::AlignLeft);

    auto view = new QChartView(chart);
    view->chart()->addSeries(series);

    view->setRenderHint(QPainter::Antialiasing);
    return view;
};
