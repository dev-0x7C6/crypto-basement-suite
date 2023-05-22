#include "libcoingecko/v3/coins/supported_vs_currencies.hpp"
#include "api.hpp"

auto coingecko::v3::coins::supported_vs_currencies(const options &opts) -> std::expected<strings, error> {
    const auto json = request("simple/supported_vs_currencies", opts);
    if (!json) return std::unexpected(json.error());

    std::vector<std::string> ret;
    for (auto &&item : json.value())
        ret.emplace_back(item);

    return ret;
}
