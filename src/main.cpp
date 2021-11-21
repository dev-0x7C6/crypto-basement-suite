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

    indicator::indicator_results results;

    for (auto &&subrange : stub.range().to_range() | views::stride(60) | views::sliding(25)) {
        results.collect(indicator::type::moving_average, MA_stride_test.compute(subrange, stub));
        results.collect(indicator::type::exponential_moving_average, EMA_stride_test.compute(subrange, stub));
        results.collect(indicator::type::price_velocity, PV_stride_test.compute(subrange, stub));
        results.collect(indicator::type::rate_of_change, ROC_stride_test.compute(subrange, stub));
        results.collect(indicator::type::relative_strength_index, RSI_stride_test.compute(subrange, stub));
        results.collect(indicator::type::stochastic_oscillator, SO_stride_test.compute(subrange, stub));
        results.collect(indicator::type::moving_average_convergence_divergence, MAcd_stride_test.compute(subrange, stub));

        for (auto &&type : indicator::types)
            spdlog::info("{}: {}", to_string(type), results.last(type).value);
    }

    return 0;
}
