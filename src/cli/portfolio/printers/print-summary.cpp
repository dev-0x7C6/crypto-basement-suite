#include "print-shares.hpp"

#include <format>
#include <ranges>
#include <set>

#include <helpers/formatter.hpp>

using namespace std;
using namespace std::ranges;

namespace printers {

void summary(
    const portfolio &total,
    const configuration &config,
    std::shared_ptr<spdlog::logger> logger) {

    logger->info("\n+ total");

    set<string> hide_ranks{symbol::btc};
    map<string, int> preferred_decimal_count{{symbol::btc, 8}};

    vector<pair<string, double>> total_sorted;
    auto total_vec = total | ranges::to<vector<pair<string, double>>>();

    const static map<string, int> sort_rank{
        {symbol::btc, 2},
        {config.preferred_currency, 1},
    };

    std::ranges::sort(total_vec, [&](auto &&lhs, auto &&rhs) {
        return value_or(sort_rank, lhs.first, 0) > value_or(sort_rank, rhs.first, 0);
    });

    for (auto &&[currency, valuation] : total_vec) {
        const auto demical = value_or(preferred_decimal_count, currency, 2);
        const auto hide_rank = hide_ranks.contains(currency);
        const auto price = format::formatted_price(valuation, config, hide_rank, demical);
        const auto symbol = format::to_symbol(currency);
        logger->info(" -> {} {}", price, symbol);
    }
}

} // namespace printers
