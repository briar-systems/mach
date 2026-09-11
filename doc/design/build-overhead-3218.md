# Build overhead on feat/3218 (#2299, #3218)

Working log for the per-build slowdown measured on the v5 integration branch.
Numbers are wall and `-v` phase times on x86_64-linux, serial, 3 repetitions each,
host load average 7 to 8 from a concurrent test-suite run (noise under 5% on every
row below, worst observed spread 61 ms on a 1.3 s row).

## Landing on dev (#2299)

The same regression reached dev through PR #3247, which carried d9373679's owned
products and transitive validation but not 60570613's object-image cache: the CI
targets job's corpus step went from 342 to 620 seconds. Fixes A and B apply to dev
unchanged and were cherry-picked from 60c2c479. Fix C has no target on dev, since
dev never hashes the running compiler (there is no `src/lang/build/cache/`); it
lands with the object cache under F4 (#3221). Finding 1 below (the per-invocation
digest) therefore does not apply to dev today.

On dev the regression is per-artifact compile time in the sema and lower phases,
not per-process overhead. `-v` on one o2 corpus artifact (43 modules, 40 of them
std), debug compilers built by the 4.30.0 seed, median of 3, ms:

| compiler | resolve | sema | lower | optimize | codegen | wall |
|---|---|---|---|---|---|---|
| pre-#3247 dev (44d1df47) | 22 | 37 | 82 | 271 | 113 | 615 |
| dev (853d4c17) | 27 | 212 | 261 | 274 | 122 | 986 |
| dev + A + B | 26 | 86 | 180 | 274 | 115 | 781 |
| dev + A + B + E + F | 28 | 75 | 117 | 275 | 115 | 706 |

Optimize and codegen do not move on dev. A + B recovered 72% of the sema delta and
45% of the lower delta, so the remainder was profiled (`perf record` on the single
build, stacks diffed against the pre-#3247 compiler). The lower phase's serial
main-thread work (`lower_raw`) had grown from 421 to 921 samples, and 346 of the
new samples were `read_definition` reached from `lower_callee`: every imported
call site acquired the callee's current definition through the query layer again,
and each acquisition re-ran `fields.install` (223 samples) to verify the origin's
field graph against the already published tables, plus a Q_SEMA lookup with its
dependency edge (101) and the parsed-definition lookup (48). The optimize pipeline
inclusive of verification is identical in both compilers (5873 vs 5930 samples); it
only shifts between the phase timers because the lower workers overlap it.

Fix E: the definition reader remembers each origin it has acquired for the life of
the reader (one sema or lower computation), serving later requests at the same or a
lower phase from the entry and upgrading it when a higher phase is asked for. The
first acquisition still records the dependency edge. Fix F: a field graph records
the projection generation it was installed under; published tables only change
through a projection reset, which bumps the generation, so a second install under
the same projection returns at once. Both carry a unit test that fails with the
memo disabled and nothing else changes (2676 tests either way, one failure each).

Fix D (the `definition_index` map replacing the linear scan in `acquire_symbol`,
fe56b9ad) was retried on top of E + F: lower 113/130/115/113/115 ms without it
against 114/109/110/109/114 with it, five alternating repetitions. About 5 ms
inside the spread, the same verdict as step 4; not taken.

What remains over the pre-#3247 compiler, sema +38 ms and lower +35 ms, is the
accepted contract: the owned typed-surface copy decoded per consumer in
`build_sema_deps_query` (about 80 samples in each phase), the by-value expression
view through `interpretation.get` (#3121, about 40 per phase), the per-consumer
`constant_decode`, and the generic-instance queueing in sema. Those are finding 2,
handed to F4/R2.

The corpus wall numbers for this landing are in the pull request.

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

## Step 4: second pass, verification, and what remains

Compilers for this step were all built from this branch by the v5-integration
bootstrap on 2026-09-10 (`base` is 60c2c479, fixes A+B; `fixc` adds fix C), plus the
merge base a4107eb4 rebuilt at release by the installed seed (`BR`). Host load
average 7 to 12 throughout from concurrent workers; every row is 3 repetitions in
alternation, spreads stated where they exceed 10%.

Review of the interrupted worker's uncommitted edits:

- Fix C, `cache.compiler.digest` memo: kept. Every build unit begins a fresh
  `Project` (`engine.execute_warm` -> `driver.begin_build` -> `init_project`), which
  resets `compiler_identity_initialized`, so an N-unit process hashed the executable
  N times. The running image cannot change under a live process, and the digest is
  only requested from the operation setup on the main thread, so one hash per process
  is the same value every time. Proven by strace on a 3-artifact freestanding
  project: `/proc/self/exe` opened 3 times by `base`, once by `fixc`.
- Fix D, `ResolveResult.definition_index` replacing the linear scan in
  `acquire_symbol`: dropped. The scan runs 9134 times on the `one` build (gdb), but
  the timing cannot separate it from noise: sema is 93 to 100 ms for `base`, `fixc`
  and a `fixd` build alike at o0, 65 to 80 ms at release. A fix whose reversal
  restores no cost has no mutation control; it also adds a map to every resolve
  product. The patch is preserved on commit fe56b9ad for the day a profile shows it.

Fix C numbers. Single-unit builds are unchanged by construction (`one` o0 debug:
`base` 825/827/852 ms, `fixc` 830/845/853 ms; release 570/583/591 vs 595/604/609;
`nostd` 283 to 305 ms both). The 91-artifact corpus project built by one process
(`corpus91`, o0, debug compiler):

| compiler | run0 | run1 | run2 | median |
|---|---|---|---|---|
| base | 100878 | 106755 | 112345 | 106755 (spread 11%) |
| fixc | 81290 | 75793 | 75421 | 75793 (spread 8%) |

The 25 to 31 s recovered is 90 digests at 275 ms each. Reverting fix C is the `base`
row. The real corpus driver (`test/run.sh`) spawns one `mach build --bin` per case,
so it still pays the digest 91 times; that is finding 2, below.

Suite: `mach test . --jobs 8` with the fix-C debug compiler passes 2799 of 2799;
`base` passes 2798 of 2798. The delta is the one added test, which proves the memo
by perturbing it: with the memo branch disabled that test fails and nothing else
changes.

Matched-profile regression that remains (release compilers, `one`, median of 3, ms):

| project | profile | BR | fixc | ratio | where |
|---|---|---|---|---|---|
| one | o0 | 269 | 636 | 2.4x | sema 23->69, lower 58->121, codegen 66->271, resolve 15->25, link 7->17 |
| one | o2 | 526 | 834 | 1.6x | same phases; optimize equal |
| nostd | o0 | 7 | 159 | 23x | codegen: the executable digest |

Was 2.8x / 1.9x at step 1. Of the codegen delta on `one`, about 150 ms is the
digest plus cache store and about 55 ms is per module. Warm second build with the
object cache populated (`fixc`, o0, 3 reps): debug 861 to 882 cold -> 778 to 829
warm, release 572 to 595 -> 509 to 524; only the object compile (about 70 ms of
codegen) is recovered, the digest and the per-module sema/lower costs are not.

Handed to F4/R2, not optimized here because each is the cost of an accepted contract:

1. Per-invocation executable digest, 139 ms release / 275 ms debug, every process.
   The cache key is the content of the running compiler, and content hashing 13 to
   18 MB with the pure-mach std sha256 (46 / 130 MB/s) is what that costs. Ways out
   all change a contract or a layer: a linker-emitted build id (ELF note, LC_UUID, PE
   debug directory) read from the running image with a full-hash fallback when the
   note is absent; a digest sidecar keyed by file identity (device, inode, size,
   mtime, ctime); a faster hash in std. The 91-process corpus lane pays this 91
   times per target.
2. Per-module-loaded cost from d9373679 (owned products, transitive validation):
   `typed_surface_decode`, `constant_decode` and `verify_function` over the 40 std
   modules, about 46 ms sema + 64 ms lower + 55 ms codegen + 20 ms resolve/link at
   release for `one`. Decoding an owned copy per consumer and verifying every lowered
   function are the contract; fixes A and B removed the part that was repeated.
3. The warm cache recovers only the object compile. A hit still decodes, lowers and
   verifies every module; whether the cache should also carry the front-end products
   is a design question for the query engine, not a redundancy.
