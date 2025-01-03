#pragma once

#include "common/share.hpp"

class QChartView;

namespace gui::chart {

using query_price_fn = std::function<std::string(const shares::share &)>;

auto shares(const shares::shares &shares, const query_price_fn &formatter, double min = 2.00, double max = 100.0) -> QChartView *;
auto bitcoin_altcoin_ratio(const shares::shares &shares) -> QChartView *;

} // namespace gui::chart
