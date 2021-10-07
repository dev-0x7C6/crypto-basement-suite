#include <benchmark/benchmark.h>
#include <model/stub.hpp>
#include <range/v3/all.hpp>
#include <types.hpp>

#include <indicators/exponential-moving-average.hpp>
#include <indicators/moving-average.hpp>
#include <indicators/price_velocity.hpp>
#include <indicators/rate_of_change.hpp>

using namespace ranges;

namespace {

template <typename indicator_type>
void indicator_benchmark(benchmark::State &state) {
    provider::stub stub{};
    indicator_type test_object{};

    while (state.KeepRunning()) {
        auto ret = 0.0;
        for (auto &&subrange : stub.range().to_range() | views::stride(60) | views::sliding(25)) {
            ret += test_object.compute(subrange, stub).value;
        }

        benchmark::DoNotOptimize(ret);
    }
}
indicator::exponential_moving_average EMA_stride_test{};
indicator::price_velocity PV_stride_test{};
indicator::rate_of_change ROC_stride_test{};

void ma_compute(benchmark::State &state) { indicator_benchmark<indicator::moving_average>(state); }
void ema_compute(benchmark::State &state) { indicator_benchmark<indicator::exponential_moving_average>(state); }
void pv_compute(benchmark::State &state) { indicator_benchmark<indicator::price_velocity>(state); }
void roc_compute(benchmark::State &state) { indicator_benchmark<indicator::rate_of_change>(state); }
} // namespace

BENCHMARK(ma_compute);
BENCHMARK(ema_compute);
BENCHMARK(pv_compute);
BENCHMARK(roc_compute);

auto main(int argc, char **argv) -> int {
    ::benchmark::Initialize(&argc, argv);
    ::benchmark::RunSpecifiedBenchmarks();
    return 0;
}
