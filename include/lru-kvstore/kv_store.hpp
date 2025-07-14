#pragma once

#include <string>
#include <vector>
#include <optional>
#include <cstdint>


namespace kvstore{

    class KVStore
    {
    public:
        explicit KVStore(size_t capacity);
        ~KVStore();

        // Disable copy/move semantics
        KVStore(const KVStore&) = delete;
        KVStore& operator=(const KVStore&) = delete;
        KVStore(KVStore&&) = delete;
        KVStore& operator=(KVStore&&) = delete;

        void put(const std::string& key, const std::string& value);
        std::optional<std::string> get(const std::string& key);
        bool remove(const std::string& key);
        size_t size() const;

    private:

        struct Node {
            std::string key;
            std::string value;
            Node* prev = nullptr;
            Node* next = nullptr;
        };

        enum class BucketState : std::uint8_t {
            Empty,
            Occupied,
            Deleted
        };

        struct Bucket {
            size_t hash = 0;
            Node* node = nullptr;
            BucketState state = BucketState::Empty;
        };

        std::vector<Bucket> table;
        Node* head = nullptr;
        Node* tail = nullptr;

        size_t capacity = 0;
        size_t current_size = 0;

        // Hashing + probing
        static size_t fnv1a(const std::string& key) ;
        std::pair<bool, size_t> find(const std::string& key, size_t hash) const;

        // LRU list ops
        void insertToFront(Node* node);
        void unlink(Node* node);
        void moveToFront(Node* node);
        void evict();
    };

}
