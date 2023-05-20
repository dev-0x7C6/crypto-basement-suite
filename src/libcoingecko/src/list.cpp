#include "libcoingecko/v3/coins/list.hpp"
#include "api.hpp"

#include <fmt/format.h>
#include <range/v3/all.hpp>

using namespace ranges;
using namespace coingecko::v3::coins;

namespace coingecko::v3::coins {

auto list(bool include_platform, const options &opts) -> std::expected<coins, error> {
    const auto json = request(fmt::format("coins/list?include_platform={}", include_platform), opts);
    if (!json) return std::unexpected(json.error());

    coins ret;
    for (auto &&object : json.value()) {
        struct coin coin;
        set(object, "id", coin.id);
        set(object, "symbol", coin.symbol);
        set(object, "name", coin.name);
        set(object, "platforms", coin.platforms);

        ret.emplace_back(std::move(coin));
    }

    return ret;
}

} // namespace coingecko::v3::coins
