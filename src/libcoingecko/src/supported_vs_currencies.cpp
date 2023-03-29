#include "libcoingecko/v3/coins/supported_vs_currencies.hpp"
#include "api.hpp"

#include <fmt/format.h>

auto coingecko::v3::coins::supported_vs_currencies(const options &opts) -> strings {
    const auto json = request("simple/supported_vs_currencies", opts);
    if (json.empty()) return {};

    std::vector<std::string> ret;
    for (auto &&item : json)
        ret.emplace_back(item);

    return ret;
}
