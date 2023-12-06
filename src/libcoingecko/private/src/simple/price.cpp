#include "libcoingecko/v3/simple/price.hpp"
#include "api.hpp"

#include <format>
#include <range/v3/all.hpp>

using namespace std::literals;
using namespace coingecko::v3::simple;
using namespace nlohmann;
using namespace ranges;

namespace coingecko::v3::simple::price {

namespace {
auto from_json(const json &j, const std::string &currency) -> std::pair<std::string, struct price> {
    struct price ret;
    set(j, currency, ret.value);
    set(j, std::format("{}_24h_change", currency), ret.change_24h);
    set(j, std::format("{}_market_cap", currency), ret.market_cap);
    set(j, std::format("{}_24h_vol", currency), ret.volume_24h);
    return {currency, ret};
}
} // namespace

auto query(const parameters &query, const options &opts) -> std::expected<prices, error> {
    if (query.ids.empty()) return {};
    if (query.vs_currencies.empty()) return {};

    constexpr auto comma = "%2C"sv;

    const auto ids = query.ids | views::join(comma) | to<std::string>();
    const auto vs = query.vs_currencies | views::join(comma) | to<std::string>();
    const auto params = {
        std::format("ids={}", ids),
        std::format("vs_currencies={}", vs),
        std::format("include_market_cap={}", query.include_market_cap),
        std::format("include_24hr_vol={}", query.include_24hr_vol),
        std::format("include_24hr_change={}", query.include_24hr_change),
        std::format("include_last_updated_at={}", query.include_last_updated_at),
    };

    const auto url_params = params | views::join('&') | to<std::string>();
    const auto json = request(std::format("simple/price?{}", url_params), opts);
    if (!json) return std::unexpected(json.error());

    prices ret;

    for (auto &&[key, value] : json.value().items())
        for (auto &&currency : query.vs_currencies)
            ret[key].emplace(from_json(value, currency));

    return ret;
}

} // namespace coingecko::v3::simple::price
