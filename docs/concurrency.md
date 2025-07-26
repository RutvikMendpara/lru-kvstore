# Concurrency Model

## Overview

This KVStore supports concurrent access for:
- Multiple `get()` readers (lock-free)
- Multiple `put()` writers (via sharding)

## Design

- Store is sharded: each shard has its own hash table, spinlock, and LRU list.
- Each bucket uses `std::atomic<Node*>` for safe, lock-free reads.
- Writers acquire per-shard `SpinLock` to insert/evict safely.
- LRU list is shard-local and only updated by the writer.
- Readers do not modify LRU to avoid contention.

## Guarantees

- `get()` is **wait-free** (no locks, no modification).
- `put()` is **safe**, but may evict entries under pressure.
- No data corruption observed under stress benchmarks.

## Limitations

- Writes to the *same shard* are serialized
- LRU ordering is **per-shard**, not global
- No cross-shard consistency or atomicity

## Summary

| Operation | Concurrency            | Locking              |
|-----------|------------------------|----------------------|
| `get()`   | Multi-reader           | Lock-free            |
| `put()`   | Multi-writer (sharded) | SpinLock per shard   |
| LRU List  | Shard-local            | Serialized per shard |

