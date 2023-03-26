#include "libcoingecko/v3/coins/price.hpp"
#include "api.hpp"

#include <fmt/format.h>
#include <range/v3/all.hpp>

using namespace ranges;
using namespace coingecko::v3::coins;

namespace coingecko::v3::coins {

namespace {
auto from_json(const json &j, const std::string &currency) -> std::pair<std::string, struct price> {
    struct price ret;
    set(j, currency, ret.value);
    set(j, fmt::format("{}_24h_change", currency), ret.change_24h);
    set(j, fmt::format("{}_market_cap", currency), ret.market_cap);
    set(j, fmt::format("{}_24h_vol", currency), ret.volume_24h);
    return {currency, ret};
}
} // namespace

auto price(const price_query &query) -> prices {
    if (query.ids.empty()) return {};
    if (query.vs_currencies.empty()) return {};

    constexpr std::array<char, 3> comma = {'%', '2', 'C'};

    const auto ids = query.ids | views::join(comma) | to<std::string>();
    const auto vs = query.vs_currencies | views::join(comma) | to<std::string>();
    const auto params = {
        fmt::format("ids={}", ids),
        fmt::format("vs_currencies={}", vs),
        fmt::format("include_market_cap={}", query.include_market_cap),
        fmt::format("include_24hr_vol={}", query.include_24hr_vol),
        fmt::format("include_24hr_change={}", query.include_24hr_change),
        fmt::format("include_last_updated_at={}", query.include_last_updated_at),
    };

    const auto url_params = params | views::join('&') | to<std::string>();
    const auto url = fmt::format("{}/simple/price?{}", api, url_params);
    const auto json = request(url);
    if (json.empty()) return {};

    prices ret;

    for (auto &&[key, value] : json.items())
        for (auto &&currency : query.vs_currencies)
            ret[key].emplace(from_json(value, currency));

    return ret;
}

} // namespace coingecko::v3::coins
