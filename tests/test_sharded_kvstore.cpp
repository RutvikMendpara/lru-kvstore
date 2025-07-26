#include <gtest/gtest.h>
#include <thread>
#include "lru-kvstore/kv_store.hpp"

using namespace kvstore;

TEST(ShardConcurrencyTest, ParallelShardWritesAreSafe) {
    KVStore store;

    auto writer = [&](int thread_id) {
        for (int i = 0; i < 100; ++i) {
            std::string key = "t" + std::to_string(thread_id) + "_k" + std::to_string(i);
            std::string val = "val_" + std::to_string(i);
            store.put(key, val);
        }
    };

    std::vector<std::thread> threads;
    const int num_threads = 8;
    for (int t = 0; t < num_threads; ++t) {
        threads.emplace_back(writer, t);
    }

    for (auto& th : threads) th.join();

    for (int t = 0; t < num_threads; ++t) {
        for (int i = 0; i < 100; ++i) {
            std::string key = "t" + std::to_string(t) + "_k" + std::to_string(i);
            auto val = store.get(key);
            ASSERT_TRUE(val.has_value());
            EXPECT_EQ(*val, "val_" + std::to_string(i));
        }
    }
}

TEST(KVStoreEvictionTest, StoreHandlesOverloadGracefully) {
    KVStore store;
    const int over_capacity = 2000;

    for (int i = 0; i < over_capacity; ++i) {
        store.put("key_" + std::to_string(i), "value_" + std::to_string(i));
    }

    int found = 0;
    for (int i = 0; i < over_capacity; ++i) {
        if (store.get("key_" + std::to_string(i)).has_value()) {
            ++found;
        }
    }

    EXPECT_LT(found, over_capacity);
    EXPECT_GT(found, 0);
}

TEST(KVStoreStressTest, ReadersWorkDuringHeavyWrites) {
    KVStore store;

    std::atomic<bool> done = false;
    std::thread writer([&]() {
        int i = 0;
        while (!done) {
            store.put("key_" + std::to_string(i % 512), "val_" + std::to_string(i));
            ++i;
        }
    });

    std::vector<std::thread> readers;
    for (int r = 0; r < 4; ++r) {
        readers.emplace_back([&]() {
            for (int i = 0; i < 10000; ++i) {
                auto val = store.get("key_" + std::to_string(i % 512));
                if (val.has_value()) {
                    ASSERT_GE(val->size(), 1);
                }
            }
        });
    }

    for (auto& r : readers) r.join();
    done = true;
    writer.join();
}

TEST(ShardThreadTest, ParallelWritesAcrossShards) {
    KVStore store;

    auto writer = [&](int base) {
        for (int i = 0; i < 100; ++i) {
            std::string key = "k" + std::to_string(base + i * 1000);
            store.put(key, "v" + std::to_string(i));
        }
    };

    std::vector<std::thread> threads;
    for (int t = 0; t < 8; ++t)
        threads.emplace_back(writer, t);

    for (auto& t : threads) t.join();

    int success = 0;
    for (int t = 0; t < 8; ++t) {
        for (int i = 0; i < 100; ++i) {
            std::string key = "k" + std::to_string(t + i * 1000);
            auto val = store.get(key);
            if (val.has_value()) success++;
        }
    }

    EXPECT_GT(success, 700);
}

TEST(ShardThreadTest, ParallelReadsWritesDoNotCrash) {
    KVStore store;

    for (int i = 0; i < 256; ++i) {
        store.put("key" + std::to_string(i), "init");
    }

    auto writer = [&store]() {
        for (int i = 0; i < 10000; ++i) {
            store.put("key" + std::to_string(i % 256), "val" + std::to_string(i));
        }
    };

    auto reader = [&store]() {
        for (int i = 0; i < 10000; ++i) {
            auto val = store.get("key" + std::to_string(i % 256));
        }
    };

    std::vector<std::thread> threads;
    threads.emplace_back(writer);
    for (int i = 0; i < 4; ++i)
        threads.emplace_back(reader);

    for (auto& t : threads)
        t.join();

    SUCCEED();
}
