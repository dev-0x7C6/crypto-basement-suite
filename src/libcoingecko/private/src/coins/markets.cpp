#include "libcoingecko/v3/coins/markets.hpp"
#include "api.hpp"

#include <expected>
#include <format>
#include <ranges>
#include <string>
#include <vector>

using namespace std;
using namespace std::ranges;
using namespace std::ranges::views;

using namespace coingecko::v3::coins;

namespace coingecko::v3::coins {

auto markets(const markets_query &query, const options &opts) -> std::expected<std::vector<market_data>, error> {
    using namespace std;
    using namespace std::ranges;
    using namespace std::ranges::views;

    std::vector<std::string> params = {
        format("ids={}", query.ids | join_with(',') | to<string>()),
        format("vs_currency={}", query.vs_currency),
    };

    if (!query.category.empty())
        params.emplace_back(format("category={}", query.category));

    const auto url_params = params | join_with('&') | to<string>();
    const auto json = request(std::format("coins/markets?{}", url_params), opts);
    if (!json) return std::unexpected(json.error());

    std::vector<market_data> ret;
    for (auto &&object : json.value()) {
        struct market_data data;
        ::set(object, "id", data.id);
        ::set(object, "symbol", data.symbol);
        ::set(object, "name", data.name);
        ::set(object, "image", data.image);
        ::set(object, "current_price", data.current_price);
        ::set(object, "market_cap", data.market_cap);
        ::set(object, "market_cap_rank", data.market_cap_rank);
        ::set(object, "fully_diluted_valuation", data.fully_diluted_valuation);
        ::set(object, "total_volume", data.total_volume);
        ::set(object, "high_24h", data.high_24h);
        ::set(object, "low_24h", data.low_24h);
        ::set(object, "price_change_24h", data.price_change_24h);
        ::set(object, "price_change_percentage_24h", data.price_change_percentage_24h);
        ::set(object, "market_cap_change_24h", data.market_cap_change_24h);
        ::set(object, "market_cap_change_percentage_24h", data.market_cap_change_percentage_24h);

        ::set(object, "circulating_supply", data.circulating_supply);
        ::set(object, "total_supply", data.total_supply);
        ::set(object, "max_supply", data.max_supply);

        ::set(object, "ath", data.ath);
        ::set(object, "ath_change_percentage", data.ath_change_percentage);
        ::set(object, "ath_date", data.ath_date);
        ::set(object, "atl", data.atl);
        ::set(object, "atl_change_percentage", data.atl_change_percentage);
        ::set(object, "roi", data.roi);
        ::set(object, "last_updated", data.last_updated);

        ret.emplace_back(std::move(data));
    }

    return ret;
}

} // namespace coingecko::v3::coins

auto to_pairs(const std::vector<coingecko::v3::coins::market_data> &entries) -> std::vector<std::pair<std::string, coingecko::v3::coins::market_data>> {
    std::vector<std::pair<std::string, coingecko::v3::coins::market_data>> ret;
    for (auto &&entry : entries)
        ret.emplace_back(std::pair<std::string, coingecko::v3::coins::market_data>{entry.id, entry});

    return ret;
}

auto to_map(const std::vector<coingecko::v3::coins::market_data> &entries) -> std::map<std::string, coingecko::v3::coins::market_data> {
    std::map<std::string, coingecko::v3::coins::market_data> ret;
    for (auto &&entry : entries)
        ret.emplace(entry.id, entry);

    return ret;
}
