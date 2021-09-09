#include <model/model.hpp>
#include <model/stub.hpp>
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

    return 0;
}
