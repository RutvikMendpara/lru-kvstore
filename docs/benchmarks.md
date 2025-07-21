# Benchmark Results (10 Runs)

**System**: *AMD Ryzen 5 3550H - 16GB RAM - Fedora Linux 42 Kernel - Linux 6.15.5-200*
**Build Flags**: `-O3 -march=native -flto -DNDEBUG`
**CAPACITY**: `1024`
**Benchmark source**: `Google Benchmark`

---

## Summary Table (Real Time, ns)

| Benchmark             | Run 1    | Run 2    | Run 3    | Run 4    | Run 5    | Run 6    | Run 7    | Run 8    | Run 9    | Run 10   |
| --------------------- | -------- | -------- | -------- | -------- | -------- | -------- | -------- | -------- | -------- | -------- |
| `BM_Insert_NoEvict`   | 105514   | 105274   | 104890   | 104784   | 105366   | 104861   | 105143   | 104855   | 104523   | 105482   |
| `BM_Insert_WithEvict` | 36080575 | 35571743 | 35336001 | 36920138 | 35596662 | 36395030 | 35755403 | 36232086 | 36525949 | 36464424 |
| `BM_Get_HotHit`       | 53975    | 53725    | 53830    | 53826    | 53970    | 53860    | 54035    | 53699    | 54736    | 59000    |
| `BM_Get_ColdMiss`     | 126846   | 126501   | 126536   | 127427   | 126364   | 126410   | 127503   | 126299   | 142332   | 159062   |
| `BM_Mixed_HotCold`    | 5118922  | 5096251  | 5098609  | 5096247  | 5096967  | 5110853  | 5002528  | 5103241  | 5227938  | 5101690  |

---

### Iteration Counts per Benchmark (Typical)
- BM_Insert_NoEvict: ~6,200–6,600
- BM_Insert_WithEvict: 18–20
- BM_Get_HotHit: ~11,700–12,700
- BM_Get_ColdMiss: ~4,300–5,500
- BM_Mixed_HotCold: ~100–140
