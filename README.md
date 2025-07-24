# lru-kvstore
High-performance in-memory key-value store with LRU eviction and thread-safe access. Built for predictability, low-latency, and extensibility.

## Features

* Fixed-size hash table with linear probing
* LRU eviction using global doubly-linked list
* O(1) best-case `put()` and `get()`
* No dynamic memory allocations after init (no `std::unordered_map`, `std::list`, etc.)
* Thread-safe via spinlocks + atomic pointers
* Unit-tested with GoogleTest
* Benchmarks powered by Google Benchmark ([see results](docs/benchmarks.md))

## Build & Run

```bash
git clone https://github.com/RutvikMendpara/lru-kvstore
cd lru-kvstore
./scripts/build.sh   # Build
./scripts/test.sh    # Run unit tests (gtest)
./scripts/bench.sh   # Run performance benchmarks (gbench)
```

## Documentation

* [Design Overview](docs/Design.md)
* [Benchmark Results](docs/benchmarks.md)
* [Concurrency Model](docs/concurrency.md)

## Requirements

* CMake 3.16+
* C++20 or newer

