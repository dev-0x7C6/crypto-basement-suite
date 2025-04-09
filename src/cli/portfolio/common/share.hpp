#pragma once

#include <expected>
#include <functional>
#include <map>
#include <optional>
#include <string>
#include <utility>
#include <vector>

using portfolio = std::map<std::string, double>;
using currency_quantity = std::pair<std::string, double>;

namespace shares {

enum class error {
    unknown_asset_price,
};

struct share {
    std::string asset;
    double share{};
    double quantity{};
};

constexpr auto operator<=>(const share &lhs, const share &rhs) noexcept {
    return lhs.share > rhs.share;
}

using shares_vec = std::vector<share>;
using shares_map = std::map<std::string, share>;

using query_price_fn = std::function<std::optional<currency_quantity>(const std::string &asset)>;

auto calculate(const portfolio &portfolio, query_price_fn &&query_price, double total) -> std::expected<shares_vec, error>;
auto to_map(const shares_vec &) -> shares_map;
} // namespace shares
