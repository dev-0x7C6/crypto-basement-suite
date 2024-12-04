#pragma once

#include <chrono>
#include <vector>

namespace bitcoin::halving {

auto trivial_next_halving_approximation() -> std::vector<std::chrono::year_month_day>;
}
