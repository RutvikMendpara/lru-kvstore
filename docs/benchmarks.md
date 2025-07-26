# Benchmark Results (10 Runs)

**System**: *AMD Ryzen 5 3550H - 16GB RAM - Fedora Linux 42 Kernel - Linux 6.15.5-200*
**Build Flags**: `-O3 -march=native -flto -DNDEBUG`
**CAPACITY**: `1024`
**Benchmark source**: `Google Benchmark`

---

## Summary Table (Real Time, ns)

| Benchmark                           | Old Time (ns) | New Time (ns) | delta (%) | Description                            |
| ----------------------------------- | ------------- | ------------- |-----------| -------------------------------------- |
| `BM_Insert_NoEvict`                 | 129,733       | **78,789**    | **−39%**  | Insert N keys where N ≤ CAPACITY       |
| `BM_Insert_WithEvict`               | 43,546,943    | **6,374,137** | **−85%**  | Insert 10× CAPACITY, triggers eviction |
| `BM_Get_HotHit`                     | 73,892        | **61,645**    | **−17%**  | 100% cache hit                         |
| `BM_Get_ColdMiss`                   | 154,292       | **176,832**   | **+14%**  | 100% miss, never-inserted keys         |
| `BM_Mixed_HotCold`                  | 19,807,657    | **3,164,775** | **−84%**  | 90% hot reads, 10% cold writes         |
| `BM_Get_ParallelReaders/threads:8`  | 92.6          | **87.9**      | **−5%**   | 8 threads concurrently reading         |
| `BM_Concurrent_ReadWrite/threads:5` | 953           | **217**       | **−77%**  | 4 readers + 1 writer under stress      |

---

## Notes

* `get()` performance remains highly stable under concurrent load, minimal contention observed.
* Eviction now completes over 6× faster significant reduction in latency and overhead.
* Mixed workloads (hot/cold access) are vastly more efficient, LRU path is now well-optimized.
* No data corruption or race conditions observed under aggressive multithreaded stress.
* Overall memory usage and locking behavior appear well-balanced across shards.

