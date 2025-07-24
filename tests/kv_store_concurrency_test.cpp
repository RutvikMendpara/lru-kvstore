#include <gtest/gtest.h>
#include <thread>
#include <vector>
#include <atomic>
#include <chrono>
#include "lru-kvstore/kv_store.hpp"

using namespace kvstore;

TEST(KVStoreConcurrencyTest, ReadHeavyLoadWithWrites) {
    KVStore store;

    for (int i = 0; i < 512; ++i) {
        store.put("key" + std::to_string(i), "value" + std::to_string(i));
    }

    std::atomic<bool> stop{false};

    std::thread writer([&]() {
        int i = 0;
        while (!stop) {
            store.put("key" + std::to_string(i % 512), "newval" + std::to_string(i));
            ++i;
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    });

    std::vector<std::thread> readers;
    const int reader_threads = 4;
    for (int i = 0; i < reader_threads; ++i) {
        readers.emplace_back([&]() {
            for (int j = 0; j < 10000; ++j) {
                int key_id = j % 512;
                auto val = store.get("key" + std::to_string(key_id));
                ASSERT_TRUE(val.has_value());
                ASSERT_GE(val->size(), 1);
            }
        });
    }

    for (auto& r : readers) r.join();
    stop = true;
    writer.join();
}

TEST(KVStoreBenchmarkTest, MeasureGetLatencyUnderWrites) {
    KVStore store;
    for (int i = 0; i < 256; ++i) {
        store.put("key" + std::to_string(i), "value" + std::to_string(i));
    }

    std::atomic<bool> stop{false};
    std::thread writer([&]() {
        int i = 0;
        while (!stop) {
            store.put("key" + std::to_string(i % 256), "v" + std::to_string(i));
            ++i;
        }
    });

    auto start = std::chrono::high_resolution_clock::now();
    size_t reads = 0;
    for (int i = 0; i < 100'000; ++i) {
        auto val = store.get("key" + std::to_string(i % 256));
        if (val.has_value()) ++reads;
    }
    auto end = std::chrono::high_resolution_clock::now();
    stop = true;
    writer.join();

    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "Total Reads: " << reads << ", Duration(ms): " << duration.count() << std::endl;
}
