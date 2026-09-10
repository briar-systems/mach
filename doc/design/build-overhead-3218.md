# Build overhead on feat/3218 (#2299, #3218)

Working log for the per-build slowdown measured on the v5 integration branch.
Numbers are wall and `-v` phase times on x86_64-linux, serial, 3 repetitions each,
host load average 7 to 8 from a concurrent test-suite run (noise under 5% on every
row below, worst observed spread 61 ms on a 1.3 s row).

## Step 1: shape

Compilers (all built from source on this host, 2026-09-10):

| label | source | profile | builder |
|---|---|---|---|
| P  | dev 4.30 seed (`~/.local/bin/mach`) | release (matches BR within noise) | installed |
| BD | merge base a4107eb4 | debug (opt 0) | P |
| BR | merge base a4107eb4 | release (opt 2) | P |
| ND | feat/3218 at 123399ef | debug (opt 0) | v5-integration bootstrap (debug) |
| NR | feat/3218 at 123399ef | release (opt 2) | v5-integration bootstrap (debug) |

Projects: `one` is one file plus `[dep.std] path = "dep/std"` (std at the branch pin
ad7add30, 40 std modules reached); `nostd` is the same file freestanding, no dependency.

`one` at profile o0 (median of 3, ms):

| compiler | wall | load | resolve | sema | lower | optimize | codegen | link |
|---|---|---|---|---|---|---|---|---|
| P  | 248 | 35 | 13 | 21 | 58 | 26 | 59 | 7 |
| BR | 237 | 36 | 13 | 21 | 56 | 27 | 57 | 6 |
| BD | 337 | 52 | 19 | 38 | 84 | 37 | 68 | 11 |
| NR | 673 | 39 | 23 | 160 | 138 | 27 | 235 | 16 |
| ND | 1011 | 54 | 30 | 223 | 199 | 37 | 406 | 28 |

`one` at profile o2: P 440, BR 450, BD 614, NR 870, ND 1296.

`nostd` at o0 (1 module): P 6, BD 6, BR 6, NR 146, ND 282. The whole difference is
the codegen phase: 139 ms (NR) / 275 ms (ND) against 0.2 ms.

Findings:

1. The reported 3.7x compares a release dev compiler against a debug-profile branch
   compiler (the branch manifest defaults to `debug`, and `build . -o` takes the
   default). At matched profile the regression is 2.8x (BR 237 -> NR 673) at o0 and
   1.9x at o2. The profile mismatch accounts for ND/NR = 1.5x on top.
2. A fixed per-invocation cost of 139 ms (release) / 275 ms (debug) that does not
   depend on the project at all. perf: 100% of it is `std.crypto.hash.sha256` called
   from `mach.lang.build.cache.compiler.digest`, which reads and hashes
   `/proc/self/exe` (12.8 to 17.9 MB) once per build to key the persistent object
   cache (`src/lang/build/cache/compiler.mach`, reached from
   `driver/cache.mach:prepare_operation` out of the codegen phase).
3. A per-dependency-load cost that scales with modules reached, not artifacts:
   for 40 std modules at NR vs BR, sema +139 ms, lower +82 ms, codegen +37 ms beyond
   the digest, link +10 ms, resolve +10 ms (about 280 ms). perf on ND shows
   `driver.query.typed_surface_decode` (11%), `me.ir.verify.verify_function` (11%)
   and `driver.query.constant_decode` as the largest new inclusive frames.
4. The warm-cache second build of `one` is not faster than the cold one (1.2 s vs
   1.0 s on ND), so the object cache does not recover the sema/lower cost and the
   codegen phase does not shrink on a hit.

Shape: per invocation (digest) plus per module loaded (sema/lower/codegen/link),
nothing per artifact beyond that. The 91-case corpus pays both on every one of its
91 invocations because each artifact is built by a separate `mach build --bin`.
