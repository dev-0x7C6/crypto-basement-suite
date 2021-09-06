#include <model/model.hpp>
#include <model/stub.hpp>

#include <chrono>
#include <iostream>

using namespace std::chrono_literals;

auto main(int, char **) -> int {
    currency::data::provider::stub stub{};

    currency::data::provider::iterate(stub, 1s, [](auto &&t) {
        std::cout << t.timestamp << std::endl;
    });

    return 0;
}
