 #include "lru-kvstore/kv_store.hpp"

#include <cstring>
#include <string>
#include <algorithm>
#include <cassert>


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

    std::pair<bool, size_t> Shard::find(std::string_view key, size_t hash) const {
        size_t idx = hash % LOCAL_CAPACITY;
        size_t start = idx;
        std::optional<size_t> first_deleted;

        while (true) {
            const auto& bucket = table[idx];

            if (bucket.state == BucketState::Empty)
                return {false, first_deleted.value_or(idx)};

            if (bucket.state == BucketState::Deleted) {
                if (!first_deleted)
                    first_deleted = idx;
            } else {
                Node* node = bucket.node.load(std::memory_order_acquire);
                if (     node &&
                           bucket.hash == hash &&
                           node->key_len == key.size() &&
                           std::memcmp(node->key, key.data(), key.size()) == 0) {
                    return {true, idx};
                           }
            }



            idx = (idx + 1) % LOCAL_CAPACITY;
            if (idx == start)
               break;
        }
        return {false, first_deleted.value_or(LOCAL_CAPACITY)};
    }



    std::optional<std::string_view> KVStore::get(std::string_view key) {
        size_t hash = fnv1a(key);
        size_t shard_idx = hash % NUM_SHARDS;
        Shard& shard = shards[shard_idx];

        std::lock_guard<SpinLock> guard(shard.lock);

        auto [found, idx] = shard.find(key, hash);
        if (!found) return std::nullopt;

        Shard::Node* node = shard.table[idx].node.load(std::memory_order_acquire);
        if (!node) return std::nullopt;

        if (node->key_len != key.size()) return std::nullopt;
        if (memcmp(node->key, key.data(), key.size()) != 0) return std::nullopt;

        return std::string_view{node->value};
    }

    void KVStore::put(std::string_view key, std::string_view value) {
        size_t hash = fnv1a(key);
        size_t shard_idx = hash % NUM_SHARDS;
        Shard& shard = shards[shard_idx];

        // if (key.size() >= sizeof(Shard::Node::key) || value.size() >= sizeof(Shard::Node::value)) {
        //     return;
        // }

        std::lock_guard<SpinLock> guard(shard.lock);

        auto [found, idx] = shard.find(key, hash);

        if (found) {
            auto* node = shard.table[idx].node.load();
            size_t val_len = std::min(value.size(), sizeof(node->value) - 1);
            memcpy(node->value, value.data(), val_len);
            node->value[val_len] = '\0';
            node->hash = hash;
            shard.moveToFront(node);
            return;
        }

        if (shard.current_size >= LOCAL_CAPACITY) {
            shard.evict();
            std::tie(found, idx) = shard.find(key, hash);
            // if (idx >= LOCAL_CAPACITY) {
            //     std::abort();
            // }
        }

        Shard::Node *node = shard.allocate_node();
        if (!node) {
            return;
        }

        size_t key_len = std::min(key.size(), sizeof(node->key) - 1);
        memcpy(node->key, key.data(), key_len);
        node->key[key_len] = '\0';
        node->key_len = key_len;
        node->hash = hash;

        size_t val_len = std::min(value.size(), sizeof(node->value) - 1);
        memcpy(node->value, value.data(), val_len);
        node->value[val_len] = '\0';

        shard.insertToFront(node);

        auto &bucket = shard.table[idx];
        bucket.hash = hash;
        bucket.node.store(node);
        bucket.state = Shard::BucketState::Occupied;

        ++shard.current_size;
    }


    void Shard::insertToFront(Node* node)
    {
        node->prev = nullptr;
        node->next = head;

        if (head)
            head->prev = node;
        else
            tail = node;

        head = node;
    }



    void Shard::unlink(Node* node)
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

    void Shard::moveToFront(Node* node)
    {
        if (node == head)
            return;
        unlink(node);
        insertToFront(node);
    }


    void Shard::evict() {
        if (!tail)
            return;

        Node* node = tail;

        size_t hash = node->hash;
        size_t idx = hash % LOCAL_CAPACITY;
        size_t start = idx;

        bool removed = false;

        while (true) {
            auto& bucket = table[idx];
            Node* current = bucket.node.load(std::memory_order_acquire);

            if (current &&
                bucket.hash == hash &&
                current->key_len == node->key_len &&
                std::memcmp(current->key, node->key, node->key_len) == 0) {

                bucket.node.store(nullptr, std::memory_order_release);
                bucket.state = BucketState::Deleted;
                removed = true;
                break;
                }

            idx = (idx + 1) % LOCAL_CAPACITY;
            if (idx == start)
                break;
        }

        if (!removed) {
            std::abort();
        }

        unlink(node);
        free_node(node);
        --current_size;
    }

    bool KVStore::erase(std::string_view key) {
        size_t hash = fnv1a(key);
        Shard& shard = shards[hash % NUM_SHARDS];
        return shard.erase(key, hash);
    }


    bool Shard::erase(std::string_view key, size_t hash) {
        lock.lock();
        auto [found, idx] = find(key, hash);
        if (!found) {
            lock.unlock();
            return false;
        }


        Node* node = table[idx].node.load(std::memory_order_acquire);

        if (!node) {
            lock.unlock();
            return false;
        }

        unlink(node);
        free_node(node);

        table[idx].node.store(nullptr, std::memory_order_release);
        table[idx].state = BucketState::Deleted;

        --current_size;

        lock.unlock();
        return true;
    }


    size_t KVStore::size() const {
        size_t total = 0;
        for (const auto& shard : shards) {
            std::lock_guard<SpinLock> guard(shard.lock);
            total += shard.current_size;
        }
        return total;
    }




    Shard::Node* Shard::allocate_node()
    {
        for (size_t i = 0; i < LOCAL_CAPACITY; ++i) {
            if (!node_used[i]) {
                node_used[i] = true;
                return &node_pool[i];
            }
        }
        return nullptr;
    }

    void Shard::free_node(Node* node)
    {
        size_t idx = node - node_pool;
        if (idx < LOCAL_CAPACITY) {
            node_used[idx] = false;
            *node = Node{};
            node->prev = nullptr;
            node->next = nullptr;
            node->key_len = 0;
            node->hash = 0;

        }
    }
}



