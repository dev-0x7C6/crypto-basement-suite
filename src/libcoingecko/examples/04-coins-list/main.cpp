#include <iostream>

#include <libcoingecko/v3/coins/list.hpp>

struct coin {
    std::string id;
    std::string symbol;
    std::string name;
    std::unordered_map<std::string, std::string> platforms;
};

auto main(int argc, char **argv) -> int {
    const auto result = coingecko::v3::coins::list();

    if (!result) {
        std::cerr << "coingecko: unable to fetch coin list" << std::endl;
        return 1;
    }

    for (auto &&coin : result.value()) {
        std::cout << "id: " << coin.id << std::endl;
        for (auto &&[k, v] : coin.platforms)
            std::cout << " -> " << k << " " << v << std::endl;
        std::cout << std::endl;
    }

    return {};
}
