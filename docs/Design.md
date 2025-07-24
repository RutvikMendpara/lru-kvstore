# **LRU KV Store Design Notes**

## Architecture Diagram
![lru kvstore image](lru-kvstore.png)

* We use a fixed-size C-style array (`table[]`) with `CAPACITY` buckets (e.g., 1024).
* Each bucket stores:
    * A status flag: `Empty`, `Occupied`, or `Deleted`
    * `std::atomic<Node*>`: pointer to key-value node
* All Nodes are linked via a global doubly-linked list, which tracks usage order:
    * Head = most recently used
    * Tail = least recently used

---
### **Put (Insert / Update)**
* Compute `hash = fnv1a(key)`
* Get bucket index: `idx = hash % CAPACITY`
* If the bucket is:
    * **Empty**: insert directly
    * **Occupied**:
        * If same key: update value, move node to head
        * Else: use linear probing to find next slot
* If inserting a new key and store is full (`size == CAPACITY`):
    * **Evict tail node**
    * Clear its slot and remove from linked list
* **Concurrency note**: In concurrent mode, LRU list is not updated on reads to avoid locking.

---

### **Get (Access)**

* Compute hash + probe for key
* If found:
    * Return `std::string_view` to value

---

### **Eviction**

* Triggered only when inserting a new key and store is full
* Evict tail of LRU list (least recently used)
* Remove entry from hash table

---

### **Complexity**

| Operation | Best Case | Worst Case            |
| --------- | --------- | --------------------- |
| `put()`   | O(1)      | O(n) (linear probing) |
| `get()`   | O(1)      | O(n) (collisions)     |