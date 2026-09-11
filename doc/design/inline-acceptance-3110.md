# Inlining acceptance inventory for #3110

Roadmap item N6, phase 1. One row per acceptance bullet of #3110: what `dev`
does at the start of this lane (`853d4c17`) with the test that shows it, what
the three candidate commits on `archive/3218-candidate-era` add, and a verdict.
The compiler policy is covered here with local fixtures shaped like the eight
`std.sync.atomic` wrappers (short asm bodies with volatile memory effects); the
std-side annotations are Track C's S4 and are not part of this lane.

Candidate commits, in the order they are ported:

- `c2fbd635` "fix(compiler): integrate inline bodies and final security
  provenance". A 64-file integration commit. The hunks that belong to this lane
  are `src/lang/me/ir/body.mach` (new), `src/lang/me/pass/inline.mach`,
  `src/lang/me/ir.mach` (`Function.body_module`), `src/lang/me/pipeline.mach`,
  `src/lang/me/lower/expr.mach` (`stamp_body_provider` replacing
  `enqueue_inline_import`), `src/lang/driver.mach`, `src/lang/driver/project.mach`,
  `src/lang/driver/passes.mach` (`Q_INLINE_BODIES`), `src/lang/query.mach`, the
  `inline` section of `doc/language/decorators.md`, and the two driver tests
  `inline_bodies_cross_modules_preserve_identity_effects_and_owned_metadata` and
  `inline_body_availability_refreshes_after_unannotated_provider_edits`.
- `602cf806` "fix(inline): retain actual providers for shared weak
  specializations".
- `d2a9842d` "fix(inline): make body cost independent of debug annotations so
  -g cannot move an inlining decision".

## The two designs

`dev` (`195c81436`) flows a dependency body into an importer only when the
function is a template (generic, comptime-param, pack-tailed) or carries
`#[inline]`; the copy is monomorphized at lowering as `PUB | WEAK` under the
origin's mangled name, and `Q_LOWERED_SURFACE` includes `#[inline]` bodies so
an edit invalidates importers. A regular small `pub fun` in another module
(`vec3`, `aabb_add`, `fold.mix_u64`, every `std.sync.atomic` wrapper) stays an
out-of-line call: `should_inline` refuses `FN_FLAG_EXTERN` unconditionally.

The candidate replaces that with a query product. `Q_INLINE_BODIES` (one per
module, keyed like `Q_LOWER`) is the provider's raw lowered module after
`mem2reg`, reduced by `body.extract` to the functions `body.candidate` admits:
not extern, noinline, scalar or naked, not in a call cycle, and either
`#[inline]` or fewer than `SMALL_INSTRUCTIONS` (25) live instructions, capped at
`MAX_INSTRUCTIONS` (1024). When an importer lowers in release, `acquire_inline_bodies`
reads that product for every extern function whose `body_module` names another
module (stamped at lowering by `stamp_body_provider`), copies the body into a
`body.Available` store owned by that lowering, and `pipeline.inline_available`
attaches those bodies onto the importer's extern declarations for the duration
of the inline pass. Nothing is emitted for the copy: an inlined call disappears,
a remaining call or a taken address still names the provider's strong symbol,
and the store is released with the lowering. `#[inline]` is no longer a
monomorphization axis, so `Q_LOWERED_SURFACE` no longer carries its body; the
invalidation edge is the `Q_INLINE_BODIES` read.

Resolutions against `dev` made while porting:

- `Q_INLINE_BODIES` is registered with `register_derived_equatable` and
  `lower_output_equal`, the F2 contract for IR-module products
  (`doc/design/query-semantic-results.md`), so an edit to the provider that
  leaves every extracted body identical stops propagation at the product
  instead of re-optimizing every importer.
- `body.mach` is written against `dev`'s opaque `IRT_PTR` (the candidate's
  `pointee` field is the SPIR-V logical-addressing hunk owned by N4) and
  without `IRT_TAG`, which `dev` does not have.
- `lower_parallel_enabled` and the `slot_len <= 1` gate keep `dev`'s shape; the
  candidate's change to run the prepared path unconditionally came with its
  readout test rewrite and is not part of the inline concern.

Not taken from `c2fbd635` (owned by other lanes): the SPIR-V logical
addressing hunks (`ir/type.mach` pointee, `ir/printer.mach`, `ir/verify.mach`
`reference_valid`, `lower/context.mach` `LogicalTypePath`, `vecform.mach`,
`target/abi/spirv.mach`, `target/isa/spirv/*`), the RISC-V extension hunks
(`ct.mach` `op_known`/`CT_CAP_UNKNOWN` and its `asm.mach` consumer,
`fe/sema/check.mach`, `target/isa/riscv/*`, `target/isa.mach`, `target.mach`),
the linker and object-format hunks (`be/linker*.mach`, `be/obj.mach`,
`target/of/*`, `build/cache/image.mach`, `publication/testing.mach`
`retain_bytes`), the codegen hunks (`be/codegen/*`), `doc/cli.md`,
`doc/language/asm.md`, `doc/language/fun.md`, the `dep/std` and `mach.toml`
bumps, the corpus case edits and `test/golden/spirv/SKIPS`, and the
`tests.mach` hunks outside the inline tests (`ut_single_result_n`,
`ut_vec_diag*`, readout, SPIR-V and RISC-V tests).

## Per bullet

### Same-module elimination

`dev`: `me/pass/inline.mach` at `OPT_RELEASE` inlines a direct call when the
callee is `#[inline]`, or is the single caller's private helper (not
address-taken, not exported), or has fewer than 25 non-debug instructions and
has been duplicated fewer than `INLINE_SMALL_CALLEE_FANOUT_CAP` (16) times.
Shown by `inline.run:small_single_use_callee_inlines_by_default`,
`inline.run:small_high_fanout_callee_stops_fully_inlining` and
`driver:generic_instance_call_inlines_at_release`, all of which count `OP_CALL`
in IR. No test on `dev` counts `call` in the emitted machine text.

Candidate: no policy change for the same-module case; `callee_instr_count` now
reads `body.live_instructions`.

Verdict: accepted on `dev` at the IR level. Phase 1 adds a machine-text check:
`driver:inline_same_module_helper_leaves_no_call_in_x86_64_text` counts
`call` mnemonics in the x86-64 assembly `codegen_module` writes.

### Cross-module elimination

`dev`: only `#[inline]` bodies flow, as weak monomorphized copies. Shown by
`driver:incremental_lower_inline_body_invalidates_importers` (invalidation
only; the int cases `2231-cross-module-inline-*` from `195c81436` were stripped
with `int/`). An unannotated small helper in another module keeps its call.

Candidate: `Q_INLINE_BODIES` plus `body.Available` as described above. Shown by
`driver:inline_bodies_cross_modules_preserve_identity_effects_and_owned_metadata`:
`main -> helper.wrap -> helper.first -> leaf.bump` folds to `ret 42` with zero
`OP_CALL` in `main`, across a two-hop chain of unannotated helpers, with
`jobs = 0` and `jobs = 2`.

Verdict: gap on `dev`, closed by the port. Phase 1 adds the machine-text
check `driver:inline_cross_module_helper_leaves_no_call_in_x86_64_text`.

### Bounded growth

`dev`: `INLINE_BUDGET` (1024) counts inlining events per caller, not
instructions, so one caller may grow by 1024 x 24 instructions; `PEEL_BUDGET`
likewise. Shown by `inline.run:exhausted_inline_budget_is_a_distinct_reported_state`
and `inline.run:a_later_function_is_not_starved_by_an_earlier_functions_exhaustion`.

Candidate: `body.Budget` charges live instructions (`MAX_INSTRUCTIONS` 1024)
and owned bytes (`MAX_BYTES` 256 KiB) per caller during inlining, per import
set during acquisition, and per provider during extraction; a refused charge is
not recorded. The candidate bar `SMALL_INSTRUCTIONS` (25) is the documented
threshold below which an unannotated helper is eligible.

Verdict: accepted with the port. Phase 1 adds
`driver:inline_helper_at_the_size_threshold_keeps_its_call` (a 25-live-instruction
helper keeps its call, a 24-instruction one does not) with a mutation control
on the threshold.

### Recursion, indirect calls, `noinline`, unsupported asm

`dev`: recursion is detected per function by a BFS from each root
(`reaches_self`), self-recursion is peeled up to `PEEL_LEVELS` (2); an indirect
call (callee not a `VAL_FN`) is never a candidate; `noinline`, `naked` and
`scalar` are refused. Shown by `inline.mark_recursive:direct_self_call`,
`inline.run:mutual_recursion_not_peeled`, `inline.run:noinline_flag_blocks_inlining`,
`inline.run:naked_flag_blocks_inlining` and
`driver:noinline_decorator_survives_object_all_isas`. There is no asm-specific
refusal: an asm block is cloned with its `IrAsm` payload (`ir.clone_asm_payload`)
and the encode-time checks (ISA tag mismatch, stack-pointer write under a
`{name}` binding, unknown mnemonic) apply at the inlined site exactly as at the
original; a body whose last statement does not fall through lowers to
`unreachable` after the block (`stmt.mach asm_body_returns`) and inlines as such.

Candidate: `mark_recursive` is one Tarjan traversal (`cycles_exclude_callers_and_completed_components`),
extraction skips every member of a call cycle, `peel_eligible` refuses
`noinline`, `scalar` and `naked` (extension of `inline.run:peels_direct_self_recursion`),
and an extern-attached body is never treated as a free single-caller inline.

Verdict: accepted with the port; the rule is documented in
`doc/language/decorators.md`. Phase 1 adds one driver test over the four cases
with a control per guard: `driver:inline_rule_keeps_recursive_indirect_noinline_and_naked_calls`.

### Effects and order

`dev`: `OP_ASM` is neither discardable, CSE-safe, movable nor speculatable and
may call, read and write memory (`ir/opdesc.mach`); the `{name}` effect model
(#2258) decides what the block clobbers at codegen. An inlined asm keeps its
payload, and instruction order within the cloned blocks is the callee's order.
Shown by `inline.run:*` cloning tests and `ctvalidate.opaque:inline_asm_forbidden`.

Candidate: `copy_function` carries `asm_blocks` and `inline_sites` across the
module boundary; `clone_instructions` re-owns constant payloads for an
extern-attached body. Shown by the `assembly` (one `OP_ASM`, zero `OP_CALL`,
`asm_blocks.len == 1`) and `touch` (one `OP_STORE` to the provider's global,
which stays `is_extern` and not `is_local`) legs of the cross-module test.

Verdict: accepted with the port. Phase 1 adds
`driver:inline_effectful_asm_wrapper_keeps_its_effects_and_order` (an atomic-shaped
wrapper inlined cross-module: the asm survives the release pipeline with its
result unused, two identical wrappers are not merged, and the MIR instruction
keeps its `MirAsm` payload) and `driver:inline_naked_asm_wrapper_is_refused`.

### Secrecy

`dev`: `secret` on values and instructions survives cloning
(`inline.run:preserves_secret_param_operand`); `oblivious.check_module` runs
before the pipeline and rejects a non-`#[oblivious]` function that computes on a
secret; `ctvalidate` validates only functions whose MIR carries `oblivious`.
Nothing stops the inliner from moving an `#[oblivious]` callee's computation
into a caller that is not `#[oblivious]`, where `ctvalidate` never looks at it.

Candidate: `own_constant` and `copy_value` keep `secret`; the cross-module test
checks that an `#[oblivious]` importer inlining an `#[oblivious]` provider keeps
the secret `OP_ADD` and its flag. The caller-side hole above is untouched.

Verdict: gap on both. Phase 1 closes it in the inline pass: an `#[oblivious]`
callee is inlined only into an `#[oblivious]` caller, so the validation domain a
function declared is the one its instructions are validated in.
`driver:inline_oblivious_callee_stays_a_call_in_a_public_caller` with a
mutation control.

### Debug mapping

`dev`: `stamp_inline_metadata` records an `InlineSite` per inlined call with
the call location and parent site; DWARF consumes it
(`dwarf.produce:nested_inline_debug_build_survives_real_inlining`,
`inline.stamp_inline_metadata:carries_alloca_debug_metadata_into_the_caller`).
`callee_instr_count` ignores `OP_DBG_VALUE` (`callee_instr_count:dbg_value_blind`)
but the budget counted events, and phis and terminators were counted.

Candidate: `copy_function` carries debug variables, expressions, alloca debug
variables and inline sites across modules (the `assembly` leg asserts a nonzero
`inline_site_len`). `d2a9842d` makes every cost read one `counted()` view that
excludes `OP_DBG_VALUE` and dead arena slots (`body:debug_annotation_does_not_move_a_bodys_cost`),
so `-g` cannot change an inlining decision.

Verdict: accepted with the port. Phase 1 adds
`driver:release_text_is_byte_identical_with_and_without_g_at_the_inline_threshold`
over a fixture whose callee sits one instruction under the bar with a `dbg_value`
that would push it over.

### Cache and query dependency invalidation

`dev`: an `#[inline]` body is part of `Q_LOWERED_SURFACE`, so its edit
invalidates importers (`driver:incremental_lower_inline_body_invalidates_importers`);
an unannotated body is firewalled (`driver:incremental_lower_surface_firewall`).

Candidate: `Q_INLINE_BODIES` is read through `query.get` inside the importer's
`Q_LOWER` compute, so the edge is recorded by reading. Shown by
`driver:inline_body_availability_refreshes_after_unannotated_provider_edits`:
a helper that was ineligible (it names a module-local literal pool) becomes
eligible and then changes, and the importer's `Q_LOWER` revision moves each
round while the provider's surface does not.

Verdict: accepted with the port, with the equatable registration above. The
`dev` test is retargeted: the surface no longer moves on an `#[inline]` body
edit and the importer still re-lowers. Phase 1 adds the early-cutoff control
`driver:inline_body_product_stops_propagation_when_extracted_bodies_are_equal`.

### Pure versus atomic controls

`dev`: there is no purity model in the inliner or the pipeline; an inlining
decision reads size and flags only, and after inlining every later pass reads
`ir/effect.mach`, where `OP_ASM` is never discardable, CSE-safe or movable.
Nothing classifies a short asm body as pure.

Candidate: unchanged.

Verdict: accepted on `dev`. The controls are the effect legs above: an inlined
atomic-shaped wrapper whose result is unused is not removed and two identical
ones are not merged.

### The eight atomic wrappers

`std.sync.atomic` at the pinned std (`168a9f76`, 1.0.1) defines `load`,
`store`, `cas`, `fetch_add`, `fetch_sub`, `exchange`, `fence` and `spin_hint`
as unannotated `pub fun` bodies of one `asm` block under `$if` on the build
arch, each well under 25 live instructions after `mem2reg`. Under the ported
policy every one is extracted into `Q_INLINE_BODIES` and inlined at its
importers with no std-side annotation; `#[inline]` would only override the
size bar, which they clear. The local fixtures in the phase 1 tests mirror
their shape (a `{ptr}`/`{result}` binding pair around one memory instruction).

Verdict: compiler policy covered by the effect legs above; the std-side
annotations remain Track C's S4 and are not needed for elimination.
