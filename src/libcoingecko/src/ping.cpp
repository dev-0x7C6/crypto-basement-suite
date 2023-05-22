#include "libcoingecko/v3/ping.hpp"
#include "api.hpp"

auto coingecko::v3::ping(const options &opts) -> bool {
    const auto req = request("ping", opts);
    return req && !req.value().empty() && req.value().contains("gecko_says");
}
