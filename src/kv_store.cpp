#include "lru-kvstore/kv_store.hpp"
#include <cassert>
#include <iterator>

namespace kvstore{
    KVStore::KVStore(size_t cap)
        : capacity(cap), size(0), buckets(cap)
    {
        assert(capacity > 0 && "Capacity must be > 0");
    }


    void KVStore::put(const std::string& key, const std::string& value)
    {
        size_t idx = hash(key);
        auto& bucket = buckets[idx];

        for (auto& entry : bucket)
        {
            if (entry.key == key)
            {
                entry.value = value;
                return;
            }
        }

        bucket.emplace_back(Entry{key, value});
        ++size;
    }

    std::optional<std::string> KVStore::get(const std::string& key) const
    {
        size_t idx = hash(key);
        const auto& bucket = buckets[idx];
        for (auto& entry : bucket)
        {
            if (entry.key == key)
            {
                return entry.value;
            }
        }
        return std::nullopt;
    }

    bool KVStore::remove(const std::string& key)
    {
        size_t idx = hash(key);
        auto& bucket = buckets[idx];

       for (auto it = bucket.begin(); it != bucket.end(); ++it)
       {
           if (it->key == key)
           {
               bucket.erase(it);
               --size;
               return true;
           }
       }
        return false;
    }

    size_t KVStore::get_size() const
    {
         return size;
    }

    size_t KVStore::hash(const std::string& key) const {
        return std::hash<std::string>{}(key) % capacity;
    }

}
