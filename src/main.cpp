#include <indicators/exponential-moving-average.hpp>
#include <indicators/indicators.hpp>
#include <indicators/moving-average.hpp>
#include <indicators/price_velocity.hpp>
#include <indicators/rate_of_change.hpp>
#include <model/model.hpp>
#include <model/stub.hpp>
#include <model/tests.hpp>
#include <types.hpp>

#include <chrono>
#include <iostream>

using namespace std::chrono_literals;

auto main(int, char **) -> int {
    provider::stub stub{};

    for (auto &&t : stub.range().to_range())
        std::cout << "p:" << stub.value(t).price << ", " << t << std::endl;

    provider::iterate(stub, [](const types::time_point t, const types::currency value) {
        std::cout << "p:" << value.price << ", " << t.point << std::endl;
    });

    indicator::moving_average MA_ind{};
    indicator::exponential_moving_average EMA_ind{};
    indicator::price_velocity PV_ind{};
    indicator::rate_of_change ROC_ind{};

    //generate data
    provider::iterate(
        stub, [&MA_ind, &EMA_ind, &PV_ind, &ROC_ind](const types::time_point t, const types::currency curr_data) {
            std::cout << "p:" << curr_data.price << ", " << curr_data.time_stamp.point;
            // load data point into indicator
            MA_ind.load_data(curr_data);
            EMA_ind.load_data(curr_data);
            PV_ind.load_data(curr_data);
            ROC_ind.load_data(curr_data);
            // compute indicator value for any time stamp from loaded data
            std::cout << " MA: " << MA_ind.compute_value(t).value << " EMA: " << EMA_ind.compute_value(t).value << " PV: "
            << PV_ind.compute_value(t).value << " ROC: " << ROC_ind.compute_value(t).value << std::endl;
        },
        60s);

    return 0;
}
