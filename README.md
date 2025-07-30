# lru-kvstore
High-performance, fixed-capacity in-memory key-value store with LRU eviction and thread-safe access. Designed for low latency, high concurrency, and predictable memory usage.

## Features

- Fixed-size open-addressed hash table (linear probing per shard)
- **Per-shard** LRU eviction using intrusive doubly-linked list
- Lock-striping with per-shard spinlocks (no global locks)
- Zero heap allocations during runtime (no `unordered_map`, `std::list`, etc.)
- O(1) expected-case `get()` / `put()` under low to moderate contention
- Fully unit-tested (GoogleTest) and benchmarked (Google Benchmark) ([see results](docs/benchmarks.md))


## Build & Run

```bash
git clone https://github.com/RutvikMendpara/lru-kvstore
cd lru-kvstore
./scripts/build.sh   # Build
./scripts/test.sh    # Run unit tests (gtest)
./scripts/bench.sh   # Run performance benchmarks (gbench)
```
## Project Structure

* `include/` – Public headers
* `src/` – Implementation (core logic, LRU list, spinlocks, etc.)
* `tests/` – Unit and concurrency tests (GoogleTest)
* `bench/` – Performance benchmarks (Google Benchmark)
* `docs/` – Design, benchmark results, concurrency details

## Documentation

* [Design Overview](docs/Design.md)
* [Benchmark Results](docs/benchmarks.md)
* [Concurrency Model](docs/concurrency.md)
* [Blog](https://www.rutvikmendpara.com/blog/viewer.html?post=blogs/LRU-key-value-store.md)

## Requirements

* CMake 3.16+
* C++20 compatible compiler (GCC, Clang)
* GoogleTest + Google Benchmark (auto-installed via build scripts)
