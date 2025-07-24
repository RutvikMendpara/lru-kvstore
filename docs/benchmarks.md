# Benchmark Results (10 Runs)

**System**: *AMD Ryzen 5 3550H - 16GB RAM - Fedora Linux 42 Kernel - Linux 6.15.5-200*
**Build Flags**: `-O3 -march=native -flto -DNDEBUG`
**CAPACITY**: `1024`
**Benchmark source**: `Google Benchmark`

---

## Summary Table (Real Time, ns)

| Benchmark                           | Mean Time (ns) | Description                                    | Iterations |
| ----------------------------------- |----------------| ---------------------------------------------- |------------|
| `BM_Insert_NoEvict`                 | 129733         | Insert N keys where N ≤ CAPACITY, no eviction. | 5310       |
| `BM_Insert_WithEvict`               | 43546943       | Insert 10× CAPACITY, triggers max eviction.    | 16         |
| `BM_Get_HotHit`                     | 73892          | 100% cache hit, read from hot set.             | 9399       | 
| `BM_Get_ColdMiss`                   | 154292         | 100% miss, read from never-inserted keys.      | 4490       | 
| `BM_Mixed_HotCold`                  | 19807657       | 90% hot reads, 10% cold writes.                | 37         | 
| `BM_Get_ParallelReaders/threads:8`  | 92.6           | 8 threads hammering `get()` concurrently.      | 7103224    | 
| `BM_Concurrent_ReadWrite/threads:5` | 953            | 4 readers + 1 writer, simulates live load.     | 732270     | 

---

## Notes

* `get()` performance remains **stable under concurrent load**.
* Eviction introduces predictable overhead — no major latency spikes.
* No data corruption observed under multithreaded read/write stress.
