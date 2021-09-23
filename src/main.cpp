#include <indicators/exponential-moving-average.hpp>
#include <indicators/indicators.hpp>
#include <indicators/moving-average.hpp>
#include <indicators/price_velocity.hpp>
#include <indicators/rate_of_change.hpp>
#include <indicators/relative_strength_index.hpp>
#include <indicators/stochastic_oscillator.hpp>
#include <indicators/ma_convergence_divergence.hpp>
#include <model/model.hpp>
#include <model/stub.hpp>
#include <model/tests.hpp>
#include <types.hpp>

#include <chrono>
#include <iostream>

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <range/v3/all.hpp>

using namespace std::chrono_literals;

auto main(int, char **) -> int {
    provider::stub stub{};

    auto console = spdlog::stdout_color_mt("console");

    for (auto &&t : stub.range().to_range())
        spdlog::debug("price: {}, timestamp: {}", stub.value(t).price, t);

    provider::iterate(stub, [](const types::time_point t, const types::currency value) {
        spdlog::debug("price: {}, timestamp: {}", value.price, t.point);
    });

    indicator::moving_average MA_ind_debug{};
    for (auto &&sub : stub.range().to_range() | ::ranges::views::chunk(25)) {
        spdlog::info("MA: {}", MA_ind_debug.compute(sub, stub).value);
    }

    indicator::moving_average MA_ind{};
    indicator::exponential_moving_average EMA_ind{};
    indicator::price_velocity PV_ind{};
    indicator::rate_of_change ROC_ind{};
    indicator::relative_strength_index RSI_ind{};
    indicator::stochastic_oscillator SO_ind{};
    indicator::ma_convergence_divergence MAcd_ind{};
    //generate data
    provider::iterate(
        stub, [&](const types::time_point t, const types::currency curr_data) {
            spdlog::info("---------------------------------------------------------------------");
            spdlog::info("price: {}, timestamp: {}", curr_data.price, t.point);
            // load data point into indicator
            MA_ind.load_data(curr_data);
            EMA_ind.load_data(curr_data);
            PV_ind.load_data(curr_data);
            ROC_ind.load_data(curr_data);
            RSI_ind.load_data(curr_data);
            SO_ind.load_data(curr_data);
            MAcd_ind.load_data(curr_data);
            // compute indicator value for any time stamp from loaded data
            const auto MA = MA_ind.compute_value(t).value;
            const auto EMA = EMA_ind.compute_value(t).value;
            const auto PV = PV_ind.compute_value(t).value;
            const auto ROC = ROC_ind.compute_value(t).value;
            const auto RSI = RSI_ind.compute_value(t).value;
            const auto SO = SO_ind.compute_value(t).value;
            const auto MAcd = MAcd_ind.compute_value(t).value;

            spdlog::info("MA: {}, EMA: {}, PV: {}, ROC: {}", MA, EMA, PV, ROC);
            spdlog::info("RSI: {}, SO: {}, MAcd: {}", RSI, SO, MAcd);
        },
        60s);

    return 0;
}
