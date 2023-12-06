#include "libblockfrost/v0/balance.hpp"
#include "api.hpp"
#include <fmt/format.h>
#include <string>

auto blockfrost::v0::balance(const std::string &address, const options &opts) -> std::optional<double> {
    const auto json = request(fmt::format("addresses/{}", address), opts);
    if (!json) return {};

    auto &&data = *json;
    for (auto &&entry : data["amount"]) {
        if (entry.value("unit", "") == "lovelace")
            return std::stoll(entry.value("quantity", "0"), 0) / 1000000.0;
    }

    return {};
}
