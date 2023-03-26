#include "libcoingecko/v3/ping.hpp"
#include "api.hpp"

#include <fmt/format.h>

auto coingecko::v3::ping() -> bool {
    const auto url = fmt::format("{}/ping", api);
    const auto json = request(url);
    return !json.empty() && json.contains("gecko_says");
}
