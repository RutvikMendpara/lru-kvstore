#include "lru-kvstore/kv_store.hpp"

#include <cstring>
#include <string>
#include <algorithm>


namespace kvstore{


    KVStore::~KVStore()
    {
    }


    size_t KVStore::fnv1a(std::string_view key)
    {
        constexpr size_t FNV_OFFSET = 14695981039346656037ull;
        constexpr size_t FNV_PRIME  = 1099511628211ull;

        size_t hash = FNV_OFFSET;
        for (char c : key) {
            hash ^= static_cast<size_t>(c);
            hash *= FNV_PRIME;
        }
        return hash;
    }

    std::pair<bool, size_t> KVStore::find( std::string_view key, size_t hash) const
    {
        size_t idx = hash % CAPACITY;
        size_t start = idx;
        std::optional<size_t> first_deleted;

        while (true) {
            const auto& bucket = table[idx];

            if (bucket.state == BucketState::Empty)
                return {false, first_deleted.value_or(idx)}; // Free slot - key not found

            if (bucket.state == BucketState::Deleted) {
                if (!first_deleted) first_deleted = idx;
            }
            else if (bucket.hash == hash &&
                std::strncmp(bucket.node->key, key.data(), sizeof(bucket.node->key)) == 0 &&
                key.size() == std::strlen(bucket.node->key))
                {
                    return {true, idx};
                }

            idx = (idx + 1) % CAPACITY;
            if (idx == start)
                return {false, CAPACITY}; // Full loop - table is full
        }
    }


    std::optional<std::string_view> KVStore::get(std::string_view key) {
        size_t hash = fnv1a(key);
        auto [found, idx] = find(key, hash);
        if (!found) return std::nullopt;

        Node* node = table[idx].node;
        moveToFront(node);
        return std::string_view{node->value};
    }




    void KVStore::put(std::string_view key, std::string_view value) {
        if (key.size() >= sizeof(Node::key) || value.size() >= sizeof(Node::value)) {
            return;
        }

        size_t hash = fnv1a(key);
        auto [found, idx] = find(key, hash);

        if (found) {
            Node* node = table[idx].node;
            size_t val_len = std::min(value.size(), sizeof(node->value) - 1);
            memcpy(node->value, value.data(), val_len);
            node->value[val_len] = '\0';
            moveToFront(node);
            return;
        }

        if (current_size >= CAPACITY) {
            evict();
            std::tie(found, idx) = find(key, hash);
        }

        Node* node = allocate_node();
        if (!node) {
            return;
        }

        size_t key_len = std::min(key.size(), sizeof(node->key) - 1);
        memcpy(node->key, key.data(), key_len);
        node->key[key_len] = '\0';

        size_t val_len = std::min(value.size(), sizeof(node->value) - 1);
        memcpy(node->value, value.data(), val_len);
        node->value[val_len] = '\0';

        insertToFront(node);

        auto& bucket = table[idx];
        bucket.hash = hash;
        bucket.node = node;
        bucket.state = BucketState::Occupied;

        ++current_size;
    }


    void KVStore::insertToFront(Node* node)
    {
        node->prev = nullptr;
        node->next = head;

        if (head)
            head->prev = node;
        else
            tail = node; // List was empty

        head = node;
    }



    void KVStore::unlink(Node* node)
    {
        if (node->prev)
            node->prev->next = node->next;
        else
            head = node->next;

        if (node->next)
            node->next->prev = node->prev;
        else
            tail = node->prev;

        node->prev = nullptr;
        node->next = nullptr;
    }

    void KVStore::moveToFront(Node* node)
    {
        if (node == head)
            return;
        unlink(node);
        insertToFront(node);
    }

    void KVStore::evict() {
        if (!tail)
            return;

        Node* node = tail;
        unlink(node);

        size_t hash = fnv1a(node->key);
        size_t idx = hash % CAPACITY;

        while (true) {
            auto& bucket = table[idx];

            if (bucket.state == BucketState::Occupied &&
                bucket.hash == hash &&
                std::strcmp(bucket.node->key, node->key) == 0)
            {
                bucket.node = nullptr;
                bucket.state = BucketState::Deleted;
                break;
            }

            idx = (idx + 1) % CAPACITY;
        }

        free_node(node);
        --current_size;
    }

    bool KVStore::erase(std::string_view key) {
        size_t hash = fnv1a(key);
        auto [found, idx] = find(key, hash);
        if (!found) return false;

        Node* node = table[idx].node;
        unlink(node);

        free_node(node);

        table[idx].node = nullptr;
        table[idx].state = BucketState::Deleted;

        --current_size;
        return true;
    }


    size_t KVStore::size() const
    {
        return current_size;
    }

    KVStore::Node* KVStore::allocate_node()
    {
        for (size_t i = 0; i < CAPACITY; ++i) {
            if (!node_used[i]) {
                node_used[i] = true;
                return &node_pool[i];
            }
        }
        return nullptr;
    }

    void KVStore::free_node(Node* node)
    {
        size_t idx = node - node_pool;
        if (idx < CAPACITY) {
            node_used[idx] = false;
            *node = Node{};
        }
    }
}



