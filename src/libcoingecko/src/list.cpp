#include "libcoingecko/v3/coins/list.hpp"
#include "api.hpp"

#include <fmt/format.h>
#include <range/v3/all.hpp>

using namespace ranges;
using namespace coingecko::v3::coins;

namespace coingecko::v3::coins {

auto list(bool include_platform) -> coins {
    const auto url = fmt::format("{}/coins/list?include_platform={}", api, include_platform);
    const auto json = request(url);
    if (json.empty()) return {};

    coins ret;
    for (auto &&object : json) {
        struct coin coin;
        set(object, "id", coin.id);
        set(object, "symbol", coin.symbol);
        set(object, "name", coin.name);

        if (object.contains("platforms"))
            for (auto &&[platform, contract] : object["platforms"].items())
                if (contract.is_string())
                    coin.platforms[platform] = contract.get<std::string>();

        ret.emplace_back(std::move(coin));
    }

    return ret;
}

} // namespace coingecko::v3::coins
