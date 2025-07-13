#pragma once

#include <list>
#include <string>
#include <vector>
#include <optional>

namespace kvstore{

    struct Entry
    {
        std::string key;
        std::string value;
    };

    class KVStore
    {
    public:
        KVStore(size_t cap = 1024);

        void put(const std::string& key , const std::string& value);
        std::optional<std::string> get(const std::string& key) const;
        bool remove(const std::string& key);
        size_t get_size() const;

    private:
        size_t capacity;
        size_t size = 0;

        std::vector<std::list<Entry>> buckets;
        size_t hash(const std::string& key) const;
    };

}
