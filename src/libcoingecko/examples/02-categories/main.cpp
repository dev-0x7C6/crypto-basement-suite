#include <iostream>

#include <libcoingecko/v3/categories/list.hpp>

auto main(int argc, char **argv) -> int {
    const auto result = coingecko::v3::coins::categories::list();

    if (!result) {
        std::cerr << "coingecko: unable to fetch categories" << std::endl;
        return 1;
    }

    std::cout << "categories:" << std::endl;
    for (auto &&category : result.value())
        std::cout << " -> " << category.id << " [" << category.name << "]" << std::endl;

    return {};
}
