# Concurrency Model

## Overview

This KVStore supports concurrent access for:
- Multiple `get()` readers (lock-free via atomic pointers)
- Single `put()` writer (protected by spinlock)

## Design

- Each bucket uses `std::atomic<Node*>` for safe reads.
- Global LRU list is modified only by writer thread.
- Writer uses `SpinLock` to ensure mutual exclusion on insert/evict.
- Reader threads do not update LRU order to avoids contention.

## Guarantees

- `get()` is **wait-free** (no locks, no modification).
- `put()` is **safe**, but may evict entries under pressure.
- No data corruption observed under stress benchmarks.

## Limitations

- Only one writer supported at a time.
- LRU order may be stale under high concurrency.
