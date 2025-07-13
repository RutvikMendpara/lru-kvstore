#include <gtest/gtest.h>
#include "lru-kvstore/kv_store.hpp"

using namespace kvstore;

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
    EXPECT_TRUE(store.remove("foo"));
    EXPECT_FALSE(store.get("foo").has_value());
}

TEST(KVStoreTest, GetSizeTracksEntries) {
    KVStore store;
    EXPECT_EQ(store.get_size(), 0);
    store.put("a", "1");
    store.put("b", "2");
    EXPECT_EQ(store.get_size(), 2);
    store.remove("a");
    EXPECT_EQ(store.get_size(), 1);
}

TEST(KVStoreTest, PutOverwritesValueForSameKey) {
    KVStore store;
    store.put("key", "val1");
    store.put("key", "val2");
    EXPECT_EQ(store.get("key"), std::optional<std::string>("val2"));
}

TEST(KVStoreTest, RemoveNonExistentKeyReturnsFalse) {
    KVStore store;
    EXPECT_FALSE(store.remove("ghost"));
}

TEST(KVStoreTest, GetMissingKeyReturnsNullopt) {
    KVStore store;
    EXPECT_FALSE(store.get("missing").has_value());
}

TEST(KVStoreTest, CollisionHandlingStoresAllKeys) {
    KVStore store(1); // force all keys into same bucket
    store.put("a", "1");
    store.put("b", "2");
    store.put("c", "3");

    EXPECT_EQ(store.get("a"), std::optional<std::string>("1"));
    EXPECT_EQ(store.get("b"), std::optional<std::string>("2"));
    EXPECT_EQ(store.get("c"), std::optional<std::string>("3"));
}

TEST(KVStoreTest, SizeDoesNotIncreaseOnOverwrite) {
    KVStore store;
    store.put("k", "v1");
    size_t s1 = store.get_size();
    store.put("k", "v2");
    size_t s2 = store.get_size();
    EXPECT_EQ(s1, s2);
}

TEST(KVStoreTest, RemoveThenGetReturnsNullopt) {
    KVStore store;
    store.put("temp", "data");
    store.remove("temp");
    EXPECT_FALSE(store.get("temp").has_value());
}

TEST(KVStoreTest, RemoveSameKeyTwiceSecondFails) {
    KVStore store;
    store.put("one", "uno");
    EXPECT_TRUE(store.remove("one"));
    EXPECT_FALSE(store.remove("one"));
}