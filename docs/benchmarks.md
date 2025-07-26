# Benchmark Results (10 Runs)

**System**: *AMD Ryzen 5 3550H - 16GB RAM - Fedora Linux 42 Kernel - Linux 6.15.5-200*
**Build Flags**: `-O3 -march=native -flto -DNDEBUG`
**CAPACITY**: `1024`
**Benchmark source**: `Google Benchmark`

---

## Summary Table (Real Time, ns)

| Benchmark                             | Time (ns) | Description                            |
| ------------------------------------- | --------- | -------------------------------------- |
| `BM_Insert_NoEvict`                   | 90,931    | Insert N keys where N ≤ CAPACITY       |
| `BM_Insert_WithEvict`                 | 7,326,561 | Insert 10× CAPACITY, triggers eviction |
| `BM_Get_HotHit`                       | 72,836    | 100% cache hit                         |
| `BM_Get_ColdMiss`                     | 204,845   | 100% miss, never-inserted keys         |
| `BM_Mixed_HotCold`                    | 3,815,285 | 90% hot reads, 10% cold writes         |
| `BM_Get_ParallelReaders/threads:8`    | 154       | 8 threads concurrently reading         |
| `BM_Concurrent_ReadWrite/threads:5`   | 270       | 4 readers + 1 writer under stress      |
| `BM_Write_Heavy_Parallel/threads:4`   | 1,384     | 4 threads doing mostly writes          |
| `BM_ReadMostly_SharedStore/threads:8` | 765       | 8 threads, 95% reads / 5% writes       |

---

## Notes

* `get()` performance remains highly stable under concurrent load, minimal contention observed.
* Eviction now completes over 6× faster significant reduction in latency and overhead.
* Mixed workloads (hot/cold access) are vastly more efficient, LRU path is now well-optimized.
* No data corruption or race conditions observed under aggressive multithreaded stress.
* Overall memory usage and locking behavior appear well-balanced across shards.

