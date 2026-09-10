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

## Step 2: bisect

Oracle: median wall of three `one` o0 builds by a compiler built from the probe
commit (default `debug` profile), threshold 670 ms, halfway between BD 337 and ND
1005. 78 first-parent commits from the merge base. Probes before 52a53af8 were built
by the dev seed (the bootstrap rejects their manifest); the rest by the v5
bootstrap. Twelve commits pin std at dcb33e1c, c08f0f83 or 58ca9ea1, which exist in no
local clone and are not on the mach-std remote; they were skipped.

| index | commit | one o0 wall (ms) | sema | lower | codegen |
|---|---|---|---|---|---|
| base | a4107eb4 | 337 | 38 | 84 | 68 |
| 1 | d9373679 fix(query): own reusable products and validate transitive inputs | 824 / 862 / 859 | 343 | 197 | 66 |
| 2 | d2bbbfba | 823 | 342 | | 64 |
| 3 | 6c0a27c5 | 826 | 343 | | 66 |
| 5 | f15a628d | 816 | 343 | | 67 |
| 10 | fec48251 | 819 | 344 | | 65 |
| 13 | f242eeb3 | 834 | 343 | | 68 |
| 26 | 60570613 feat(build): cache object images across compiler processes | 1167 | 352 | | 376 |
| 39 | 386046a4 | 1164 | 351 | | 381 |
| 78 | 62966055 (tip) | 971 | 223 | 197 | 387 |

First commit over the threshold: d9373679 (index 1), which alone takes the one-file
build from 337 ms to about 830 ms. A second step comes from 60570613 (index 26), which
adds about 330 ms of codegen-phase time (the compiler digest plus cache store writes).
Between 39 and 78 something recovers about 130 ms of sema; not bisected further.

Mechanisms in d9373679 (call counts from gdb on the `one` build, 41 modules):

- `read_definition` runs 12226 times and calls `resolve.remap_result` on every call,
  which walks the whole symbol table of the origin module and does one hash lookup
  per symbol. Module ids do not change inside an operation, so 12007 of those walks
  repeat work already done.
- `imported_type_export` runs 3633 times; for every forwarded public symbol it decoded
  the complete typed surface of the origin module (`typed_surface_decode`), picked one
  row and freed the copy. 1811 full surface decodes per build.
- `build_sema_deps_query` runs twice per module and decodes an owned copy of the
  surface of every module in the import closure for each consumer.
- `acquire_symbol` finds a symbol by a linear scan of the origin's symbol table on
  every acquisition.

Mechanism in 60570613: `cache.compiler.digest` SHA-256s `/proc/self/exe` once per
process (12.8 MB debug, 17.9 MB release binary) using the std pure-mach sha256, which
runs at about 46 MB/s in the debug compiler and 130 MB/s in the release compiler.
`--no-cache` skips it: `nostd` goes from 282 ms to 7 ms, `one` from 1005 to 684 ms.

## Step 3: fixes

Fix A: `build_type_surface` decodes each origin surface once and shares it across the
symbols that forward from it (`origin_surface`). Fix B: `remap_result` records the
operation sequence and module census it last ran under and returns early when both
are unchanged. Both keep the owned-product contract: the surface copies are still
owned by the builder for the duration of the build and freed with it.

`one` o0, ND (debug profile) after A+B, median of 3:

| build | wall | sema | lower | codegen |
|---|---|---|---|---|
| before | 1005 | 223 | 199 | 406 |
| after | 812 | 93 | 163 | 385 |
| after, --no-cache | 484 | 93 | 162 | 76 |

`read_typed_surface` calls: 1811 -> 1064. `remap_result` still entered 12007 times but
returns at the census check.
