#include <benchmark/benchmark.h>
#include "lru-kvstore/kv_store.hpp"
#include <string>
#include <vector>
#include <sstream>

using namespace kvstore;

// Utility: generate keys
std::vector<std::string> generate_keys(size_t count) {
    std::vector<std::string> keys;
    keys.reserve(count);
    for (size_t i = 0; i < count; ++i) {
        keys.push_back("key_" + std::to_string(i));
    }
    return keys;
}

static void BM_Insert(benchmark::State& state) {
    const size_t N = state.range(0);
    auto keys = generate_keys(N);

    for (auto _ : state) {
        state.PauseTiming();
        KVStore store(N);
        state.ResumeTiming();

        for (size_t i = 0; i < N; ++i) {
            store.put(keys[i], "val");
        }
    }
}

static void BM_Get_Hit(benchmark::State& state) {
    const size_t N = state.range(0);
    auto keys = generate_keys(N);
    KVStore store(N);
    for (size_t i = 0; i < N; ++i) {
        store.put(keys[i], "val");
    }

    for (auto _ : state) {
        for (size_t i = 0; i < N; ++i) {
            auto val = store.get(keys[i]);
            benchmark::DoNotOptimize(val);
        }
    }
}

static void BM_Get_Miss(benchmark::State& state) {
    const size_t N = state.range(0);
    auto keys = generate_keys(N);
    KVStore store(N);

    for (auto _ : state) {
        for (size_t i = 0; i < N; ++i) {
            auto val = store.get(keys[i]);
            benchmark::DoNotOptimize(val);
        }
    }
}

BENCHMARK(BM_Insert)->Arg(1000000);
BENCHMARK(BM_Get_Hit)->Arg(1000000);
BENCHMARK(BM_Get_Miss)->Arg(1000000);

BENCHMARK_MAIN();
