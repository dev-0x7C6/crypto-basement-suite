#include "libcoingecko/v3/coins/supported_vs_currencies.hpp"
#include "api.hpp"

#include <fmt/format.h>

auto coingecko::v3::coins::supported_vs_currencies() -> strings {
    const auto url = fmt::format("{}/simple/supported_vs_currencies", api);
    const auto json = request(url);
    if (json.empty()) return {};

    std::vector<std::string> ret;
    for (auto &&item : json)
        ret.emplace_back(item);

    return ret;
}
