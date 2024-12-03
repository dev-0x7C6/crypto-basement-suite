#include "libcoingecko/v3/simple/supported_vs_currencies.hpp"
#include "api.hpp"

#include <ranges>

namespace simple = coingecko::v3::simple;

auto simple::supported_vs_currencies::query(const options &opts) -> std::expected<std::set<std::string>, error> {
    const auto json = request("simple/supported_vs_currencies", opts);
    if (!json) return std::unexpected(json.error());

    return json.value() | std::ranges::to<std::set<std::string>>();
}
