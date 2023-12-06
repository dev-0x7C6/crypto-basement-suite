#include <format>
#include <iostream>

#include <libblockfrost/v0/balance.hpp>

auto main(int argc, char **argv) -> int {
    const auto address = "addr1qy6qvd3szupa7ayqf6zw7cd0ple7w3yg5f3xh5gkkc4q9zmc9ty742qncmffaesxqarvqjmxmy36d9aht2duhmhvekgq52e2en";
    const auto result = blockfrost::v0::address_balance(address);

    if (!result) {
        std::cerr << "blockfrost: error" << std::endl;
        return 1;
    }

    std::cout << std::format("{}: amount: {:.3f}", address, *result) << std::endl;
    return {};
}
