# **LRU KV Store Design Overview**

## Architecture Diagram
![lru kvstore image](lru-kvstore.png)

- The store is divided into **multiple shards** (e.g., 8).
- Each shard contains:
  - Fixed-size array of `CAPACITY` buckets (e.g., 1024)
  - Each bucket stores:
    - Status flag: `Empty`, `Occupied`, or `Deleted`
    - `std::atomic<Node*>` pointing to a key-value node
  - A **shard-local doubly-linked list** for LRU eviction:
    - Head = most recently used
    - Tail = least recently used

---

## **Put (Insert / Update)**

- Hash key with `fnv1a(key)`
- Determine shard: `shard_id = hash % NUM_SHARDS`
- Inside the shard:
  - Index into hash table: `idx = hash % SHARD_CAPACITY`
  - If bucket is:
    - **Empty**: insert new node
    - **Occupied**:
      - If same key: update value, move to LRU head
      - Else: linear probe for next slot
- If inserting a new key and shard is full:
  - **Evict** tail of LRU list
- **Thread-safety**: Writer uses spinlock per shard

---

## **Get (Access)**

- Hash key and determine shard
- Probe for key in shard’s table
- If found:
  - Return `std::string_view` to value
  - LRU **not** updated (lock-free read path)

---

## **Eviction**

- Happens *within shard* when it’s full
- Evict tail node from LRU list
- Remove corresponding hash table entry
- No cross-shard eviction coordination

---

## **Complexity (Per Shard)**

| Operation | Best Case | Worst Case                       |
|-----------|-----------|----------------------------------|
| `put()`   | O(1)      | O(n) (probing on hash collision) |
| `get()`   | O(1)      | O(n) (same as above)             |

---

## Notes

- Sharding improves concurrency: multiple writers can write simultaneously to different shards.
- LRU is **per-shard**, not global. Hot data may be duplicated across shards.
- No dynamic memory allocation during operation (all nodes pre-allocated at init).
