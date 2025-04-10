#include "libcoingecko/v3/coins/list.hpp"
#include "api.hpp"

#include <format>

using namespace coingecko::v3::coins;

namespace coingecko::v3::coins::list {

auto query(const settings &s, const options &opts) -> std::expected<coins, error> {
    const auto json = request(std::format("coins/list?include_platform={}", s.include_platform), opts);
    if (!json) return std::unexpected(json.error());

    coins ret;
    for (auto &&object : json.value()) {
        struct coin coin;
        set(object, "id", coin.id);
        set(object, "symbol", coin.symbol);
        set(object, "name", coin.name);
        if (s.include_platform && object.contains("platforms"))
            for (auto &&[key, value] : object["platforms"].items())
                if (value.is_string())
                    coin.platforms[key] = value.get<std::string>();

        ret.emplace_back(std::move(coin));
    }

    return ret;
}

} // namespace coingecko::v3::coins::list
