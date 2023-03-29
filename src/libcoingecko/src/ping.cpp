#include "libcoingecko/v3/ping.hpp"
#include "api.hpp"

#include <fmt/format.h>

auto coingecko::v3::ping(const options &opts) -> bool {
    const auto json = request("ping", opts);
    return !json.empty() && json.contains("gecko_says");
}
