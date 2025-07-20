#include <random>
#include <benchmark/benchmark.h>
#include "lru-kvstore/kv_store.hpp"
#include <string>
#include <vector>
#include <sstream>

using namespace kvstore;

constexpr size_t CAPACITY = 1024;

// Utility: generate N unique keys
std::vector<std::string> generate_keys(size_t count) {
    std::vector<std::string> keys;
    keys.reserve(count);
    for (size_t i = 0; i < count; ++i) {
        keys.push_back("key_" + std::to_string(i));
    }
    return keys;
}

// Benchmark: insert CAPACITY keys (no eviction)
static void BM_Insert_NoEvict(benchmark::State& state) {
    auto keys = generate_keys(CAPACITY);
    for (auto _ : state) {
        state.PauseTiming();
        KVStore store;
        state.ResumeTiming();
        for (size_t i = 0; i < CAPACITY; ++i) {
            store.put(keys[i], "val");
        }
    }
}

// Benchmark: insert 10x CAPACITY keys (max eviction)
static void BM_Insert_WithEvict(benchmark::State& state) {
    const size_t N = CAPACITY * 10;
    auto keys = generate_keys(N);
    for (auto _ : state) {
        state.PauseTiming();
        KVStore store;
        state.ResumeTiming();
        for (size_t i = 0; i < N; ++i) {
            store.put(keys[i], "val");
        }
    }
}

// Benchmark: read only from hot set (100% hit rate)
static void BM_Get_HotHit(benchmark::State& state) {
    auto keys = generate_keys(CAPACITY);
    KVStore store;
    for (auto& key : keys) store.put(key, "val");

    for (auto _ : state) {
        for (auto& key : keys) {
            benchmark::DoNotOptimize(store.get(key));
        }
    }
}

// Benchmark: read cold keys that were never inserted (100% miss)
static void BM_Get_ColdMiss(benchmark::State& state) {
    auto keys = generate_keys(CAPACITY * 10);
    KVStore store; // Empty

    for (auto _ : state) {
        for (auto& key : keys) {
            benchmark::DoNotOptimize(store.get(key));
        }
    }
}

// Benchmark: mixed hot/cold access pattern
static void BM_Mixed_HotCold(benchmark::State& state) {
    const size_t hot_size = CAPACITY / 2;
    const size_t cold_size = CAPACITY * 10;
    auto hot_keys = generate_keys(hot_size);
    auto cold_keys = generate_keys(cold_size);

    KVStore store;
    for (auto& key : hot_keys) store.put(key, "val");

    std::mt19937 rng(42);
    std::bernoulli_distribution hot_prob(0.9);
    std::uniform_int_distribution<size_t> hot_dist(0, hot_size - 1);
    std::uniform_int_distribution<size_t> cold_dist(0, cold_size - 1);

    for (auto _ : state) {
        for (int i = 0; i < 10000; ++i) {
            if (hot_prob(rng)) {
                benchmark::DoNotOptimize(store.get(hot_keys[hot_dist(rng)]));
            } else {
                store.put(cold_keys[cold_dist(rng)], "val");
            }
        }
    }
}

BENCHMARK(BM_Insert_NoEvict)->UseRealTime();
BENCHMARK(BM_Insert_WithEvict)->UseRealTime();
BENCHMARK(BM_Get_HotHit)->UseRealTime();
BENCHMARK(BM_Get_ColdMiss)->UseRealTime();
BENCHMARK(BM_Mixed_HotCold)->UseRealTime();

BENCHMARK_MAIN();
