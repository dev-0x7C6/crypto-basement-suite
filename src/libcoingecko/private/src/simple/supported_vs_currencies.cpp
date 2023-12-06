#include "libcoingecko/v3/simple/supported_vs_currencies.hpp"
#include "api.hpp"

namespace simple = coingecko::v3::simple;

auto simple::supported_vs_currencies::query(const options &opts) -> std::expected<strings, error> {
    const auto json = request("simple/supported_vs_currencies", opts);
    if (!json) return std::unexpected(json.error());

    std::vector<std::string> ret;
    for (auto &&item : json.value())
        ret.emplace_back(item);

    return ret;
}
