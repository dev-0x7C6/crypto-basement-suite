#pragma once

#include <functional>
#include <map>
#include <optional>
#include <string>
#include <vector>

using portfolio = std::map<std::string, double>;

namespace shares {

struct share {
    std::string asset;
    double share{};
    double quantity{};

    constexpr auto operator<=>(const auto &other) const noexcept {
        return this->share > other.share;
    };
};

using shares = std::vector<share>;

using query_price_fn = std::function<std::optional<double>(const std::string &asset)>;

auto calculate(const portfolio &portfolio, query_price_fn &&query_price, double total) -> std::vector<share>;
} // namespace shares
