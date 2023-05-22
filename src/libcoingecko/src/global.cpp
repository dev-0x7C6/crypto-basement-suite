#include "libcoingecko/v3/global/global.hpp"
#include "api.hpp"

namespace coingecko::v3::global {

auto list(const options &opts) -> std::expected<data, error> {
    const auto json = request("global", opts);
    if (!json) return std::unexpected(json.error());

    struct data data;
    set(json, "active_cryptocurrencies", data.active_cryptocurrencies);
    set(json, "upcoming_icos", data.upcoming_icos);
    set(json, "ongoing_icos", data.ongoing_icos);
    set(json, "ended_icos", data.ended_icos);
    set(json, "markets", data.markets);
    set(json, "total_market_cap", data.total_market_cap);
    set(json, "total_volume", data.total_volume);
    set(json, "market_cap_percentage", data.market_cap_percentage);
    set(json, "market_cap_change_percentage_24h_usd", data.market_cap_change_percentage_24h_usd);
    set(json, "updated_at", data.updated_at);

    return data;
}

} // namespace coingecko::v3::global
