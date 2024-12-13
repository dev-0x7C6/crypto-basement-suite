#pragma once

#include "common/share.hpp"

class QChartView;

namespace gui::chart {

using query_price_fn = std::function<std::string(const shares::share &)>;

auto shares(const shares::shares &shares, const query_price_fn &formatter, double min = 2.00, double max = 100.0) -> QChartView *;

} // namespace gui::chart
