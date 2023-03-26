#include "includes/coins/price.hpp"
#include "api.hpp"

#include <fmt/format.h>
#include <range/v3/all.hpp>

using namespace ranges;
using namespace coingecko::v3::coins;

namespace coingecko::v3::coins {

auto price(const options &opts) -> prices {
    if (opts.ids.empty()) return {};
    if (opts.vs_currencies.empty()) return {};

    constexpr std::array<char, 3> comma = {'%', '2', 'C'};

    const auto ids = opts.ids | views::join(comma) | to<std::string>();
    const auto vs = opts.vs_currencies | views::join(comma) | to<std::string>();
    const auto params = {
        fmt::format("ids={}", ids),
        fmt::format("vs_currencies={}", vs),
        fmt::format("include_market_cap={}", opts.include_market_cap),
        fmt::format("include_24hr_vol={}", opts.include_24hr_vol),
        fmt::format("include_24hr_change={}", opts.include_24hr_change),
        fmt::format("include_last_updated_at={}", opts.include_last_updated_at),
    };

    const auto url_params = params | views::join('&') | to<std::string>();
    const auto url = fmt::format("{}/simple/price?{}", api, url_params);
    const auto json = request(url);
    if (json.empty()) return {};

    prices ret;

    for (auto &&[key, value] : json.items()) {
        for (auto &&currency : opts.vs_currencies) {
            struct price valuation;
            valuation.value = get<double>(value, currency).value_or(0);
            valuation.change_24h = get<double>(value, fmt::format("{}_24h_change", currency)).value_or(0);
            valuation.market_cap = get<double>(value, fmt::format("{}_market_cap", currency)).value_or(0);
            valuation.volume_24h = get<double>(value, fmt::format("{}_24h_vol", currency)).value_or(0);
            ret[key].emplace(currency, valuation);
        }
    }

    return ret;
}

} // namespace coingecko::v3::coins
