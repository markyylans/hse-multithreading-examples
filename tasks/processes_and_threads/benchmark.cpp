#include "apply_function.hpp"

#include <benchmark/benchmark.h>

#include <format>
#include <vector>
#include <numeric>
#include <cmath>

static void BM_LightWork(benchmark::State& state) {
    auto size = state.range(0);
    auto threads = state.range(1);
    std::vector<int> data(size);

    for (auto _ : state) {
        state.PauseTiming();
        std::iota(data.begin(), data.end(), 0);
        state.ResumeTiming();

        ApplyFunction<int>(data, [] (int& x) { x += 1; }, threads);
    }
}

BENCHMARK(BM_LightWork)
    ->Args({100, 1})
    ->Args({100, 4})
    ->Args({1000, 1})
    ->Args({1000, 4})
    ->Args({100000, 1})
    ->Args({100000, 4})
    ->Unit(benchmark::kMicrosecond)
    ->UseRealTime();

static void BM_HeavyWork(benchmark::State& state) {
    auto size = state.range(0);
    auto threads = state.range(1);
    std::vector<double> data(size);

    for (auto _ : state) {
        state.PauseTiming();
        for (int i = 0; i < size; ++i) {
            data[i] = i + 1;
        }
        state.ResumeTiming();

        ApplyFunction<double>(data, [] (double& x) {
            auto _ = std::format("{:.100f}", x);
        }, threads);
    }
}

BENCHMARK(BM_HeavyWork)
    ->Args({100, 1})
    ->Args({100, 4})
    ->Args({1000, 1})
    ->Args({1000, 4})
    ->Args({5000, 1})
    ->Args({5000, 4})
    ->Unit(benchmark::kMillisecond)
    ->UseRealTime();

BENCHMARK_MAIN();
