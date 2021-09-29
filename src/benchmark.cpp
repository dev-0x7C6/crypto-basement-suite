#include <benchmark/benchmark.h>
#include <indicators/moving-average.hpp>
#include <model/stub.hpp>
#include <range/v3/all.hpp>
#include <types.hpp>

using namespace ranges;

static void ma_compute_with_range(benchmark::State &state) {
    provider::stub stub{};
    indicator::moving_average ma{};

    while (state.KeepRunning()) {
        auto ret = 0.0;
        for (auto &&subrange : stub.range().to_range() | views::stride(60) | views::sliding(25)) {
            ret += ma.compute(subrange, stub).value;
        }

        benchmark::DoNotOptimize(ret);
    }
}

static void ma_compute(benchmark::State &state) {
    provider::stub stub{};
    indicator::moving_average ma{};

    while (state.KeepRunning()) {
        indicator::moving_average MA_stride_test{};
        auto ret = 0.0;
        provider::iterate(
            stub, [&](const types::time_point t, const types::currency curr_data) {
                ma.load_data(curr_data);
                ret += ma.compute_value(t).value;
            },
            60s);
        benchmark::DoNotOptimize(ret);
    }
}

BENCHMARK(ma_compute_with_range);
BENCHMARK(ma_compute);

auto main(int argc, char **argv) -> int {
    ::benchmark::Initialize(&argc, argv);
    ::benchmark::RunSpecifiedBenchmarks();
    return 0;
}
