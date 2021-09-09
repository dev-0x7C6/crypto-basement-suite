#include <model/model.hpp>
#include <model/stub.hpp>

#include <chrono>
#include <iostream>

using namespace std::chrono_literals;

auto main(int, char **) -> int {
    currency::data::provider::stub stub{};

    currency::data::provider::iterate(stub, 1s, [](auto &&t, currency::data::provider::currency_details value) {
        std::cout << "p:" << value.price << ", " << t.timestamp << std::endl;
    });

    return 0;
}
