#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <vector>


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

        void put( std::string_view key,  std::string_view value);
        std::optional<std::string> get( std::string_view key);
        bool erase(std::string_view key);
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
        static size_t fnv1a( std::string_view key) ;
        std::pair<bool, size_t> find( std::string_view key, size_t hash) const;

        // LRU list ops
        void insertToFront(Node* node);
        void unlink(Node* node);
        void moveToFront(Node* node);
        void evict();
    };

}
