# Archive inventory for #2299 scratch ownership (F5 remainder, O1)

Inventory of the 43 commits from 2026-09-05 and 2026-09-06 on the fourteen
`fix/2299*` branches that survive only in the workspace archive bundle. They were
written against dev of 2026-09-05 (archive base `3a962411`, std `c2e2340`, later
`3ee8e709`), before the F2 query landing (#3247), the F5 decode fixes (#3252,
#3254) and the `sel` to `pick` rename. Every branch, its unique commits and the
verdict are recorded here so the bundle can be discarded.

Roadmap rows: F5 is the #2299 measurement remainder, O1 is #3123 allocating
transform ownership.

## Verdicts

| group | commits | on dev as | applies to dev | row | verdict |
|---|---|---|---|---|---|
| (a) per-function optimizer scratch, typed transform results | 8 | `98102529` (2026-09-06), diff empty on `src/lang/me/` | 4 conflict on already-present hunks, 4 empty | O1 | superseded |
| (b) module scratch release after object retention | 1 | `bb0a9deb` (2026-09-06), `codegen.mach` byte-identical | CHANGELOG conflict only | F5 | superseded |
| (c) scalarization ownership | 2 | `98102529`, then #3123 fail-at-N cleanup `bd3ffad8`, `1311df15` | conflict on already-present hunks | O1 | superseded |
| (d) memory measurement harness and CI workflows | 15 | nothing on dev | new files apply, but every script pins archive SHAs, std `3ee8e709` and seed `v4.26.5`, and every workflow triggers on a deleted branch | F5 | extract, de-pinned (see below) |
| (e) native proof and mutation CI drivers | 10 | the in-tree guards they exercise landed with (a) to (c) | new files apply, pinned the same way | O1 | superseded, mutation controls re-run against dev instead |
| (f) fixture and phase-outcome fixes | 4 | `e1885b82` (2026-09-06), a superset | 22, 3 and 1 conflicts on already-present hunks, one empty | neither | superseded |
| merge commits | 3 | n/a | n/a | n/a | discard |

Nothing in (a), (b), (c) or (f) is unlanded: the coordinator squashed the three
source branches onto dev on 2026-09-06 as `e1885b82`, `bb0a9deb` and `98102529`
before the archive was taken. `src/lang/me/scratch.mach` exists on dev with the
bounded workspace, and the four proof tests below are in the tree.

## The branches

| branch | tip | unique commits | contents |
|---|---|---|---|
| fix/2299 | `5932eca2` | 7 | (b) `fb4f28f4` and (f) `885ffee5`, `032c1a49`, `9b97449f`, `f7792877`, two merges |
| fix/2299-proof | `b2a2770f` | 6 | (e) `96bc615e`, `129962c2`, `b2a2770f` over fix/2299 |
| fix/2299-memory-baseline | `31be33ea` | 8 | (d) `db681836`, `ca012deb`, `200acdcb`, `49ddc059`, `17b4ed7a`, `6769cce9`, `dbd7b3fe`, `31be33ea` off `feef0bc5` |
| fix/2299-memory-after | `a018751b` | 7 | (d) `a018751b` over fix/2299 |
| fix/2299-memory-control | `ba8f1000` | 8 | (d) `ba8f1000`, a revert of (b) used as the matched control |
| fix/2299-memory-matched | `25ab8ffc` | 8 | (d) `25ab8ffc` over fix/2299 |
| fix/2299-optimizer | `298b0ac5` | 13 | (a) `e8bd94fa`, `aca8a9ab`, `82e5ea21`, `9af91635`, `a7b49168`, `298b0ac5` |
| fix/2299-optimizer-proof | `7f0b0f11` | 12 | (e) `c2a488b7`, `ebe15e2c`, `882fa40c`, `7f0b0f11` |
| fix/2299-optimizer-matched | `b577205b` | 12 | (d) `a2278ebd`, `46bb0657`, `b577205b` |
| fix/2299-transforms | `bfdf7890` | 16 | (a) `9ecef8ab`, `bfdf7890`, one merge |
| fix/2299-transforms-proof | `0d0c3f50` | 15 | (e) `02d6e973`, `0d0c3f50` |
| fix/2299-transforms-matched | `24046fb0` | 13 | (d) `24046fb0` |
| fix/2299-scalarize | `5109e34e` | 19 | (c) `e3115052`, `5109e34e` |
| fix/2299-scalarize-proof | `12588405` | 18 | (e) `12588405` |

## Group (a), (b), (c): landed

The dev landings are byte-identical to the archive tips on every file the archive
commits touch. `git diff 5109e34e 98102529 -- src/lang/me` is empty, and so is
`git diff fb4f28f4 bb0a9deb -- src/lang/be/codegen.mach`. The (c) fail-at-N
ownership the archive sketched in `e3115052` was then taken further under #3123
by `bd3ffad8` and `1311df15`.

The archive's (a) commit messages record the design that dev now carries: one
bounded 64 KiB workspace chunk reused between functions, verifier scratch separate
from algebraic scratch, promoted debug operands retained with the IR, typed
transform change results, and LICM and SROA analysis reclaimed per function.

## Group (f): landed

`e1885b82` carries the explicit phase outcomes (`src/lang/fail.mach`,
`doc/design/failure-kind.md`, the driver and sema consumers) plus diagnostic
ownership on top. The two follow-up fixes are present in their final form: the
typed reinference check in `lower/decl.mach` and the scaffold failure checks in
`sema/tests.mach`, and the editor buffer initialization the archive patched in
three places is one struct literal on dev.

## Group (e): the guards are in-tree, the drivers are one-shot

Each driver built A from the seed and B from A at a pinned archive SHA, ran the
middle-end suite in both profiles, then mutated one release site and asserted
that exactly one named test fails with a specific exit. The guards, their anchors
and the expected failure, all present on dev today:

| guard | mutation | expected |
|---|---|---|
| `mach.lang.be.codegen.scratch:module_images_survive_reclaimed_working_storage` | `fin { arena.dnit(?scratch); }` to `fin { }` in `be/codegen.mach` | exit 3 |
| `mach.lang.me.pipeline:simple_passes_do_not_retain_function_work_in_ir` | `verify_work` passes `m.alloc` instead of `alloc` | exit 10 |
| `mach.lang.me.pass.scalarize:rejected_work_does_not_retain_maps_in_ir` | `scalarize_work` allocates from `m.alloc` instead of `alloc` | exit 4 |
| `mach.lang.me.pass.scalarize:failed_instruction_releases_untransferred_operands` | delete the `A.deallocate[value.Value](m.alloc, owned, ...)` in the refused path | exit 3 |

The native runs that established these, all green: codegen 34005798043, optimizer
34008225851, transforms 34009898175, scalarize 34010965307 (ubuntu and windows
each). The drivers are not extracted: each pins `baseline` to an archive SHA and
triggers on a branch that no longer exists, and the mutation is a four-line recipe
the PR for this inventory re-ran against dev (results in the PR).

## Group (d): the measurements, preserved

The harness generated three synthetic families (`modules` at 10, 50, 150 modules
of 16 functions, `dense` with the same functions in one module, `aggregate` with a
4096, 16384 and 65536 byte record copied by value), built each in debug and release
at jobs 1 and 4 with output removed before every process, verified the executable's
printed checksum, ELF objects and jobs-1 versus jobs-4 image identity, and recorded
GNU time peak RSS plus a 20 ms `/proc` sampler on the child. Matched runs built two
compilers from two refs with the same seed and alternated them for three
repetitions over the same generated inputs. Seed `v4.26.5` throughout, ubuntu-latest,
one compiler process at a time with a process census before each.

Peak RSS in MiB and wall seconds, medians of three where n is three. Artifacts
expire 2026-12-05.

Self-build of the compiler source, debug profile unless stated:

| step | before | after | run |
|---|---|---|---|
| (b) codegen scratch, `ba8f1000` vs `5932eca2` | 2973.1 / 17.10 | 1419.5 / 17.25 | 34006203230 |
| (a) optimizer scratch, `5932eca2` vs `9af91635` | 1423.4 / 17.32 | 1185.1 / 17.29 | 34007743132 |
| (a) optimizer scratch, release | 3973.6 / 54.37 | 2555.3 / 54.17 | 34008565946 |
| (a) LICM and SROA, `298b0ac5` vs `bfdf7890` | 1182.8 / 17.46 | 1182.2 / 17.46 | 34010537722 |
| (a) LICM and SROA, release | 2559.8 / 54.78 | 2179.3 / 54.68 | 34010537722 |

Single-cell baselines, pre-scratch `feef0bc5` (run 34004956086) and after (b)
`1750f256` (run 34005747560), debug jobs 1 unless stated:

| workload | pre-scratch | after (b) |
|---|---|---|
| modules-150 | 87.2 / 0.31 | 58.3 / 0.47 |
| modules-150 jobs 4 | 87.3 / 0.21 | 58.3 / 0.31 |
| dense-150 | 87.9 / 0.92 | 82.9 / 1.07 |
| aggregate-65536 | 67.2 / 0.12 | 68.8 / 0.15 |
| seed to A | 3198.2 / 7.52 | 3216.6 / 9.26 |
| A to B | 2951.5 / 12.27 | 1420.2 / 17.12 |

Matched synthetic cells after (a), `5932eca2` vs `9af91635`, run 34007743132:

| workload | before | after |
|---|---|---|
| modules-150 debug jobs 1 | 58.1 / 0.48 | 52.1 / 0.49 |
| modules-150 release jobs 1 | 63.2 / 0.53 | 51.4 / 0.57 |
| dense-150 debug jobs 1 | 85.6 / 1.09 | 77.8 / 1.10 |
| dense-150 release jobs 1 | 86.3 / 0.48 | 75.2 / 0.48 |
| aggregate-65536 debug jobs 1 | 69.2 / 0.15 | 65.9 / 0.15 |

The many-module and self-build workloads moved. The large aggregate did not, which
is the #3109 bulk lowering item, tracked separately. All cells passed the checksum
and the jobs identity, and every self-build pair was byte-identical.

Three further scripts on fix/2299-memory-baseline (`compiler-memory-isolate.py`,
`compiler-memory-piece-compare.py`, `compiler-memory-matched-combined.py`, runs
34039970329, 34042979096, 34071499020) were a stage-by-stage bootstrap slowdown
investigation pinned to feat/3218 workloads `49fbbc48` and `2152b51d`. They belong
to the #3218 overhead log, not to scratch ownership, and are discarded here.

## What is extracted

The generator and runner from (d), de-pinned, as `test/memory.py`: the compiler
under test is an argument, an optional control compiler enables the alternating
matched mode, the checkout under test is the self-build workload, and provenance
records compiler digests, the checkout and std heads and the host. A
`workflow_dispatch` workflow builds the compilers from the seed and runs it. It is
not on the PR lane. The reproducible curves #2299 asks for come from running it
against the current compiler, not from these preserved numbers.
