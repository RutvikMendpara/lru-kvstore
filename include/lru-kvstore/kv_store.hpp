#pragma once

#include <cstdint>
#include <optional>
#include <string_view>
#include <vector>


namespace kvstore{

    class KVStore
    {
    public:
        KVStore() = default;
        ~KVStore();

        KVStore(const KVStore&) = delete;
        KVStore& operator=(const KVStore&) = delete;
        KVStore(KVStore&&) = delete;
        KVStore& operator=(KVStore&&) = delete;

        void put( std::string_view key,  std::string_view value);
         std::optional<std::string_view> get( std::string_view key);
        bool erase(std::string_view key);
        size_t size() const;

    private:

        struct Node {
            char key[32];
            char value[64];
            size_t key_len = 0;
            Node* prev = nullptr;
            Node* next = nullptr;
            size_t hash = 0;
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

        static constexpr size_t CAPACITY = 1024;
        Bucket table[CAPACITY];
        Node node_pool[CAPACITY];
        bool node_used[CAPACITY] = {};

        Node* head = nullptr;
        Node* tail = nullptr;

        size_t current_size = 0;

        static size_t fnv1a( std::string_view key) ;
        std::pair<bool, size_t> find( std::string_view key, size_t hash) const;

        void insertToFront(Node* node);
        void unlink(Node* node);
        void moveToFront(Node* node);
        void evict();

        Node* allocate_node();
        void free_node(Node* node);
    };

}
