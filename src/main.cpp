#include <indicators/indicators.hpp>
#include <indicators/exponential_moving_average.hpp>
#include <indicators/moving_average_convergence_divergence.hpp>
#include <indicators/moving_average.hpp>
#include <indicators/price_velocity.hpp>
#include <indicators/rate_of_change.hpp>
#include <indicators/relative_strength_index.hpp>
#include <indicators/stochastic_oscillator.hpp>

#include <model/model.hpp>
#include <model/stub.hpp>
#include <model/tests.hpp>
#include <types.hpp>

#include <chrono>
#include <iostream>

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <range/v3/all.hpp>

#include <networking/downloader.hpp>

#include <QCoreApplication>

using namespace ranges;
using namespace std::chrono_literals;

auto main(int argc, char **argv) -> int {
    QCoreApplication application(argc, argv);

    provider::stub stub{};
    auto console = spdlog::stdout_color_mt("console");

    network::downloader dl;
    dl.download({"https://api.alternative.me/fng/"}, [&](const QByteArray &data) {
        spdlog::info("json: {}", QString::fromUtf8(data).toStdString());
        application.quit();
    });

    application.exec();

    indicator::moving_average MA_stride_test{};
    indicator::exponential_moving_average EMA_stride_test{};
    indicator::price_velocity PV_stride_test{};
    indicator::rate_of_change ROC_stride_test{};
    indicator::relative_strength_index RSI_stride_test{};
    indicator::stochastic_oscillator SO_stride_test{};
    indicator::ma_convergence_divergence MAcd_stride_test{};

    std::vector<types::indicator_value> MA_values;
    std::vector<types::indicator_value> EMA_values;
    std::vector<types::indicator_value> PV_values;
    std::vector<types::indicator_value> ROC_values;

    for (auto &&subrange : stub.range().to_range() | views::stride(60) | views::sliding(25)) {
        const auto MA_value = MA_stride_test.compute(subrange, stub).value;
        const auto EMA_value = EMA_stride_test.compute(subrange, stub).value;
        const auto PV_value = PV_stride_test.compute(subrange, stub).value;
        const auto ROC_value = ROC_stride_test.compute(subrange, stub).value;

        MA_values.emplace_back(MA_value);
        EMA_values.emplace_back(EMA_value);
        PV_values.emplace_back(PV_value);
        ROC_values.emplace_back(ROC_value);

        spdlog::info("MA: {}", MA_value);
        spdlog::info("EMA: {}", EMA_value);
        spdlog::info("PV: {}", PV_value);
        spdlog::info("ROC: {}", ROC_value);

        spdlog::info("RSI: {}", RSI_stride_test.compute(subrange, stub).value);
        spdlog::info("SO: {}", SO_stride_test.compute(subrange, stub).value);
        spdlog::info("MACD: {}", MAcd_stride_test.compute(subrange, stub).value);
    }

    return 0;
}
