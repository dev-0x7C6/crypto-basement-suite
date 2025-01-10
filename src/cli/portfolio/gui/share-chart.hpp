#pragma once

#include "common/share.hpp"

class QChartView;

namespace gui::chart {

using query_price_fn = std::function<std::string(const shares::share &)>;

auto shares(const shares::shares_vec &shares, const query_price_fn &formatter, double min = 2.00, double max = 100.0) -> QChartView *;
auto bitcoin_altcoin_ratio(const shares::shares_map &shares) -> QChartView *;
auto day_change(shares::shares_vec shares, const std::map<std::string, double> &day_change) -> QChartView *;
auto day_value(shares::shares_vec shares, shares::query_price_fn query_price, const std::map<std::string, double> &day_change) -> QChartView *;

} // namespace gui::chart
