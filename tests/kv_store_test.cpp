#include <gtest/gtest.h>
#include "lru-kvstore/kv_store.hpp"

using namespace kvstore;


constexpr size_t CAPACITY = 1024;

std::vector<std::string> generate_keys(size_t count) {
    std::vector<std::string> keys;
    keys.reserve(count);
    for (size_t i = 0; i < count; ++i) {
        keys.push_back("key_" + std::to_string(i));
    }
    return keys;
}

TEST(KVStoreTest, PutStoresKeyValue) {
    KVStore store;
    store.put("key1", "value1");
    EXPECT_EQ(store.get("key1"), std::optional<std::string>("value1"));
}

TEST(KVStoreTest, GetReturnsCorrectValue) {
    KVStore store;
    store.put("alpha", "beta");
    auto result = store.get("alpha");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result.value(), "beta");
}

TEST(KVStoreTest, RemoveDeletesKey) {
    KVStore store;
    store.put("foo", "bar");
    EXPECT_TRUE(store.erase("foo"));
    EXPECT_FALSE(store.get("foo").has_value());
}

TEST(KVStoreTest, GetSizeTracksEntries) {
    KVStore store;
    EXPECT_EQ(store.size(), 0);
    store.put("a", "1");
    store.put("b", "2");
    EXPECT_EQ(store.size(), 2);
    store.erase("a");
    EXPECT_EQ(store.size(), 1);
}

TEST(KVStoreTest, PutOverwritesValueForSameKey) {
    KVStore store;
    store.put("key", "val1");
    store.put("key", "val2");
    EXPECT_EQ(store.get("key"), std::optional<std::string>("val2"));
}

TEST(KVStoreTest, RemoveNonExistentKeyReturnsFalse) {
    KVStore store;
    EXPECT_FALSE(store.erase("ghost"));
}

TEST(KVStoreTest, GetMissingKeyReturnsNullopt) {
    KVStore store;
    EXPECT_FALSE(store.get("missing").has_value());
}


TEST(KVStoreTest, SizeDoesNotIncreaseOnOverwrite) {
    KVStore store;
    store.put("k", "v1");
    size_t s1 = store.size();
    store.put("k", "v2");
    size_t s2 = store.size();
    EXPECT_EQ(s1, s2);
}

TEST(KVStoreTest, RemoveThenGetReturnsNullopt) {
    KVStore store;
    store.put("temp", "data");
    store.erase("temp");
    EXPECT_FALSE(store.get("temp").has_value());
}

TEST(KVStoreTest, RemoveSameKeyTwiceSecondFails) {
    KVStore store;
    store.put("one", "uno");
    EXPECT_TRUE(store.erase("one"));
    EXPECT_FALSE(store.erase("one"));
}

TEST(KVStoreTest, InsertAfterDeleteReusesSlot) {
    KVStore store;

    store.put("one", "1");
    EXPECT_TRUE(store.erase("one"));
    EXPECT_FALSE(store.get("one").has_value());

    store.put("one", "2");
    EXPECT_EQ(store.get("one"), std::optional<std::string>("2"));
}

TEST(KVStoreTest, LinearProbingWorksAfterDelete) {
    KVStore store;

    store.put("key1", "val1");
    store.put("key2", "val2");
    EXPECT_TRUE(store.erase("key1"));

    store.put("key3", "val3");

    EXPECT_EQ(store.get("key2"), std::optional<std::string>("val2"));
    EXPECT_EQ(store.get("key3"), std::optional<std::string>("val3"));
}

TEST(KVStoreTest, EvictionDoesNotCorruptTable) {
    KVStore store;
    auto keys = generate_keys(CAPACITY * 5);

    for (size_t i = 0; i < keys.size(); ++i) {
        store.put(keys[i], "val");
    }

    SUCCEED();
}
TEST(KVStoreTest, ReinsertSameKeyAfterEviction) {
    KVStore store;
    std::string key = "hotkey";

    for (int i = 0; i < CAPACITY * 5; ++i) {
        store.put("key" + std::to_string(i), "val");
        store.put(key, "val");  // keep reinserting
    }

    auto val = store.get(key);
    EXPECT_TRUE(val.has_value());
}
