#include "libcoingecko/v3/global/global.hpp"
#include "api.hpp"

#include <fmt/format.h>

namespace coingecko::v3::global {

auto list(const options &opts) -> std::optional<data> {
    const auto json = request("global", opts);
    if (json.empty()) return {};

    struct data data;
    data.active_cryptocurrencies = get<double>(json, "active_cryptocurrencies").value_or(0);
    data.upcoming_icos = get<double>(json, "upcoming_icos").value_or(0);
    data.ongoing_icos = get<double>(json, "ongoing_icos").value_or(0);
    data.ended_icos = get<double>(json, "ended_icos").value_or(0);
    data.markets = get<double>(json, "markets").value_or(0);
    return data;
}

} // namespace coingecko::v3::global
