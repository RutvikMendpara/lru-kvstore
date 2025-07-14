#include "lru-kvstore/kv_store.hpp"


namespace kvstore{
    KVStore::KVStore(size_t capacity)
        : table(capacity), capacity(capacity), current_size(0), head(nullptr), tail(nullptr)
    {}

    KVStore::~KVStore()
    {
        Node* curr = head;
        while (curr) {
            Node* next = curr->next;
            delete curr;
            curr = next;
        }
    }


    size_t KVStore::fnv1a(const std::string& key)
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

    std::pair<bool, size_t> KVStore::find(const std::string& key, size_t hash) const
    {
        size_t idx = hash % table.size();
        size_t start = idx;

        while (true) {
            const auto& bucket = table[idx];

            if (bucket.state == BucketState::Empty)
                return {false, idx}; // Free slot — key not found

            if (bucket.state == BucketState::Occupied &&
                bucket.hash == hash &&
                bucket.node->key == key)
                return {true, idx}; // Key found

            idx = (idx + 1) % table.size();
            if (idx == start)
                return {false, table.size()}; // Full loop — table is full
        }
    }


    std::optional<std::string> KVStore::get(const std::string& key)
    {
        size_t hash = fnv1a(key);
        auto [found, idx] = find(key, hash);
        if (!found) return std::nullopt;

        Node* node = table[idx].node;
        moveToFront(node);
        return node->value;
    }


    void KVStore::put(const std::string& key, const std::string& value)
    {
        size_t hash = fnv1a(key);
        auto [found, idx] = find(key, hash);

        if (found) {
            // Key exists — update value and promote in LRU
            Node* node = table[idx].node;
            node->value = value;
            moveToFront(node);
            return;
        }

        // Key does not exist
        if (current_size >= capacity) {
            evict();
        }

        // Insert new node
        Node* node = new Node{key, value};
        insertToFront(node);

        size_t insertIdx = hash % table.size();
        while (true) {
            auto& bucket = table[insertIdx];
            if (bucket.state != BucketState::Occupied) {
                bucket.hash = hash;
                bucket.node = node;
                bucket.state = BucketState::Occupied;
                break;
            }
            insertIdx = (insertIdx + 1) % table.size();
        }

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

    void KVStore::evict()
    {
        if (!tail)
            return;

        Node* node = tail;
        unlink(node);

        // Find and clear from hash table
        size_t hash = fnv1a(node->key);
        size_t idx = hash % table.size();

        while (true) {
            auto& bucket = table[idx];

            if (bucket.state == BucketState::Occupied &&
                bucket.hash == hash &&
                bucket.node->key == node->key) {

                bucket.node = nullptr;
                bucket.state = BucketState::Deleted;
                break;
                }

            idx = (idx + 1) % table.size();
        }

        delete node;
        --current_size;
    }

    bool KVStore::remove(const std::string& key)
    {
        size_t hash = fnv1a(key);
        auto [found, idx] = find(key, hash);
        if (!found) return false;

        Node* node = table[idx].node;
        unlink(node);
        delete node;

        table[idx].node = nullptr;
        table[idx].state = BucketState::Deleted;

        --current_size;
        return true;
    }

    size_t KVStore::size() const
    {
        return current_size;
    }





}
