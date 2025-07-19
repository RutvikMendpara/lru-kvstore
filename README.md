# lru-kvstore
High-performance in-memory key-value store with LRU eviction, built for cache efficiency and extensibility.

## Features

* Fixed-size hash table with linear probing
* LRU eviction using global doubly-linked list
* O(1) best-case insert and lookup
* No dynamic STL containers and manual node management
* Unit-tested with GoogleTest
* Benchmarks powered by Google Benchmark

## Build & Run

```bash
git clone https://github.com/RutvikMendpara/lru-kvstore
cd lru-kvstore
./scripts/build.sh   # Build
./scripts/test.sh    # Run tests
./scripts/bench.sh   # Run benchmarks
```

## Design Overview

See [`design.md`](docs/Design.md) for detailed internals.

