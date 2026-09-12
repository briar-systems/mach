# R2: final memory and time curves with thresholds (#2299, #3221)

The measurement remainder of #2299 and the bounds evidence of #3221, taken on
dev at `83d3c1c9d` with `test/memory.py` after the harness was extended for
it. Every number below has its compiler, tree, std, host, profile, worker
count, cache mode, repetition count with spread, and host load beside it.
Nothing here inspects or reproduces the consumer project of the original
18 GB report.

## Provenance

| item | value |
|---|---|
| compiler under test | dev `83d3c1c9d`, std `e6fc41251` (2.0.0), 280 modules loaded by its own build (compiler 248 files, 315,368 lines; std 164 files, 94,734 lines) |
| debug compiler (`out/audit/mach`) | 13,739,288 bytes, sha256 `eef2db14…`, profile `debug` (opt 0), built by the v5 stage compiler at `2a2918b23`, carries a build id |
| release compiler (`out/rel/mach`) | 17,659,160 bytes, profile `release` (opt 2), same builder |
| control | the published 4.30.0 seed, `~/.local/bin/mach`, 15,223,480 bytes, sha256 `29b9264c…`, release profile; it cannot build dev, so its self-build workload is the 4.30.0 tree `b65afb970` with std `168a9f760` (1.0.1), 260 modules (compiler 227 files, 269,904 lines; std 164 files, 85,404 lines), checked out detached |
| host | AMD Ryzen 7 5800X3D, 16 CPUs, 31.2 GiB, Linux 7.2.0-1-cachyos x86_64, transparent huge pages `always`, 27 GiB of swap in use by other lanes throughout |
| harness | `test/memory.py` at this PR, one compiler process at a time, three repetitions per cell, medians with [min, max] |
| load | one-minute load average read at the start of every process, from other lanes building and testing compilers: run A 1.85 to 8.98 (median 3.86, 279 of 624 processes saw another compiler running), run B 2.9 to 7.6, run A2 1.16 to 11.07 (median 7.52, 667 of 768) |
| runs | 2026-09-12: A (debug compiler and the seed, self-build and synthetic, 624 processes), B (release compiler, self-build, 36), A2 (debug compiler and the seed, synthetic with the blocks family, 768), four attribution probes (16), the publish loop (24 builds), the mutation anchors and the suite |

Peak is the kernel `ru_maxrss` of the compiler process as seen by a minimal
spawner whose own floor is 9.2 to 9.4 MiB (recorded per run); cells at that
floor are reported as such. Wall is the spawner's clock around fork and
wait (run A's synthetic walls, superseded by A2, were this process's clock
and quantized to 50 ms by the wait polling).

### Two apparatus findings, fixed in the harness before any number was kept

**Transparent huge pages.** With THP `always`, an anonymous mapping over
2 MiB is faulted in as a huge page when the kernel finds a contiguous block,
so RSS counts memory the process never touched, and whether it happens
depends on the host's fragmentation at that moment. The first run measured
the same serial debug self-build at 1542, 1876, 1862, 1896, 1818 and 1992 MiB.
With `PR_SET_THP_DISABLE` set in the child before `exec` it measured
1865.0, 1865.5, 1865.5 MiB (three consecutive builds), and with THP left on
in a later quiet window 1997.8, 1997.0, 1996.8. The harness now disables THP
for the child, so the peak is touched 4 KiB pages, the number the compiler
is responsible for; a THP `always` host will see up to about 7 percent more.

**Swap-out under host pressure.** Other lanes on this host kept 24 to 27 GiB
of swap in use; when they load the machine the kernel reclaims pages from
the process being measured, and `ru_maxrss` then under-reports its peak
(observed: 1478 MiB `ru_maxrss` on a build whose true peak is 1857). The
sampler now reads `VmSwap` beside `VmRSS` at 20 ms; a cell whose `VmSwap`
was ever nonzero is marked `swapped` and its headline peak is the larger of
`ru_maxrss` and the sampled maximum of `VmRSS+VmSwap`. Swapped cells are
marked in the tables; their corrected peaks agree with unswapped repetitions
of the same cell within 1 percent (serial) and 3 percent (16 workers, where
the sampler cannot catch the exact instant).

## Self-build

The compiler's own tree, one build cell (`linux-x86_64`), `off` is an
uncached build, `cold` removes the object store and publishes, `warm`
restores every module (280 of 280 `cached object` lines required by the
harness, or the run fails).

### Run A: debug compiler on dev, seed on the 4.30.0 tree

Peak MiB [min, max] / wall s [min, max], three repetitions each, `load` is
the highest one-minute load average seen at the start of a repetition.

| profile | jobs | mode | dev `83d3c1c9d` by the debug compiler | load | 4.30.0 seed on `b65afb970` | load |
|---|---|---|---|---|---|---|
| debug | 1 | off | 1857.2 [1856.8, 1861.1] / 42.53 [41.12, 44.33] (swapped 3) | 8.59 | 1075.4 [1071.8, 1076.8] / 28.88 [26.92, 29.78] (swapped 2) | 5.93 |
| debug | 1 | cold | 1857.5 [1856.0, 1862.6] / 43.54 [42.32, 48.55] (swapped 3) | 6.79 | | |
| debug | 1 | warm | 1071.1 [1071.0, 1071.1] / 11.15 [9.68, 13.74] | 6.43 | | |
| debug | 16 | off | 2263.0 [2256.8, 2266.3] / 20.80 [20.59, 24.32] (swapped 1) | 7.09 | 1318.3 [1317.7, 1339.3] / 12.14 [11.84, 13.01] | 7.16 |
| debug | 16 | cold | 2243.7 [2194.2, 2257.5] / 22.05 [21.85, 22.50] (swapped 1) | 7.85 | | |
| debug | 16 | warm | 1071.2 [1071.2, 1071.2] / 9.38 [9.32, 11.49] | 6.61 | | |
| release | 1 | off | 2285.7 [2285.2, 2285.8] / 82.85 [82.79, 87.40] (swapped 3) | 6.37 | 1938.2 [1938.1, 1942.0] / 102.63 [100.86, 109.30] (swapped 3) | 8.48 |
| release | 1 | cold | 2285.0 [2285.0, 2288.0] / 85.46 [83.70, 92.94] (swapped 3) | 5.13 | | |
| release | 1 | warm | 1086.3 [1086.3, 1086.3] / 11.09 [9.93, 11.54] | 7.97 | | |
| release | 16 | off | 2858.4 [2835.9, 2879.6] / 28.93 [26.91, 29.03] (swapped 1) | 8.68 | 2614.6 [2588.8, 2703.5] / 28.79 [27.01, 29.68] (swapped 1) | 8.78 |
| release | 16 | cold | 2863.6 [2807.7, 2880.1] / 28.76 [27.86, 30.28] (swapped 1) | 8.98 | | |
| release | 16 | warm | 1086.4 [1086.4, 1086.5] / 9.88 [9.73, 10.08] | 7.98 | | |

The dev debug build produced 560 objects (20.9 MB) and a 13.7 MB image; the
seed's build of its tree 520 objects (43.0 MB) and an 11.0 MB image. Every dev cell
of one profile produced the same image bytes across jobs 1, jobs 16 and the
three cache modes (the harness checks this and fails otherwise).

What the rows say:

- **`--jobs` now moves the peak**, unlike the July measurement (flat at
  1649 MiB from jobs 1 to 16) because per-module scratch is now released
  after each module (#3206): the peak is the front end plus the modules in
  flight, 27 MiB per extra worker at debug and 38 MiB at release on dev
  (16 versus 45 on the seed). Serial is the floor; 16 workers cost 406 MiB
  (debug) and 573 MiB (release) for a 2.0x and 2.9x wall reduction.
- **The warm cached build is the front end.** 1071 MiB (debug) and 1086 MiB
  (release) at either worker count: load, resolve, sema, the cell snapshot,
  280 restores (24 MB of entries) and the link. Lowering, optimizing and
  code generation add 786 MiB (debug) and 1199 MiB (release) at jobs 1 on
  top of it.
- **Publication costs 2 to 6 percent of wall** (cold against off) and no
  memory (the entries are serialized from the finished images and freed).
- **Warm against uncached wall:** 3.8x (debug) and 7.5x (release) at jobs 1,
  2.2x and 2.9x at jobs 16, where the uncached build already spreads the
  back half across 16 threads. The 9.4 to 11 s warm build is the serial
  front end; the object cache cannot make it shorter (finding 3 of
  `doc/design/build-overhead-3218.md`).

### Run B: the release-profile compiler on the same tree

Same tree, same cells, the compiler built at `release` (`out/rel/mach`),
three repetitions, load 2.9 to 7.6.

| profile | jobs | mode | peak MiB [min, max] | wall s [min, max] | debug compiler (run A) |
|---|---|---|---|---|---|
| debug | 1 | off | 1868.3 [1868.2, 1868.4] | 37.59 [36.14, 38.42] | 1857.2 / 42.53 |
| debug | 1 | cold | 1868.6 [1868.3, 1868.7] | 38.84 [38.19, 39.12] | 1857.5 / 43.54 |
| debug | 1 | warm | 1074.7 [1074.7, 1074.7] | 7.69 [7.65, 7.94] | 1071.1 / 11.15 |
| debug | 16 | off | 2231.5 [2218.2, 2238.1] | 20.54 [19.47, 21.02] | 2263.0 / 20.80 |
| debug | 16 | cold | 2248.3 [2232.0, 2293.1] | 20.25 [19.82, 21.44] | 2243.7 / 22.05 |
| debug | 16 | warm | 1074.7 [1074.7, 1074.7] | 7.82 [7.70, 8.06] | 1071.2 / 9.38 |
| release | 1 | off | 2294.4 [2288.6, 2298.5] (swapped 1) | 73.32 [72.60, 74.65] | 2285.7 / 82.85 |
| release | 1 | cold | 2294.9 [2294.5, 2299.2] | 72.93 [72.08, 76.63] | 2285.0 / 85.46 |
| release | 1 | warm | 1087.1 [1087.1, 1090.0] | 9.32 [8.14, 9.53] | 1086.3 / 11.09 |
| release | 16 | off | 2868.4 [2850.6, 2877.6] | 25.34 [24.57, 26.20] | 2858.4 / 28.93 |
| release | 16 | cold | 2865.5 [2834.2, 2871.6] (swapped 1) | 25.40 [24.92, 25.71] | 2863.6 / 28.76 |
| release | 16 | warm | 1087.1 [1087.0, 1087.1] | 8.05 [7.85, 8.28] | 1086.4 / 9.88 |

The compiler's own profile does not change what it holds (peaks agree with
run A within 0.6 percent on every cell) and buys 10 to 13 percent of wall
on the uncached builds and about 25 percent on the warm front end. For the
record beside these, the 2026-09-06 archive's self-build rows were 1185 MiB
debug at 17.3 s and 2179 MiB release at 54.7 s on a four-worker CI runner
with the tree of that day; they are a different host, worker count and tree
and are compared only through the seed control and the probe series below.

### Where dev's self-build peak comes from

The dev debug self-build (1857 MiB serial) is 1.73x the seed on its own
tree (1075 MiB) for 1.17x the source, and the release build is 1.18x
(2286 against 1938) for the same 1.17x. At release the two compilers hold
the same memory per line; at debug dev holds about 780 MiB more than the
tree growth explains, and the warm cell locates it in the front end. To
attribute it, four dev commits between the 4.30.0 merge base and today were
each built by the 4.30.0 seed from its own tree with std 1.0.1 (trees
272,774 to 290,037 compiler lines) and measured on their own debug
self-build, two repetitions each, load 2.7 to 5.9:

| commit | what | jobs 1 peak MiB / wall s | jobs 16 peak MiB / wall s |
|---|---|---|---|
| `44d1df47c` | before the F2 query landing | 1098.6 [1098.0, 1099.1] / 36.2 | 1360.7 [1355.8, 1365.6] / 16.7 |
| `614cc4eca` | #3247, owned products and transitive validation | 1465.2 [1465.0, 1465.5] / 66.3 | 1772.5 [1766.7, 1778.3] / 46.0 |
| `2677afd99` | after the #3252 and #3254 decode-once fixes | 1466.0 [1465.6, 1466.4] / 44.9 | 1778.8 [1735.4, 1822.3] / 25.5 |
| `2b44f81c6` | object cache phase 1 (#3273) | 1561.8 [1561.8, 1561.8] / 49.5 | 1893.0 [1875.4, 1910.5] / 29.3 |
| `83d3c1c9d` | today, by the v5 stage compiler, std 2.0.0 | 1857.2 / 42.5 | 2263.0 / 20.8 |

One step accounts for most of it: **#3247 adds 367 MiB (+33 percent) to a
tree of the same size**, and the decode-once fixes that recovered its time
(66 to 45 s) recovered none of its memory, because what they removed was
repeated work, not retained state. The remaining 1466 to 1857 MiB is spread
over the v5 language and migration landings, the cache, and std 2.0.0
(315k + 95k lines against 280k + 85k): 4.0 KiB per source line at
`2677afd99` against 4.5 today, +13 percent per line over 40 merges, none of
them individually measured here.

What #3247 introduced is the owned-product contract of
`doc/design/query-semantic-results.md` and the per-consumer decoded copies
named as finding 2 in `doc/design/build-overhead-3218.md` (the typed surface
of every module in a consumer's import closure, the by-value expression
view, `constant_decode`). The probe series attributes the 367 MiB to that
landing; which of its products stay resident past their consumers, and
whether the copies can be shared or released per computation, is the first
question of the follow-up, and this note does not answer it. The warm cell
(1071 MiB, no lowering or codegen at all) bounds the front end's resident
state on today's tree. It is linear in modules for a fixed import shape (the
synthetic families below are linear with R² above 0.9999 for both
compilers), so it is a level, not a cliff; it is nevertheless the largest
remaining item under #2299's title, with the number to beat: the debug
self-build at `44d1df47c` was 1099 MiB serial on a tree 13 percent smaller.

## Synthetic families

Four generated projects per size, freestanding (no std), executable output
checked against the generator's checksum on every cell, objects checked to
be ELF64, images identical across jobs 1, jobs 4 and the three cache modes
for each compiler (the seed's images differ from dev's, as expected between
compilers, and are not compared). Run A2: 768 processes, three repetitions,
load 1.16 to 11.07 (median 7.52), 667 of them with another lane's compiler
running, 2 swapped. Peak MiB [min, max] / wall s [min, max]; `anon` is the
sampled maximum of `RssAnon+VmSwap` for the serial uncached cell, the
process's own allocations without its mapped executable (the executable's
resident share is the kernel's choice and moved the small cells by up to
4 MiB between staging filesystems, which is why the spawner floor of 9.4
MiB and not the peak is the reference for cells under 20 MiB).

Run A's synthetic cells were measured before the spawner clock and the
disk staging and are superseded by A2 (their peaks agree with A2 within the
file-backed share above, their walls were quantized to 50 ms).

### modules (N modules of 16 functions)

debug:

| N | off, jobs 1 | off, jobs 4 | cold, jobs 1 | warm, jobs 1 | anon | seed off, jobs 1 | seed off, jobs 4 |
|---|---|---|---|---|---|---|---|
| 10 | 14.5 [14.1, 14.5] / 0.056 [0.054, 0.060] | 14.6 [14.4, 14.7] / 0.044 [0.042, 0.047] | 14.6 [14.4, 14.8] / 0.063 [0.059, 0.064] | 10.8 [10.5, 10.8] / 0.035 [0.034, 0.039] | 4.6 | 14.5 [14.3, 14.5] / 0.050 [0.049, 0.051] | 15.0 [14.8, 15.0] / 0.039 [0.036, 0.043] |
| 50 | 26.2 [25.9, 26.6] / 0.209 [0.206, 0.217] | 26.7 [26.6, 26.8] / 0.154 [0.153, 0.158] | 26.6 [26.4, 27.0] / 0.260 [0.254, 0.276] | 17.3 [17.3, 17.3] / 0.129 [0.129, 0.135] | 16.3 | 25.2 [25.1, 25.5] / 0.186 [0.179, 0.186] | 25.3 [25.2, 25.8] / 0.136 [0.135, 0.143] |
| 150 | 57.9 [57.6, 57.9] / 0.633 [0.623, 0.636] | 58.0 [58.0, 58.0] / 0.453 [0.438, 0.464] | 58.0 [58.0, 58.0] / 0.776 [0.752, 0.796] | 33.7 [33.2, 33.7] / 0.383 [0.356, 0.384] | 48.1 | 52.9 [52.7, 53.3] / 0.551 [0.546, 0.595] | 53.0 [52.7, 53.4] / 0.356 [0.346, 0.361] |
| 400 | 133.4 [133.4, 133.4] / 1.568 [1.555, 1.589] | 133.7 [133.6, 133.7] / 1.176 [1.116, 1.177] | 133.6 [133.6, 133.6] / 2.063 [2.060, 2.095] | 73.4 [73.4, 73.5] / 1.051 [0.985, 1.069] | 123.6 | 122.4 [122.4, 122.4] / 1.447 [1.371, 1.457] | 122.5 [122.4, 122.5] / 2.162 [0.953, 2.196] |

release:

| N | off, jobs 1 | off, jobs 4 | cold, jobs 1 | warm, jobs 1 | anon | seed off, jobs 1 | seed off, jobs 4 |
|---|---|---|---|---|---|---|---|
| 10 | 14.9 [14.8, 14.9] / 0.052 [0.052, 0.053] | 15.1 [14.9, 15.1] / 0.040 [0.038, 0.041] | 14.8 [14.8, 14.9] / 0.060 [0.060, 0.061] | 10.5 [10.5, 10.6] / 0.028 [0.027, 0.028] | 5.0 | 14.8 [14.7, 14.8] / 0.048 [0.045, 0.051] | 14.8 [14.6, 15.0] / 0.036 [0.035, 0.038] |
| 50 | 27.1 [26.7, 27.1] / 0.223 [0.222, 0.225] | 26.8 [26.5, 26.8] / 0.159 [0.158, 0.172] | 27.0 [27.0, 27.1] / 0.257 [0.252, 0.269] | 16.0 [15.8, 16.1] / 0.102 [0.100, 0.103] | 17.2 | 24.5 [24.5, 25.0] / 0.205 [0.198, 0.213] | 25.0 [24.6, 25.0] / 0.139 [0.130, 0.142] |
| 150 | 57.7 [56.8, 57.7] / 0.704 [0.668, 1.119] | 57.8 [57.8, 57.8] / 0.480 [0.478, 0.487] | 57.8 [57.4, 57.8] / 0.866 [0.792, 1.381] | 29.6 [29.6, 29.6] / 0.455 [0.340, 0.486] | 47.8 | 49.7 [49.4, 49.8] / 0.850 [0.565, 0.876] | 50.2 [49.4, 50.2] / 0.384 [0.371, 0.415] |
| 400 | 134.3 [134.3, 134.3] / 1.715 [1.703, 1.724] | 134.6 [134.6, 134.6] / 1.202 [1.186, 1.754] | 134.5 [134.5, 134.5] / 2.122 [2.110, 2.294] | 64.0 [64.0, 64.0] / 0.795 [0.795, 0.851] | 124.4 | 115.0 [115.0, 115.0] / 1.481 [1.471, 1.532] | 114.6 [114.4, 115.1] / 0.939 [0.931, 0.949] |

### dense (the same functions in one module, N groups of 16)

debug:

| N | off, jobs 1 | off, jobs 4 | cold, jobs 1 | warm, jobs 1 | anon | seed off, jobs 1 | seed off, jobs 4 |
|---|---|---|---|---|---|---|---|
| 10 | 15.4 [15.3, 15.5] / 0.041 [0.040, 0.044] | 15.5 [15.5, 15.5] / 0.043 [0.041, 0.043] | 15.5 [15.3, 15.6] / 0.043 [0.043, 0.045] | 10.6 [10.5, 10.6] / 0.021 [0.020, 0.021] | 4.3 | 15.6 [15.6, 15.7] / 0.037 [0.034, 0.040] | 15.8 [15.6, 15.8] / 0.037 [0.036, 0.038] |
| 50 | 32.5 [32.4, 32.8] / 0.229 [0.227, 0.231] | 32.6 [32.6, 32.6] / 0.226 [0.225, 0.240] | 32.7 [32.5, 32.7] / 0.240 [0.237, 0.249] | 17.2 [17.2, 17.3] / 0.061 [0.060, 0.061] | 23.0 | 32.1 [32.0, 32.3] / 0.190 [0.190, 0.192] | 31.9 [31.8, 32.1] / 0.188 [0.187, 0.189] |
| 150 | 77.8 [77.4, 77.8] / 1.253 [1.245, 1.303] | 77.9 [77.2, 78.0] / 1.284 [1.273, 1.319] | 77.6 [77.6, 78.0] / 1.326 [1.298, 1.337] | 37.0 [36.8, 37.0] / 0.182 [0.181, 0.189] | 68.1 | 75.1 [74.9, 75.2] / 1.000 [0.999, 1.034] | 74.9 [74.8, 75.3] / 1.024 [1.023, 1.036] |
| 400 | 184.3 [184.1, 184.6] / 9.780 [9.655, 9.918] | 184.5 [184.4, 185.4] / 7.463 [7.431, 7.513] | 184.4 [184.3, 184.5] / 7.909 [7.763, 7.914] | 80.8 [80.8, 80.8] / 0.617 [0.608, 0.625] | 173.7 | 177.1 [176.8, 177.2] / 6.088 [5.734, 8.420] | 177.4 [176.8, 177.8] / 5.613 [5.595, 6.354] |

release:

| N | off, jobs 1 | off, jobs 4 | cold, jobs 1 | warm, jobs 1 | anon | seed off, jobs 1 | seed off, jobs 4 |
|---|---|---|---|---|---|---|---|
| 10 | 15.2 [15.1, 15.3] / 0.039 [0.038, 0.040] | 15.3 [15.2, 15.3] / 0.036 [0.036, 0.037] | 15.2 [15.2, 15.3] / 0.039 [0.038, 0.042] | 10.5 [10.3, 10.5] / 0.016 [0.016, 0.018] | 4.2 | 15.5 [15.5, 15.5] / 0.035 [0.034, 0.036] | 15.4 [15.3, 15.5] / 0.033 [0.033, 0.036] |
| 50 | 31.3 [30.8, 31.3] / 0.152 [0.150, 0.153] | 31.5 [31.0, 31.5] / 0.142 [0.141, 0.143] | 31.4 [31.1, 31.4] / 0.156 [0.154, 0.157] | 16.8 [16.6, 16.8] / 0.046 [0.045, 0.047] | 19.0 | 30.7 [30.6, 30.8] / 0.138 [0.138, 0.140] | 30.8 [30.6, 30.8] / 0.134 [0.131, 0.143] |
| 150 | 73.3 [73.2, 73.6] / 0.483 [0.457, 0.491] | 73.6 [73.2, 73.8] / 0.465 [0.442, 0.467] | 73.6 [73.3, 73.6] / 0.492 [0.476, 0.496] | 35.3 [35.2, 35.6] / 0.128 [0.127, 0.135] | 64.1 | 70.6 [70.5, 70.7] / 0.451 [0.428, 0.457] | 70.8 [70.6, 70.8] / 0.409 [0.409, 0.445] |
| 400 | 172.9 [172.8, 173.0] / 1.335 [1.304, 1.356] | 172.8 [172.6, 173.0] / 1.301 [1.301, 1.386] | 173.1 [172.9, 173.3] / 1.373 [1.348, 1.419] | 77.6 [77.5, 77.6] / 0.302 [0.297, 0.313] | 163.3 | 165.6 [165.2, 165.9] / 1.211 [1.177, 1.214] | 165.4 [165.2, 165.8] / 1.168 [1.158, 1.171] |

### aggregate (one record of N bytes copied by value)

Every cell of both profiles, both compilers, both worker counts and all
three cache modes measured 11.5 to 12.6 MiB (anon 0.2 to 0.9 MiB, the
sampler catching at most one 20 ms sample of a 10 to 30 ms process) at 4096,
16384, 65536 and 262144 bytes: the process floor. The archive measured
aggregate-65536 at 65.9 MiB and 0.15 s on 2026-09-06, before #3109 (landed in
4.30.0) replaced the unrolled 8-byte copy pairs with bulk copies; after it a
256 KiB by-value record costs nothing the harness can see above the floor.
The family is kept as the regression guard for #3109 (its ceiling is the
floor-dominated minimum), not as a curve.

### blocks (one function of N conditional statements)

debug:

| N | off, jobs 1 | off, jobs 4 | cold, jobs 1 | warm, jobs 1 | anon | seed off, jobs 1 | seed off, jobs 4 |
|---|---|---|---|---|---|---|---|
| 500 | 30.6 [30.4, 30.7] / 0.85 [0.83, 0.89] | 30.6 [30.4, 30.7] / 0.86 [0.84, 0.88] | 30.6 [30.6, 30.7] / 0.85 [0.85, 0.86] | 10.3 [10.3, 10.3] / 0.02 [0.01, 0.02] | 21.7 | 31.0 [31.0, 31.3] / 0.72 [0.71, 0.74] | 31.0 [30.9, 31.2] / 0.70 [0.70, 0.74] |
| 1000 | 78.8 [78.8, 79.0] / 3.41 [3.31, 3.79] | 78.8 [78.6, 78.8] / 3.57 [3.37, 4.39] | 78.8 [78.8, 79.1] / 3.42 [3.27, 3.94] | 11.5 [11.2, 11.6] / 0.02 [0.02, 0.03] | 70.1 | 79.0 [78.8, 79.2] / 2.76 [2.69, 2.85] | 79.2 [79.2, 79.2] / 2.81 [2.79, 2.87] |
| 2000 | 261.4 [261.3, 261.5] / 13.43 [13.27, 14.36] | 261.5 [261.5, 261.7] / 13.14 [13.02, 14.00] | 261.6 [261.6, 261.6] / 12.92 [12.76, 13.46] (swapped 1) | 13.7 [13.6, 13.8] / 0.04 [0.04, 0.04] | 252.7 | 261.5 [261.4, 261.5] / 10.46 [10.32, 10.49] | 261.5 [261.5, 261.8] / 10.44 [10.36, 10.64] |
| 4000 | 970.1 [969.9, 970.2] / 50.60 [50.34, 51.86] | 970.0 [969.9, 970.0] / 50.91 [50.83, 51.12] | 970.0 [970.0, 970.3] / 50.67 [50.38, 51.89] | 18.7 [18.6, 18.7] / 0.06 [0.06, 0.07] | 961.2 | 969.2 [969.1, 969.3] / 41.22 [40.69, 41.23] | 969.1 [969.1, 969.3] / 41.03 [40.73, 41.07] |

release:

| N | off, jobs 1 | off, jobs 4 | cold, jobs 1 | warm, jobs 1 | anon | seed off, jobs 1 | seed off, jobs 4 |
|---|---|---|---|---|---|---|---|
| 500 | 30.4 [30.4, 30.4] / 1.59 [1.57, 2.46] | 30.5 [30.4, 30.5] / 1.59 [1.58, 1.60] | 30.8 [30.5, 30.8] / 1.60 [1.57, 2.20] | 10.2 [10.2, 10.2] / 0.02 [0.01, 0.02] | 21.6 | 31.1 [31.0, 31.2] / 1.31 [1.27, 1.76] | 31.1 [30.9, 31.2] / 1.27 [1.26, 1.35] |
| 1000 | 78.8 [78.6, 79.1] / 6.48 [6.44, 7.24] | 78.9 [78.8, 78.9] / 6.28 [6.24, 6.39] | 78.7 [78.6, 78.9] / 6.80 [6.29, 7.49] | 11.2 [11.1, 11.3] / 0.02 [0.02, 0.03] | 69.9 | 79.4 [79.3, 79.4] / 5.29 [5.26, 5.46] | 79.0 [78.9, 79.4] / 5.45 [5.24, 6.96] |
| 2000 | 261.2 [261.2, 261.3] / 24.77 [24.61, 24.78] | 261.2 [261.2, 261.2] / 24.94 [24.87, 25.04] | 261.4 [261.3, 261.4] / 24.96 [24.54, 25.32] | 13.6 [13.3, 13.7] / 0.03 [0.03, 0.04] | 252.3 | 261.3 [261.3, 261.4] / 20.71 [20.68, 20.83] | 261.3 [261.3, 261.4] / 20.64 [20.61, 20.66] |
| 4000 | 969.2 [969.2, 969.2] / 99.49 [96.32, 100.11] | 969.2 [969.2, 969.2] / 96.80 [96.62, 99.00] | 969.3 [969.3, 969.3] / 97.28 [97.01, 97.45] | 18.2 [17.9, 18.3] / 0.06 [0.06, 0.06] | 960.2 | 968.4 [968.4, 968.4] / 81.21 [81.07, 81.85] | 968.4 [968.4, 968.4] / 81.38 [81.11, 81.83] |

### Linearity

Least-squares fits of serial uncached peak against size over the four sizes
(A2, three repetitions per point):

| family, profile | compiler | intercept MiB | slope | R² | max residual MiB | segment slopes 10→50, 50→150, 150→400 |
|---|---|---|---|---|---|---|
| modules, debug | dev | 11.4 | 0.3054 MiB/module | 0.99992 | 0.66 | 0.293, 0.317, 0.302 |
| modules, debug | seed | 11.5 | 0.2771 | 0.99999 | 0.23 | 0.268, 0.277, 0.278 |
| modules, release | dev | 11.8 | 0.3062 | 1.00000 | 0.03 | 0.305, 0.306, 0.306 |
| modules, release | seed | 11.7 | 0.2576 | 0.99988 | 0.66 | 0.243, 0.252, 0.261 |
| dense, debug | dev | 11.4 | 0.4333 MiB/16 functions | 0.99985 | 1.38 | 0.428, 0.453, 0.426 |
| dense, debug | seed | 11.8 | 0.4141 | 0.99989 | 1.16 | 0.413, 0.430, 0.408 |
| dense, release | dev | 11.5 | 0.4044 | 0.99989 | 1.14 | 0.403, 0.420, 0.398 |
| dense, release | seed | 11.9 | 0.3851 | 0.99991 | 0.96 | 0.380, 0.399, 0.380 |

Both families are linear over a 40x span with no upturn in the last
segment; dev's slope is 10 percent (modules) and 5 percent (dense) over the
seed's. The warm cached cells are linear too (modules 0.16 MiB/module, dense
0.18 per group), which is the front end's share. The July curve for the same
`modules` shape was 0.84 MiB/module (compiler at `e9290c42`); it is 0.31
today, and the 2026-09-06 archive's modules-150 debug serial cell was 52.1
MiB (CI runner) against 57.9 here and 52.9 for the seed. Wall is linear in
`modules` (1.57 s at 400) and, at release, in `dense` (1.34 s at 400).

### Two cliffs, reported with their cause (not fixed here)

**blocks: memory and time quadratic in one function's size.** Peak per
doubling of N: 2.58x, 3.32x, 3.71x (approaching 4x; 970 MiB for one
function of 4000 conditionals at either profile), identical for dev and the
seed to within 1 MiB, so it predates everything measured here. A 50 ms
`VmRSS` trace of the dev debug build at N = 2000 stays at 20 MiB through
lowering and the 10.6 s optimize phase and rises to 261 MiB inside the 1.7 s
codegen phase: `regalloc.extend_liveness` allocates four `block_count x
vreg_count` boolean matrices (`uses`, `defs`, `live_in`, `live_out`,
`src/lang/be/codegen/regalloc.mach`). These are the "dense block-by-vreg
structures" in #2299's acceptance. A sparse replacement exists as
`b6aa8df12` ("perf(codegen): replace dense liveness matrices with sparse
propagation", on `feat/3112` and `archive/3218-candidate-era`, 610 lines)
but it does not apply to dev, whose `regalloc.mach` has diverged by 1306
lines since that commit's parent; it is a port, not a cherry-pick.

Wall per doubling is 3.8x to 4.1x at both profiles, and `perf` on the
same build puts 71 percent of it in the optimize phase's IR verifier:
`ir.block_successors` (38.5 percent), `ir.instruction_get` (15.9),
`verify.check_pred_consistency` (8.6) and `ir.block_target` (8.4).
`check_pred_consistency` (`src/lang/me/ir/verify.mach`) recomputes every
block's successors for every block, O(B²) per verification, and the release
pipeline verifies more often (24.8 s against 13.4 s at N = 2000). The fix
is one successor sweep per verification; it is separate from the liveness
matrices and from #3147, which removed the same shape from the DWARF
`validate_request` prefix scan.

**dense at `debug`: time quadratic in the functions of one module.** Memory
is linear (above) but debug wall is not: 6402 functions in one module take
9.78 s at `debug` against 1.34 s at `release`, and dense-150 to dense-400 is
7.8x the time for 2.67x the functions (the seed shows the same, 6.09 s).
`perf` on the dense-400 debug build puts 71 percent of the process in four
`be/codegen/dwarf.mach` helpers: `ir_function_at` (23.7 percent) scans every
symbol of the image for each variable location and inline PC, `build_line`
(23.0) and `first_row_loc` (18.2) scan every line row for each function, and
`gather_vars` (6.4) likewise, so debug-information emission is O(F²) in the
functions of one module. The synthetic profile has `debug = true`; the
compiler's own profiles do not, which is why the self-build does not show
it. The fix is one sweep over the offset-sorted symbols and rows per module
(or a binary search into a function table sorted by offset).

## Storage bounds (#3221)

The store limit is 512 MiB (`store.MAX_STORE_BYTES`, 536,870,912 bytes of
entries), the entry limit 64 MiB (`image.MAX_ENTRY_BYTES`). The unit test
`cache.store:eviction_is_oldest_publication_first_under_the_limit_and_keeps_this_builds_entries`
publishes at distinct mtimes in an order that disagrees with name order,
evicts under a small limit, checks the oldest went, a protected entry
survived while the next oldest went, a publication with only protected
entries left was declined with the store unchanged, and a run of
publications never exceeded the limit; it passes in the suite run below,
and dropping the protected check fails it (phase 2b control).

**Publish loop past the limit, end to end.** The compiler's own tree
(debug, 280 modules, 23,963,037 bytes of entries per generation, largest
entry 722,345 bytes) built with `--cache` 24 times by the debug compiler,
each round appending a distinct comment to `src/lang/version.mach` so the
cell key changes and every module republishes (0 restores per round, by the
`-vv` log), the store inventoried after every round (`publish-loop.json`
beside this note). Load 1.8 to 4.9.

| round | entries after | bytes after | new | evicted | from generation | wall s |
|---|---|---|---|---|---|---|
| 0 | 280 | 23,963,037 | 280 | 0 | | 21.75 |
| 10 | 3,080 | 263,593,407 | 280 | 0 | | 24.09 |
| 20 | 5,880 | 503,223,777 | 280 | 0 | | 26.21 |
| 21 | 6,160 | 527,186,814 | 280 | 0 | | 26.61 |
| 22 | 6,229 | 536,415,561 | 280 | 211 | 0 (the oldest) | 26.90 |
| 23 | 6,229 | 536,415,561 | 280 | 280 | 0 (its last 69), then 1 | 26.74 |

The store never exceeded the limit (maximum 536,415,561 of 536,870,912
bytes), eviction began exactly at the generation that would have crossed
it and removed the oldest entries first by mtime (generation 0 entirely
before any of generation 1), the round's own 280 entries were all present
after each round, and every round exited 0 with the same image. Two
observations for the record: with a whole-cell key each generation is a
full republish, so the store holds 22 generations of this tree; and the
publication cost grows with the store because `make_room` inventories the
directory (`stat` per entry) under the lock on every publication, 280
inventories of up to 6,229 entries per build, which is the 21.8 to 26.9 s
climb across the loop (4 to 5 s at the limit on a 21 s build).

## Resident ownership bounds (#3221)

The decoded-owner budget is `image.MAX_ENTRY_BYTES`, 64 MiB per entry: the
decoder charges every allocation it makes against it (`Reader.resident`,
starting at 4 KiB) and refuses with `LIMIT` before allocating past it, so
a hostile count in a corrupt entry cannot allocate unboundedly
(`cache.image:truncation_trailing_counts_and_semantics_refuse_without_interning`,
the `MAX_ENTRY_BYTES + 1` and the oversized frame-count cases). Under the
largest workloads the budget has this margin:

| workload | entries | store bytes | largest entry | of the 64 MiB budget |
|---|---|---|---|---|
| self-build, debug | 280 | 23,963,037 | 722,345 | 1.1 percent |
| self-build, release | 280 | 28,094,915 | 845,095 | 1.3 percent |
| dense-400, debug (6402 functions, one module) | 1 | 2,594,108 | 2,594,108 | 3.9 percent |
| modules-400, debug | 401 | 3,360,152 | 51,323 | 0.1 percent |
| blocks-4000, debug | 1 | 220,033 | 220,033 | 0.3 percent |

A decoded image charges about its entry size plus the collection headers,
so the largest object the compiler's own tree or any synthetic family
produces uses under 4 percent of one entry's budget. The resident state a
warm build carries is the front end plus the restored images: the
self-build warm cells (1071 MiB debug, 1086 release) against the uncached
1857 and 2286 show the 280 restored images (24 to 28 MB of entries) are
not visible above the front end, and the warm synthetic cells are 0.16 to
0.18 MiB per module or group against 0.31 to 0.43 uncached. Restored
products are boxed on the module and published by the same `Q_CODEGEN`
compute as generated ones, so no live pointer or arena crosses the file
(`driver.cache:operation_reset_keeps_restored_products_under_their_snapshot_and_drops_fresh_ones`).

## Ownership controls (#2299)

The scratch owners landed on 2026-09-06 (`bb0a9deb` per-module codegen
scratch, `98102529` the optimizer workspace and typed transform results)
carry their guards in the tree, each under the tracking allocator with
fail-at-N so a leak, a double free or a premature reclamation fails the
test. The four mutation anchors from `doc/design/2299-archive-inventory.md`
were re-run against dev `83d3c1c9d` through the debug compiler
(`mach test . --filter <test>`), each mutation applied alone and reverted
(`mutations.txt` beside this note):

| guard | mutation | result |
|---|---|---|
| `be.codegen.scratch:module_images_survive_reclaimed_working_storage` | `fin { arena.dnit(?scratch); }` to `fin { }` in `codegen_with_backing` | FAIL (exit 3), as recorded |
| `me.pipeline:simple_passes_do_not_retain_function_work_in_ir` | `verify_work` verifies through `m.alloc` instead of the workspace `alloc` | FAIL (exit 10), as recorded |
| `me.pass.scalarize:rejected_work_does_not_retain_maps_in_ir` | `scalarize_work` allocates its `pval` map from `m.alloc` instead of `alloc` | FAIL (exit 4), as recorded |
| `me.pass.scalarize:failed_instruction_releases_untransferred_operands` | delete the `A.deallocate[value.Value](m.alloc, owned, ...)` in the refused path | FAIL (exit 3), as recorded |

One precision on the third recipe: the archive's "allocates from `m.alloc`
instead of `alloc`" means the `A.allocate` calls in `scalarize_work`, not
the `c.alloc` field, which only routes the deallocations; redirecting the
field alone leaves the test green because the rejected work allocates
nothing through it.

At whole-build scale the harness is the premature-reclamation control:
every cell's image is byte-identical across jobs 1, jobs 4 or 16, and the
three cache modes for one compiler (a scratch freed and then read would
change bytes or crash), and the synthetic executables print the expected
checksum on every cell. The suite run for this note (below) covers the
remaining tracking-allocator guards (`driver:load_module_registration_fail_at_n_no_leak`,
`driver:cascade_libs_fail_at_n_releases_scratch`, the settled-live reload
tests, `cache.store:allocation_refusal_is_internal_and_releases_operation_owners`,
`cache.image:decode_allocation_refusal_releases_codec_and_image_owners`).

## Regression thresholds

`THRESHOLDS` in `test/memory.py` holds a peak ceiling in MiB per workload
and profile, applied to every jobs and cache-mode cell of that workload for
the compiler under test (never the control); the run fails when any cell's
largest repetition exceeds it. Each is the largest peak measured in runs A,
B and A2 (both compiler profiles, every jobs and cache mode) times a
multiple:

- **self-build, 1.25x**: 2867 MiB debug (measured 2293), 3601 release
  (measured 2880). The spread with THP disabled is under 1 percent serial
  and under 3 percent at 16 workers, so 25 percent is far outside noise,
  and it is smaller than any regression this program has seen: reverting
  either 2026-09-06 fix would add 52 percent (codegen scratch) or 17 to 36
  percent (optimizer scratch), and #3247 added 33 percent. A CI runner with
  four workers sits below the 16-worker peak, so the ceiling holds there
  with more margin.
- **synthetic families, 1.5x with a 32 MiB minimum**: modules 41 / 88 /
  201 MiB at 50 / 150 / 400, dense 50 / 118 / 279 debug and 48 / 111 / 260
  release, blocks 47 / 119 / 393 at 500 / 1000 / 2000, and 32 MiB for
  every cell within a few MiB of the spawner floor (modules-10, dense-10,
  the aggregate family). The multiple is wider than the self-build's
  because these peaks are 10 to 200 MiB, where the mapped executable's
  resident share (up to 4 MiB between staging filesystems) and the spawner
  floor are a visible fraction; 1.5x still catches a return of the July
  slope (0.84 against 0.31 MiB/module would be 2.7x at 400 modules) and a
  doubling of any family. The blocks ceilings describe the dense liveness
  sets above and come down when they are replaced.
- **no wall-time thresholds.** Wall on this host moved up to 20 percent
  with the other lanes' load (the load column is beside every row), and a
  CI runner is a different machine; time is recorded per cell and read
  from the curves, not gated.

The dispatch-only `compiler memory` workflow runs the harness with these
ceilings and fails above them. The gate run for this note (the harness's
defaults through the debug compiler, one repetition, 186 processes) exited
0 with every cell under its ceiling (self-build 1865.8 serial and 2276.5 at
16 workers against 2867). Threshold enforcement has its own control:
lowering `modules-10-debug` to 8 MiB makes a run of that cell exit with
`3 cells exceeded their peak-RSS ceiling: modules-10-debug-jobs1-off,
modules-10-debug-jobs1-cold, modules-10-debug-jobs1-warm`, and the same
run with the table as committed is green.

## Suite and census

Full suite through the debug compiler at `83d3c1c9d`: 3003 passed, 0
failed, 3003 total (the dev baseline at that commit; this pull request adds
no tests). `sh test/census.sh`: 10 of 10 ok.

## Files

`doc/design/r2-measurements/` beside this note holds the provenance and
summary of every run (`runA`, `runB`, `runA2`, the four probes), the publish
loop rounds and the mutation results, all produced by `test/memory.py` and
the two scratch scripts described above; per-process logs and censuses are
in the run directories and not retained. The harness options are in
`test/memory.py`'s docstring.
