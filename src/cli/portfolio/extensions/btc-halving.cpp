#include "btc-halving.hpp"

#include <algorithm>
#include <ranges>

namespace bitcoin::halving {

auto trivial_next_halving_aproximation() -> std::vector<std::chrono::year_month_day> {
    using namespace std::chrono;

    const std::vector<year_month_day> btc_halving_date_table{
        {2009y / January / 3},
        {2012y / November / 28},
        {2016y / July / 9},
        {2020y / May / 11},
        {2024y / April / 19}};

    std::vector<int> btc_halving_day_count;

    for (auto &&compare : btc_halving_date_table | std::ranges::views::slide(2)) {
        const auto start = sys_days(compare.front());
        const auto end = sys_days(compare.back());
        btc_halving_day_count.push_back(days(end - start).count());
    }

    const auto sum = std::ranges::fold_left(btc_halving_day_count, int(0), std::plus<int>());
    const auto avg = sum / btc_halving_day_count.size();

    const auto last_halving = sys_days(btc_halving_date_table.back());
    const auto next_halving = year_month_day(last_halving + days(avg));

    return {next_halving};
};
;

} // namespace bitcoin::halving
