#include <model/model.hpp>
#include <model/stub.hpp>
#include <model/indicators.hpp>
#include <model/tests.hpp>
#include <model/types.hpp>

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
    MA_ind.configure(std::list<std::tuple<std::string, std::string>>());
    EMA_ind.configure(std::list<std::tuple<std::string, std::string>>());
    //generate data

    provider::iterate(
        stub, [&MA_ind, &EMA_ind](const types::time_point t, const types::currency curr_data) {
            std::cout << "p:" << curr_data.price << ", " << curr_data.time_stamp;
            // load data point into indicator
            MA_ind.load_data(curr_data);
            EMA_ind.load_data(curr_data);
            // compute indicator value for any time stamp from loaded data
            std::cout << " MA: " << MA_ind.compute_value(t).value << " EMA: " << EMA_ind.compute_value(t).value << std::endl;
        },
        60s);

    return 0;
}
