#include <gtest/gtest.h>
#include "lru-kvstore/kv_store.hpp"

using namespace kvstore;

TEST(KVStoreTest, PutStoresKeyValue) {
    KVStore store(1024);
    store.put("key1", "value1");
    EXPECT_EQ(store.get("key1"), std::optional<std::string>("value1"));
}

TEST(KVStoreTest, GetReturnsCorrectValue) {
    KVStore store(1024);
    store.put("alpha", "beta");
    auto result = store.get("alpha");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result.value(), "beta");
}

TEST(KVStoreTest, RemoveDeletesKey) {
    KVStore store(1024);
    store.put("foo", "bar");
    EXPECT_TRUE(store.erase("foo"));
    EXPECT_FALSE(store.get("foo").has_value());
}

TEST(KVStoreTest, GetSizeTracksEntries) {
    KVStore store(1024);
    EXPECT_EQ(store.size(), 0);
    store.put("a", "1");
    store.put("b", "2");
    EXPECT_EQ(store.size(), 2);
    store.erase("a");
    EXPECT_EQ(store.size(), 1);
}

TEST(KVStoreTest, PutOverwritesValueForSameKey) {
    KVStore store(1024);
    store.put("key", "val1");
    store.put("key", "val2");
    EXPECT_EQ(store.get("key"), std::optional<std::string>("val2"));
}

TEST(KVStoreTest, RemoveNonExistentKeyReturnsFalse) {
    KVStore store(1024);
    EXPECT_FALSE(store.erase("ghost"));
}

TEST(KVStoreTest, GetMissingKeyReturnsNullopt) {
    KVStore store(1024);
    EXPECT_FALSE(store.get("missing").has_value());
}

TEST(KVStoreTest, EvictsLeastRecentlyUsedWhenOverCapacity) {
    KVStore store(1); // small capacity
    store.put("a", "1");
    store.put("b", "2");
    store.put("c", "3");

    EXPECT_FALSE(store.get("a").has_value()); // evicted
    EXPECT_FALSE(store.get("b").has_value()); // evicted
    EXPECT_TRUE(store.get("c").has_value());  // last inserted
}


TEST(KVStoreTest, SizeDoesNotIncreaseOnOverwrite) {
    KVStore store(1024);
    store.put("k", "v1");
    size_t s1 = store.size();
    store.put("k", "v2");
    size_t s2 = store.size();
    EXPECT_EQ(s1, s2);
}

TEST(KVStoreTest, RemoveThenGetReturnsNullopt) {
    KVStore store(1024);
    store.put("temp", "data");
    store.erase("temp");
    EXPECT_FALSE(store.get("temp").has_value());
}

TEST(KVStoreTest, RemoveSameKeyTwiceSecondFails) {
    KVStore store(1024);
    store.put("one", "uno");
    EXPECT_TRUE(store.erase("one"));
    EXPECT_FALSE(store.erase("one"));
}

TEST(KVStoreTest, EvictsLeastRecentlyUsedKey) {
    KVStore store(2);
    store.put("a", "1");
    store.put("b", "2");
    store.get("a");       // access 'a' to promote it
    store.put("c", "3");  // should evict 'b'

    EXPECT_TRUE(store.get("a").has_value()); // stays
    EXPECT_FALSE(store.get("b").has_value()); // evicted
    EXPECT_TRUE(store.get("c").has_value()); // inserted
}

TEST(KVStoreTest, StressPutEvictPattern) {
    KVStore store(10);
    for (int i = 0; i < 100; ++i) {
        store.put("key" + std::to_string(i), "val" + std::to_string(i));
    }
    EXPECT_LE(store.size(), 10);
}

TEST(KVStoreTest, GetUpdatesLRUOrder) {
    KVStore store(2);
    store.put("a", "1");
    store.put("b", "2");
    store.get("a");       // make 'a' recent
    store.put("c", "3");  // should evict 'b'
    EXPECT_TRUE(store.get("a").has_value());
    EXPECT_FALSE(store.get("b").has_value());
    EXPECT_TRUE(store.get("c").has_value());
}

TEST(KVStoreTest, ReinsertEvictedKey) {
    KVStore store(1);
    store.put("x", "1");
    store.put("y", "2"); // evicts x
    store.put("x", "3"); // reinsert x
    EXPECT_EQ(store.get("x"), std::optional<std::string>("3"));
    EXPECT_FALSE(store.get("y").has_value()); // evicted
}
