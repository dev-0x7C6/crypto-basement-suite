#include "api.hpp"
#include <libcoingecko/v3/global/global.hpp>

namespace coingecko::v3::global {

auto list(const options &opts) -> std::expected<data, error> {
    const auto json = request("global", opts);
    if (!json) return std::unexpected(json.error());

    auto &&src = json.value()["data"];
    struct data data;
    set(src, "active_cryptocurrencies", data.active_cryptocurrencies);
    set(src, "upcoming_icos", data.upcoming_icos);
    set(src, "ongoing_icos", data.ongoing_icos);
    set(src, "ended_icos", data.ended_icos);
    set(src, "markets", data.markets);
    set(src, "total_market_cap", data.total_market_cap);
    set(src, "total_volume", data.total_volume);
    set(src, "market_cap_percentage", data.market_cap_percentage);
    set(src, "market_cap_change_percentage_24h_usd", data.market_cap_change_percentage_24h_usd);
    set(src, "updated_at", data.updated_at);

    return data;
}

} // namespace coingecko::v3::global
