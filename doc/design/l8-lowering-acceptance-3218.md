# L8 lowering acceptance matrix (#3219, #3218)

Acceptance pass for roadmap item L8 on feat/3218, recorded 2026-09-11. L7 lane
B and L3/L4 landed the lowering; this pass proves it complete and consistent
on every path the contract implies, fills every gap with a test, and fixes the
lowering defects the gaps exposed at the layer they belong to.

Contract: `doc/design/tagged-values.md`, "Case tests", "Payload places and
guards", "Reflection and conversion". Bounded acceptance: issue #3219.

Every runtime row is established by a freestanding fixture built for
linux-x86_64, linux-arm64 (qemu-aarch64) and linux-riscv64 (qemu-riscv64) in
both profiles through `ut_tag_native_all`, exiting 0 only when every check in
the fixture holds, so one fixture covers all six ISA/profile columns of its row.
The debug-trap rows are established on the IR itself: each fixture is lowered
under both pipelines and the number of `unreachable` blocks in its function is
asserted (the expected count in debug, zero in release), which is what
`--emit-ir` shows and what the L7 review did by hand for one fixture.

Legend: `L3`, `L4`, `L5`, `L7` name the lane whose test already covered the
cell before this pass; `L8` names a test added by this pass; `gap` marks a cell
with no test before this pass; `n/a` marks a cell the grammar cannot express.

## 1. `sel` on each place kind

| Place | Fixture form | Before | After |
| --- | --- | --- | --- |
| local | `sel r.value` | L7 `guarded_payload_reads_return_the_payload` | same |
| global | `sel g.value` | L3 storage (`g_init`) | same |
| record field | `sel b.r.value` | L3 storage (`Box`) | same |
| array index (runtime) | `sel a[i].value` | L3 storage | same |
| explicit deref | `sel (@p).value` | L3 storage (`bump`) | same |
| pointer auto-deref | `sel p.value` with `p: *T` | **gap: rejected** (`sel` expects a tag, but this place has type `*Reply`) | L8 `tag_sel_and_payload_places_auto_deref_a_pointer_on_every_native_backend`, sema `sel_auto_derefs_a_pointer_place` |
| descriptor | `sel v.[c]` | L4 agreement (`code_of`) | same |
| nested payload place | `sel o.inner.value` | L3 storage (tags in tags) | same |
| `$each` constant element | `sel t.value` with `t` over a constant array | **gap: lowering error** (address of a `$each` element) | L8 `tag_comptime_sel_and_payload_reads_agree_with_runtime_on_every_native_backend` |

The pointer auto-deref cell was ruled by the owner on 2026-09-10 (`sel p.value`
auto-derefs a pointer) and left for the guard lane by the L3 inventory. The
payload place `p.value` already dereferenced (record member access rules), so
only `sel` refused it; fixed in `infer_sel` (one pointer level, outer secrecy
checked on the pointee), in `open_exit_chain` (the chain's tag type is the
pointee's), in the whole-value assignment rule (a pointer place guards its
pointee, so reassigning the pointer or anything it hangs off is allowed, exactly
as `p = q` under `sel (@p).value` already was, while `b.r = ...` under
`sel b.r.value` with `b: *Box` is still refused) and in `lower_sel` (the
discriminator loads through the pointer value, as a member access does).

## 2. Payload access under each guard kind

Rows are guard kinds, columns are access kinds. Every cell is one runtime
fixture check on all six ISA/profile columns.

| Guard | read | write | `?` address |
| --- | --- | --- | --- |
| arm `if (sel P.c) {}` | L7 | L3 | L3 |
| exiting chain, `ret` | sema only (L7) → **gap at runtime** → L8 guards fixture | **gap** → L8 | **gap** → L8 |
| exiting chain, `brk` | sema only → **gap** → L8 | **gap** → L8 | **gap** → L8 |
| exiting chain, `cnt` | L7 (`item.some`) | **gap** → L8 | **gap** → L8 |
| `&&` right operand | compiled only (L7 profile test) → **gap at runtime** → L8 | n/a (assignment is a statement) | **gap** → L8 |
| nested guard `o.inner.value` | L3 | L3 | **gap** → L8 |
| three-case exiting chain (one case left) | sema only → **gap** → L8 | **gap** → L8 | **gap** → L8 |

L8 fixture: `ut_l8_guards_program`, test
`tag_payload_access_under_every_guard_kind_on_every_native_backend`.

## 3. Debug-profile discriminator trap per guarded access kind

Each row is lowered under both pipelines; the debug IR must carry exactly the
listed number of `unreachable` blocks (one per guarded payload access, each
reached only from a discriminator compare) and the release IR none. Before this
pass a single fixture had been inspected by hand.

| Access | Expected checks (debug) | Before | After |
| --- | --- | --- | --- |
| arm read | 1 | hand-inspected (L7 review) | L8 `tag_guarded_access_traps_in_debug_ir_and_not_in_release_ir` |
| arm write | 1 | gap | L8 |
| arm address | 1 | gap | L8 |
| exiting chain read (`ret`) | 1 | gap | L8 |
| exiting chain write (`cnt`) | 1 | gap | L8 |
| exiting chain address (`brk`) | 1 | gap | L8 |
| `&&` right-operand read | 1 | gap | L8 |
| `&&` right-operand address | 1 | gap | L8 |
| nested: `sel o.inner.value` + read | 3 (the inner `sel` checks `o.inner`, the read checks both levels) | gap | L8 |
| descriptor `r.[c]` read under `sel r.[c]` | 1 | gap | L8 |
| pointer auto-deref read | 1 | gap | L8 |
| global write | 1 | gap | L8 |
| index read | 1 | gap | L8 |
| field read | 1 | gap | L8 |
| aggregate payload copy | 1 | gap | L8 |
| three-case chain: `t.c` in an arm, `t.b` after the chain | 2 | gap | L8 |
| control: `sel` alone, no payload access | 0 | gap | L8 |

Runtime trap behaviour per access kind (the case switched through a pointer
between the guard and the access, trapping under debug and reading or writing
through under release on the three ISAs): before this pass only the arm read
was run (L7 `debug_profile_traps_a_guarded_read_after_the_case_changes`). L8
adds `tag_debug_trap_fires_for_every_guarded_access_kind_on_every_native_backend`
covering arm write, arm address, exiting-chain read, `&&` right-operand read,
the nested inner read and the pointer auto-deref read (argc 1 to 7 trap under
debug; under release argc 2 lands the write on the payload and every other
access reads through), plus a no-switch control that reads through under both.

## 4. Construction into each place kind

| Destination | Before | After |
| --- | --- | --- |
| local (init and replacement) | L3 storage | same |
| global (static initializer) | L3 static | same |
| global (runtime replacement) | L3 storage | same |
| record field | L3 storage | same |
| array element (runtime index) | L3 storage, L3 order | same |
| explicit deref `@p = T.c{}` | L3 storage | same |
| destination call `@pick() = T.c{}` | L3 order | same |
| descriptor head `T.[c]{}` | L4 agreement | same |
| payload of a nested construction | L3 storage (`Outer.inner{Reply.value{9}}`) | same |
| static initializer naming another constant (`val B: T = A;`, `A::T`, `Box{r: A}`, `[2]T{x, A}`, `ARR[1]`) | sema accepted, **lowering refused** ("must be a constant expression") | L8 `static_tag_initializers_fold_the_constant_they_name_on_every_native_backend` |

The construction rows themselves had no gap; the L3 inventory holds the
byte-level evidence. The static-initializer row is the defect the corpus test
of section 6 surfaced: a `val` copied from another constant aggregate was
accepted by sema (the canonical-tag identity-cast fixture) and refused by the
constant aggregate folder, which only knew literals.

## 5. Comptime evaluation

The comptime evaluator had no arm for `sel` at all: any `sel` inside a `$if`
condition failed with an internal error from the gate dependency visitor
("this expression kind has no entry in the compile-time dependency visitor"),
exit 2, no location. Payload reads on constant tags fell through to the
module-member path and were "not a constant".

| Cell | Before | After |
| --- | --- | --- |
| `sel C.c` on a module `val` constructed with `T.c{...}` | **gap: internal error** | L8 comptime fixture + sema tests |
| `sel t.c` on a `$each` element of a constant array | **gap** | L8 |
| `sel C.[c]` through a case descriptor | **gap** | L8 |
| payload read `C.c` under a guard, scalar payload | **gap** | L8 |
| payload read through a nested constant `O.inner.value` | **gap** | L8 |
| `$if (sel C.c)` arm guards `C.c` in its block | **gap** (no guard opened by a comptime arm) | L8 |
| `sel` on a non-constant place (param, local, module `var`, a constant of another module) | **gap: internal error** | located diagnostic, L8 sema tests `comptime_sel_tests_constant_tags` and driver `comptime_sel_on_a_mutable_or_foreign_place_is_reported_at_the_condition` |
| payload read of the case a constant does not hold | **gap** | located diagnostic, L8 sema test |
| a constant declared after the gate that tests it | **gap** | L8 sema test (bound before any body is bound) |
| runtime `sel` and payload read over a `$each` element | **gap: lowering error** | fold, L8 parity fixture |
| runtime and comptime twins agree | **gap** | L8 `tag_comptime_sel_and_payload_reads_agree_with_runtime_on_every_native_backend` |

Mechanism: a module `val` whose initializer is a case literal is bound in the
module's comptime context as a constant element (the same `CT_KIND_CONST_ELEM`
the `$each` machinery uses) by name resolution, after every declaration is
collected and before any body is bound (the literal head `T.c` is a dotted
path until the tag symbol is known, so the loader cannot recognize it), then
rebound with its checked type by type checking and bound again by lowering for
the gates it decides. A constant element points into its module's syntax tree,
so it is never exported: a constant tag of another module is not a comptime
place, and `sel` on one is rejected like any non-constant place. `sel` evaluates its place to a constant
element, requires that element to be a case literal, and compares the literal's
head case with the tested case, by name or through a descriptor. A payload read
on a constant element that is a case literal yields the literal's payload when
the case matches, returns a nested constant element when the payload is itself a
literal, and is rejected when the case differs, which is the comptime analogue
of the debug trap: comptime state is never stale, so the mismatch is a
compile-time error rather than undefined behaviour. A comptime `sel` on
anything that is not a constant element is rejected with a located diagnostic
at the condition, and a gate that reaches lowering still undecided is reported
at its condition instead of failing internally. A `$if` arm whose condition is
exactly `sel P.c` guards `P.c` in its body, the same rule as a chain arm.

## 6. Profile agreement at scale

Before: six fixtures (L7 `tag_guards_accept_and_reject_identically_across_profiles`).
After: every tag fixture the suite carries. The sema-level fixtures are moved
into one corpus table (`ut_tag_corpus`) that the per-topic sema tests index by
range, so each fixture has one spelling, and
`tag_fixture_corpus_accepts_and_rejects_identically_across_profiles` builds
every row through sema, lowering and codegen under both pipelines and requires
the debug verdict, the release verdict and the expected verdict to agree. The
native fixtures are already built under both profiles by `ut_tag_native_all`.

## Gap count

Cells: 9 `sel` places, 18 guard-by-access cells (one unexpressible), 17 IR trap
rows, 7 runtime trap rows, 10 construction rows, 10 comptime rows and the
profile-agreement row.

| | cells | gaps |
| --- | --- | --- |
| before | 71 | 49 |
| after | 71 | 0 |

## Defects found and fixed

Each with the mutation control that shows its test failing without the fix.

- **`sel p.c` through a pointer** (dcce8cbbb). `infer_sel` refused a pointer
  place while the payload place accepted it. Fixed at sema (`infer_sel`,
  `open_exit_chain`, the assignment cover rule) and lowering (`lower_sel`).
  Mutation: reverting the `infer_sel` dereference makes
  `tag_sel_and_payload_places_auto_deref_a_pointer_on_every_native_backend`
  and `sel_auto_derefs_a_pointer_place` fail.
- **No comptime `sel`** (007cb2eea). Any `sel` in a `$if` condition was an
  internal error from the gate dependency visitor. Fixed in `comptime.mach`
  (evaluation, visitor, constant-element payload reads), `resolve.mach`
  (constant binding before bodies), `sema.mach` (typed rebind, `$if` arm
  guards), `lower/context.mach` (located gate failure) and `lower/expr.mach`
  (`sel` over a `$each` element folds). Mutation: removing the
  `EXPR_KIND_SEL` evaluation arm makes `comptime_sel_tests_constant_tags` and
  the comptime parity fixture fail.
- **A static initializer naming a constant aggregate** (b1d0db700). Found by
  the corpus test: sema accepts `val r2: T = r;` and `r::T`, the constant
  aggregate folder refused them. Fixed in `constagg.mach` through
  `constant_literal_expr`. Mutation: bypassing the resolution in
  `try_const_aggregate` makes
  `static_tag_initializers_fold_the_constant_they_name_on_every_native_backend`
  fail and the corpus test disagree on the identity-cast row.

Mutation results are recorded in `migration-compiler-3218.md` with the run
they were taken from.
