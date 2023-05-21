#include <iostream>

#include <libcoingecko/v3/ping.hpp>

auto main(int argc, char **argv) -> int {
    const auto result = coingecko::v3::ping();

    if (!result) {
        std::cerr << "coingecko: error" << std::endl;
        return 1;
    }

    std::cout << "coingecko: pong" << std::endl;
    return {};
}
