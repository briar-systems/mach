# Migration compiler candidate (#3218, #3219)

The record L10 and S1 consume: the exact feat/3218 state the L8 lowering
acceptance was run on, what built it, and what it proved. It is a document,
not a tag; the owner tags at release.

## The candidate

| | |
| --- | --- |
| branch | feat/3218 with the L8 commits of `feat/3218-l8` on top (see below) |
| acceptance run on | commit `94b834983e632bfff3c46d28eac282ea7f75986d` (`feat/3218-l8`, L8 test commit; the L8 fix commits are dcce8cbbb, 007cb2eea, b1d0db700) |
| feat/3218 base | `872e1c6f` (L2, L3, L4, L5, L7 landed) |
| std pin | `168a9f760d7c0f7a182f3b0685081e62f1a4f682` at `dep/std` (the old pin; the std 2.0.0 migration is S1's) |
| seed that built it | Mach 4.30.0 at `/home/octalide/.local/bin/mach`, sha256 `29b9264cfd5477419a190a8d6649e9ae8302744d935991ac8ffc47f2b4c26535` |
| host | linux x86_64, natively; aarch64 and riscv64 under qemu-user |

The compiler's own source uses no v5 syntax, so the 4.30.0 seed builds it.
Every `sel` identifier in the compiler and std trees was renamed before the
keyword was reserved (L2).

## Fixpoint

| generation | built by | sha256 |
| --- | --- | --- |
| A | the 4.30.0 seed | `3db5ad4b9c93d6f041dc16a3eee9b2df2da75d74ccba1f99a10e13d4c5cb1375` |
| B | A | `a583def71e98c87a8202cbb2f1db70704239ee60fcb9fe5210a133471f17f2c5` |
| C | B | `a583def71e98c87a8202cbb2f1db70704239ee60fcb9fe5210a133471f17f2c5` |

`cmp B C`: byte-identical. A differs from B because the seed's code generation
differs from the candidate's; the invariant is B == C (three generations, as
the fin-semantics work established).

## What the acceptance proved

Numbers as measured with generation A on the commit above.

| check | result |
| --- | --- |
| unit suite (`mach test . --jobs 8`) | 2809 passed, 0 failed (2800 before L8; nine tests added, one replaced) |
| `sh test/census.sh` | every census ok |
| corpus layer B, x86_64-linux | 91 cells pass (o2), 0 fail, 0 skip |
| corpus layer B, aarch64-linux | 91 cells pass (o2), 0 fail, 0 skip |
| corpus layer B, riscv64-linux | 91 cells pass (o2), 0 fail, 0 skip |
| link leg, x86_64-linux | 138 pass / 0 fail / 0 skip over 138 cells (debug and release) |

Layer B goldens: none moved (273 cells over the three targets compare equal to the blessed goldens; `test/golden` is untouched), so no lowering change reached the corpus's codegen surface.

The acceptance matrix itself, cell by cell, is `l8-lowering-acceptance-3218.md`.

## Tests added by L8

- `mach.lang.driver:tag_fixture_corpus_accepts_and_rejects_identically_across_profiles`
  (replaces the six-fixture `tag_guards_accept_and_reject_identically_across_profiles`;
  187 rows through the whole back half under both pipelines)
- `mach.lang.driver:comptime_sel_on_a_mutable_or_foreign_place_is_reported_at_the_condition`
- `mach.lang.driver:tag_guarded_access_traps_in_debug_ir_and_not_in_release_ir`
- `mach.lang.driver:static_tag_initializers_fold_the_constant_they_name_on_every_native_backend`
- `mach.lang.driver:tag_sel_and_payload_places_auto_deref_a_pointer_on_every_native_backend`
- `mach.lang.driver:tag_payload_access_under_every_guard_kind_on_every_native_backend`
- `mach.lang.driver:tag_comptime_sel_and_payload_reads_agree_with_runtime_on_every_native_backend`
- `mach.lang.driver:tag_debug_trap_fires_for_every_guarded_access_kind_on_every_native_backend`
- `mach.lang.fe.sema:sel_auto_derefs_a_pointer_place`
- `mach.lang.fe.sema:comptime_sel_tests_constant_tags`

## Mutation controls

One per lowering defect L8 fixed; each run rebuilds the compiler with the fix
removed and runs the suite.

| mutation | fix removed | suite result |
| --- | --- | --- |
| m1 | the pointer dereference in `infer_sel` (dcce8cbbb) | 2804 passed, 5 failed |
| m2 | the `EXPR_KIND_SEL` arm of the comptime evaluator (007cb2eea) | 2805 passed, 4 failed |
| m3 | the constant-literal resolution in `try_const_aggregate` and `place` (b1d0db700) | 2807 passed, 2 failed |

Every mutated compiler built and the suite otherwise held, so each failure set
is the tests that own that fix. The source was restored after each run
(`git status` clean apart from these documents).

## Known limits carried forward

- A constant tag is comptime-visible only inside its own module: the constant
  element points into the module's syntax tree, so it is never exported, and
  `sel lib.C.value` at compile time is rejected as a non-constant place. A
  runtime `sel` on an imported constant is unaffected.
- A nested record payload's omitted fields read at compile time need the
  constant's checked type, which name resolution does not have: a module `val`
  gets it from type checking's rebind, so this only bites a gate decided during
  name resolution.
- Darwin has no lane on this host; the RV32 forms have no execution engine.
