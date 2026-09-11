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
- `body.mach` is written without `IRT_TAG`, which `dev` does not have. The
  `IRT_PTR` pointee the candidate carried arrived on `dev` with #3268 (N4)
  while this lane was open and is handled as the candidate did: a typed
  reference is marked, remapped and re-interned through `intern_reference`.
- `lower_parallel_enabled` and the `slot_len <= 1` gate keep `dev`'s shape; the
  candidate's change to run the prepared path unconditionally came with its
  readout test rewrite and is not part of the inline concern.
- The candidate's `Q_LOWER` compute recorded no dependency on the module's own
  typed definition when it took the raw module `Q_INLINE_BODIES` had captured:
  the definition read lived inside the raw lowering, which the capture skips.
  Under the owned registration every recompute of the bodies product advanced
  its revision and hid that; under the equatable one an edit to a provider
  function that is not extracted left the provider's own `Q_LOWER` reused with
  the old body. `lower_inputs` now records the definition and target reads on
  every product built from a raw lowering, capture or not
  (`driver:inline_body_product_stops_propagation_when_extracted_bodies_are_equal`
  fails without it).

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
`inline.run:helper_at_the_size_bar_keeps_its_call` (a two-caller helper of 24
live instructions loses both calls, one of 25 keeps both; mutation control:
`<` to `<=` on the bar inlines the 25-instruction body) and the `bg` leg of the
rule test below, a 27-instruction provider body that keeps its call across
modules.

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
`doc/language/decorators.md`. Phase 1 adds one cross-module driver test over
every case beside a small direct helper that does lose its call:
`driver:inline_rule_keeps_recursive_indirect_noinline_naked_and_large_calls`
(a self-recursive provider, a call through a function-pointer parameter whose
callee operand is not a `VAL_FN`, a `noinline` provider, a `naked` asm provider
and a body over the bar each keep exactly one call).

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
`driver:inline_effectful_asm_wrapper_keeps_its_effects_and_order`: `load`,
`store` and `fence` shaped like the std wrappers, inlined across modules; two
identical loads stay two `OP_ASM`, a load whose result is unused survives the
release pipeline, store/fence/store keep their order in IR and at MIR, and each
MIR asm keeps its `MirAsm` payload with both `{name}` bindings. The `naked`
refusal is the `nkd` leg of the rule test.

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
function declared is the one its instructions are validated in. Same-module
`inline.run:oblivious_callee_inlines_only_into_an_oblivious_caller` and
cross-module `driver:inline_oblivious_callee_stays_a_call_in_a_public_caller`;
mutation control: dropping the flag check in `should_inline` inlines into the
public caller and fails both.

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
`driver:release_text_is_byte_identical_with_and_without_g_at_the_inline_bar`:
a provider body of 23 live instructions with eleven locals, whose `-g` build
adds a `dbg_value` per local, is inlined at both call sites with and without
`-g` and the importer's `.text` bytes are identical; mutation control: counting
`OP_DBG_VALUE` in `body.counted()` keeps the calls under `-g` and the texts
differ.

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

Verdict: accepted with the port, with the equatable registration and the
`lower_inputs` fix above. The `dev` test is retargeted: the surface no longer
moves on an `#[inline]` body edit and the importer still re-lowers. Phase 1
adds `driver:inline_body_product_stops_propagation_when_extracted_bodies_are_equal`:
an edit to a `noinline` provider function moves the provider's `Q_LOWER` and
not the importer's, and an edit to the extracted helper moves both.

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

## Corpus goldens moved by this change

Layer B compares `llvm-objdump` text of the `o2` build against
`test/golden/<target>/<group>/<case>.dis`. Every corpus case folds its operands
through `corpus.lib.fold`, whose `mix_*` entry points are small unannotated
helpers in another module, so cross-module inlining reaches every case: each
fold site now carries the byte loop inline instead of a call. The evidence
below follows `doc/design/corpus-goldens-3218.md`: cases are classified by the
first rule that applies (fewer calls, then a smaller first frame adjustment
with no call removed, then a larger one, then an identical instruction
multiset with frame offsets normalized, then other), calls are `call*` on
x86-64, `bl`/`blr` on aarch64 and `jal`/`jalr` on riscv64, and the frame
adjustment is the first `subq $N, %rsp`, `sub sp, sp, #N` or `addi sp, sp, -N`. Computed by comparing each blessed golden
with its `HEAD` version.

## x86_64-linux: 91 goldens changed

| classification | cases |
| --- | --- |
| calls removed | 91 |

calls before 4304, after 1791, cases reaching zero calls 31
frame direction: frame larger 37, frame same 44, frame smaller 10

| golden | lines removed | lines added | calls before | calls after | class |
| --- | --- | --- | --- | --- | --- |
| bits/logic_u32 | 65 | 322 | 22 | 5 | calls removed |
| bits/logic_u64 | 61 | 337 | 22 | 5 | calls removed |
| bits/shift_i32 | 68 | 366 | 28 | 11 | calls removed |
| bits/shift_i64 | 39 | 320 | 28 | 11 | calls removed |
| bits/shift_u32 | 67 | 357 | 27 | 10 | calls removed |
| bits/shift_u64 | 44 | 315 | 27 | 10 | calls removed |
| call/call_chain | 199 | 460 | 172 | 154 | calls removed |
| call/call_float_regs | 281 | 483 | 87 | 30 | calls removed |
| call/call_indirect | 303 | 447 | 69 | 24 | calls removed |
| call/call_int_regs | 415 | 557 | 123 | 24 | calls removed |
| call/call_mixed | 283 | 551 | 53 | 35 | calls removed |
| call/call_rec_edge | 335 | 398 | 87 | 50 | calls removed |
| call/call_rec_large | 207 | 653 | 134 | 97 | calls removed |
| call/call_rec_small | 306 | 705 | 71 | 30 | calls removed |
| call/call_recursive | 240 | 248 | 53 | 13 | calls removed |
| call/call_ret_large | 489 | 1162 | 138 | 96 | calls removed |
| call/call_ret_small | 372 | 754 | 88 | 42 | calls removed |
| call/call_variadic | 140 | 551 | 158 | 141 | calls removed |
| cmp/branch_nest | 97 | 131 | 3 | 0 | calls removed |
| cmp/cmp_i32 | 106 | 210 | 7 | 0 | calls removed |
| cmp/cmp_i64 | 109 | 206 | 7 | 0 | calls removed |
| cmp/cmp_u32 | 106 | 210 | 7 | 0 | calls removed |
| cmp/cmp_u64 | 109 | 206 | 7 | 0 | calls removed |
| cmp/loop_shapes | 95 | 185 | 6 | 0 | calls removed |
| cmp/short_circuit | 360 | 348 | 36 | 11 | calls removed |
| comptime/ct_runtime_agree | 175 | 405 | 54 | 37 | calls removed |
| convert/bitcast | 46 | 215 | 11 | 0 | calls removed |
| convert/ext_sign | 70 | 307 | 15 | 0 | calls removed |
| convert/ext_zero | 70 | 305 | 15 | 0 | calls removed |
| convert/f2f | 57 | 224 | 11 | 0 | calls removed |
| convert/f2i | 125 | 735 | 54 | 21 | calls removed |
| convert/f2u_high | 172 | 579 | 25 | 0 | calls removed |
| convert/i2f | 101 | 702 | 37 | 4 | calls removed |
| convert/trunc | 46 | 183 | 9 | 0 | calls removed |
| float/arith_f32 | 437 | 405 | 65 | 46 | calls removed |
| float/arith_f64 | 380 | 366 | 65 | 46 | calls removed |
| float/cmp_f32 | 259 | 459 | 20 | 2 | calls removed |
| float/cmp_f64 | 311 | 709 | 26 | 7 | calls removed |
| float/special_f32 | 483 | 618 | 117 | 84 | calls removed |
| float/special_f64 | 511 | 652 | 127 | 94 | calls removed |
| frame/frame_align | 463 | 522 | 79 | 24 | calls removed |
| frame/frame_large | 193 | 432 | 16 | 0 | calls removed |
| frame/frame_spill | 149 | 373 | 32 | 15 | calls removed |
| imm/imm_add | 66 | 332 | 25 | 8 | calls removed |
| imm/imm_addr | 175 | 491 | 21 | 4 | calls removed |
| imm/imm_logic | 90 | 363 | 21 | 4 | calls removed |
| imm/imm_mov | 81 | 268 | 12 | 0 | calls removed |
| mem/addr_modes | 299 | 517 | 36 | 19 | calls removed |
| mem/array_index | 358 | 664 | 36 | 15 | calls removed |
| mem/global_data | 160 | 436 | 35 | 12 | calls removed |
| mem/global_zero | 210 | 492 | 28 | 7 | calls removed |
| mem/loadstore | 107 | 375 | 25 | 8 | calls removed |
| mem/ptr_arith | 233 | 491 | 29 | 8 | calls removed |
| mem/rec_layout | 473 | 880 | 50 | 25 | calls removed |
| mem/uni_layout | 216 | 493 | 32 | 15 | calls removed |
| scalar/arith_i16 | 79 | 299 | 13 | 0 | calls removed |
| scalar/arith_i32 | 87 | 301 | 13 | 0 | calls removed |
| scalar/arith_i64 | 68 | 285 | 13 | 0 | calls removed |
| scalar/arith_i8 | 79 | 299 | 13 | 0 | calls removed |
| scalar/arith_u16 | 77 | 297 | 13 | 0 | calls removed |
| scalar/arith_u32 | 85 | 299 | 13 | 0 | calls removed |
| scalar/arith_u64 | 66 | 283 | 13 | 0 | calls removed |
| scalar/arith_u8 | 77 | 297 | 13 | 0 | calls removed |
| scalar/divrem_i32 | 112 | 357 | 13 | 0 | calls removed |
| scalar/divrem_i64 | 91 | 285 | 13 | 0 | calls removed |
| scalar/divrem_u32 | 82 | 259 | 10 | 0 | calls removed |
| scalar/divrem_u64 | 70 | 230 | 10 | 0 | calls removed |
| scalar/mul_i32 | 56 | 184 | 8 | 0 | calls removed |
| scalar/mul_i64 | 41 | 164 | 8 | 0 | calls removed |
| scalar/mul_u32 | 54 | 182 | 8 | 0 | calls removed |
| scalar/mul_u64 | 38 | 160 | 8 | 0 | calls removed |
| vec/autovec_loop | 502 | 550 | 10 | 0 | calls removed |
| vec/vec_cmp_select | 301 | 534 | 169 | 109 | calls removed |
| vec/vec_f32x2 | 265 | 217 | 37 | 16 | calls removed |
| vec/vec_f32x3 | 310 | 273 | 33 | 9 | calls removed |
| vec/vec_f32x4 | 413 | 273 | 71 | 16 | calls removed |
| vec/vec_f32x5 | 1225 | 945 | 88 | 16 | calls removed |
| vec/vec_f32x8 | 1639 | 1142 | 139 | 16 | calls removed |
| vec/vec_f64x2 | 258 | 210 | 37 | 16 | calls removed |
| vec/vec_i16x4 | 373 | 229 | 78 | 23 | calls removed |
| vec/vec_i16x8 | 654 | 319 | 146 | 23 | calls removed |
| vec/vec_i32x4 | 560 | 393 | 78 | 23 | calls removed |
| vec/vec_i64x2 | 369 | 305 | 44 | 23 | calls removed |
| vec/vec_i8x16 | 425 | 773 | 42 | 25 | calls removed |
| vec/vec_lane_ops | 537 | 733 | 107 | 39 | calls removed |
| vec/vec_mem | 1339 | 1369 | 78 | 19 | calls removed |
| vec/vec_scalar_mix | 568 | 650 | 67 | 20 | calls removed |
| vec/vec_u16x8 | 654 | 319 | 146 | 23 | calls removed |
| vec/vec_u32x4 | 559 | 394 | 78 | 23 | calls removed |
| vec/vec_u64x2 | 369 | 305 | 44 | 23 | calls removed |
| vec/vec_u8x16 | 433 | 781 | 42 | 25 | calls removed |

## aarch64-linux: 91 goldens changed

| classification | cases |
| --- | --- |
| calls removed | 91 |

calls before 4304, after 1791, cases reaching zero calls 31
frame direction: frame larger 26, frame same 9, frame smaller 32, no frame adjustment 24

| golden | lines removed | lines added | calls before | calls after | class |
| --- | --- | --- | --- | --- | --- |
| bits/logic_u32 | 104 | 337 | 22 | 5 | calls removed |
| bits/logic_u64 | 79 | 315 | 22 | 5 | calls removed |
| bits/shift_i32 | 99 | 345 | 28 | 11 | calls removed |
| bits/shift_i64 | 135 | 326 | 28 | 11 | calls removed |
| bits/shift_u32 | 98 | 345 | 27 | 10 | calls removed |
| bits/shift_u64 | 131 | 325 | 27 | 10 | calls removed |
| call/call_chain | 162 | 376 | 172 | 154 | calls removed |
| call/call_float_regs | 294 | 378 | 87 | 30 | calls removed |
| call/call_indirect | 235 | 363 | 69 | 24 | calls removed |
| call/call_int_regs | 388 | 571 | 123 | 24 | calls removed |
| call/call_mixed | 110 | 383 | 53 | 35 | calls removed |
| call/call_rec_edge | 372 | 419 | 87 | 50 | calls removed |
| call/call_rec_large | 179 | 655 | 134 | 97 | calls removed |
| call/call_rec_small | 237 | 527 | 71 | 30 | calls removed |
| call/call_recursive | 217 | 226 | 53 | 13 | calls removed |
| call/call_ret_large | 234 | 700 | 138 | 96 | calls removed |
| call/call_ret_small | 276 | 529 | 88 | 42 | calls removed |
| call/call_variadic | 109 | 339 | 158 | 141 | calls removed |
| cmp/branch_nest | 104 | 126 | 3 | 0 | calls removed |
| cmp/cmp_i32 | 91 | 185 | 7 | 0 | calls removed |
| cmp/cmp_i64 | 110 | 196 | 7 | 0 | calls removed |
| cmp/cmp_u32 | 91 | 185 | 7 | 0 | calls removed |
| cmp/cmp_u64 | 110 | 196 | 7 | 0 | calls removed |
| cmp/loop_shapes | 90 | 159 | 6 | 0 | calls removed |
| cmp/short_circuit | 244 | 209 | 36 | 11 | calls removed |
| comptime/ct_runtime_agree | 155 | 375 | 54 | 37 | calls removed |
| convert/bitcast | 65 | 197 | 11 | 0 | calls removed |
| convert/ext_sign | 96 | 279 | 15 | 0 | calls removed |
| convert/ext_zero | 96 | 279 | 15 | 0 | calls removed |
| convert/f2f | 51 | 199 | 11 | 0 | calls removed |
| convert/f2i | 128 | 622 | 54 | 21 | calls removed |
| convert/f2u_high | 85 | 456 | 25 | 0 | calls removed |
| convert/i2f | 135 | 649 | 37 | 4 | calls removed |
| convert/trunc | 65 | 166 | 9 | 0 | calls removed |
| float/arith_f32 | 398 | 258 | 65 | 46 | calls removed |
| float/arith_f64 | 392 | 234 | 65 | 46 | calls removed |
| float/cmp_f32 | 240 | 431 | 20 | 2 | calls removed |
| float/cmp_f64 | 175 | 418 | 26 | 7 | calls removed |
| float/special_f32 | 278 | 399 | 117 | 84 | calls removed |
| float/special_f64 | 307 | 396 | 127 | 94 | calls removed |
| frame/frame_align | 363 | 395 | 79 | 24 | calls removed |
| frame/frame_large | 170 | 400 | 16 | 0 | calls removed |
| frame/frame_spill | 171 | 475 | 32 | 15 | calls removed |
| imm/imm_add | 118 | 323 | 25 | 8 | calls removed |
| imm/imm_addr | 140 | 434 | 21 | 4 | calls removed |
| imm/imm_logic | 87 | 304 | 21 | 4 | calls removed |
| imm/imm_mov | 85 | 262 | 12 | 0 | calls removed |
| mem/addr_modes | 199 | 411 | 36 | 19 | calls removed |
| mem/array_index | 212 | 444 | 36 | 15 | calls removed |
| mem/global_data | 117 | 299 | 35 | 12 | calls removed |
| mem/global_zero | 93 | 275 | 28 | 7 | calls removed |
| mem/loadstore | 173 | 373 | 25 | 8 | calls removed |
| mem/ptr_arith | 161 | 356 | 29 | 8 | calls removed |
| mem/rec_layout | 168 | 477 | 50 | 25 | calls removed |
| mem/uni_layout | 129 | 375 | 32 | 15 | calls removed |
| scalar/arith_i16 | 70 | 260 | 13 | 0 | calls removed |
| scalar/arith_i32 | 70 | 260 | 13 | 0 | calls removed |
| scalar/arith_i64 | 69 | 247 | 13 | 0 | calls removed |
| scalar/arith_i8 | 70 | 260 | 13 | 0 | calls removed |
| scalar/arith_u16 | 68 | 258 | 13 | 0 | calls removed |
| scalar/arith_u32 | 68 | 258 | 13 | 0 | calls removed |
| scalar/arith_u64 | 67 | 245 | 13 | 0 | calls removed |
| scalar/arith_u8 | 68 | 258 | 13 | 0 | calls removed |
| scalar/divrem_i32 | 130 | 307 | 13 | 0 | calls removed |
| scalar/divrem_i64 | 128 | 293 | 13 | 0 | calls removed |
| scalar/divrem_u32 | 74 | 204 | 10 | 0 | calls removed |
| scalar/divrem_u64 | 73 | 194 | 10 | 0 | calls removed |
| scalar/mul_i32 | 43 | 152 | 8 | 0 | calls removed |
| scalar/mul_i64 | 42 | 144 | 8 | 0 | calls removed |
| scalar/mul_u32 | 41 | 150 | 8 | 0 | calls removed |
| scalar/mul_u64 | 40 | 142 | 8 | 0 | calls removed |
| vec/autovec_loop | 463 | 630 | 10 | 0 | calls removed |
| vec/vec_cmp_select | 156 | 409 | 169 | 109 | calls removed |
| vec/vec_f32x2 | 304 | 119 | 37 | 16 | calls removed |
| vec/vec_f32x3 | 168 | 163 | 33 | 9 | calls removed |
| vec/vec_f32x4 | 283 | 164 | 71 | 16 | calls removed |
| vec/vec_f32x5 | 891 | 560 | 88 | 16 | calls removed |
| vec/vec_f32x8 | 1404 | 796 | 139 | 16 | calls removed |
| vec/vec_f64x2 | 153 | 121 | 37 | 16 | calls removed |
| vec/vec_i16x4 | 322 | 190 | 78 | 23 | calls removed |
| vec/vec_i16x8 | 577 | 288 | 146 | 23 | calls removed |
| vec/vec_i32x4 | 304 | 184 | 78 | 23 | calls removed |
| vec/vec_i64x2 | 118 | 106 | 44 | 23 | calls removed |
| vec/vec_i8x16 | 270 | 464 | 42 | 25 | calls removed |
| vec/vec_lane_ops | 417 | 448 | 107 | 39 | calls removed |
| vec/vec_mem | 1189 | 1162 | 78 | 19 | calls removed |
| vec/vec_scalar_mix | 393 | 430 | 67 | 20 | calls removed |
| vec/vec_u16x8 | 577 | 288 | 146 | 23 | calls removed |
| vec/vec_u32x4 | 305 | 184 | 78 | 23 | calls removed |
| vec/vec_u64x2 | 118 | 106 | 44 | 23 | calls removed |
| vec/vec_u8x16 | 270 | 464 | 42 | 25 | calls removed |

## riscv64-linux: 91 goldens changed

| classification | cases |
| --- | --- |
| calls removed | 91 |

calls before 4304, after 1791, cases reaching zero calls 31
frame direction: frame larger 8, frame same 5, frame smaller 64, no frame adjustment 14

| golden | lines removed | lines added | calls before | calls after | class |
| --- | --- | --- | --- | --- | --- |
| bits/logic_u32 | 123 | 327 | 22 | 5 | calls removed |
| bits/logic_u64 | 98 | 289 | 22 | 5 | calls removed |
| bits/shift_i32 | 119 | 368 | 28 | 11 | calls removed |
| bits/shift_i64 | 154 | 348 | 28 | 11 | calls removed |
| bits/shift_u32 | 117 | 383 | 27 | 10 | calls removed |
| bits/shift_u64 | 150 | 347 | 27 | 10 | calls removed |
| call/call_chain | 309 | 511 | 172 | 154 | calls removed |
| call/call_float_regs | 333 | 550 | 87 | 30 | calls removed |
| call/call_indirect | 398 | 485 | 69 | 24 | calls removed |
| call/call_int_regs | 478 | 697 | 123 | 24 | calls removed |
| call/call_mixed | 235 | 467 | 53 | 35 | calls removed |
| call/call_rec_edge | 1102 | 1242 | 87 | 50 | calls removed |
| call/call_rec_large | 626 | 1404 | 134 | 97 | calls removed |
| call/call_rec_small | 340 | 842 | 71 | 30 | calls removed |
| call/call_recursive | 314 | 338 | 53 | 13 | calls removed |
| call/call_ret_large | 701 | 1403 | 138 | 96 | calls removed |
| call/call_ret_small | 370 | 1290 | 88 | 42 | calls removed |
| call/call_variadic | 201 | 410 | 158 | 141 | calls removed |
| cmp/branch_nest | 96 | 110 | 3 | 0 | calls removed |
| cmp/cmp_i32 | 167 | 260 | 7 | 0 | calls removed |
| cmp/cmp_i64 | 154 | 251 | 7 | 0 | calls removed |
| cmp/cmp_u32 | 167 | 260 | 7 | 0 | calls removed |
| cmp/cmp_u64 | 154 | 251 | 7 | 0 | calls removed |
| cmp/loop_shapes | 101 | 175 | 6 | 0 | calls removed |
| cmp/short_circuit | 287 | 259 | 36 | 11 | calls removed |
| comptime/ct_runtime_agree | 228 | 435 | 54 | 37 | calls removed |
| convert/bitcast | 81 | 198 | 11 | 0 | calls removed |
| convert/ext_sign | 127 | 308 | 15 | 0 | calls removed |
| convert/ext_zero | 123 | 308 | 15 | 0 | calls removed |
| convert/f2f | 69 | 208 | 11 | 0 | calls removed |
| convert/f2i | 152 | 690 | 54 | 21 | calls removed |
| convert/f2u_high | 145 | 481 | 25 | 0 | calls removed |
| convert/i2f | 189 | 676 | 37 | 4 | calls removed |
| convert/trunc | 85 | 178 | 9 | 0 | calls removed |
| float/arith_f32 | 418 | 335 | 65 | 46 | calls removed |
| float/arith_f64 | 367 | 305 | 65 | 46 | calls removed |
| float/cmp_f32 | 280 | 477 | 20 | 2 | calls removed |
| float/cmp_f64 | 199 | 387 | 26 | 7 | calls removed |
| float/special_f32 | 353 | 457 | 117 | 84 | calls removed |
| float/special_f64 | 308 | 432 | 127 | 94 | calls removed |
| frame/frame_align | 773 | 682 | 79 | 24 | calls removed |
| frame/frame_large | 224 | 424 | 16 | 0 | calls removed |
| frame/frame_spill | 233 | 531 | 32 | 15 | calls removed |
| imm/imm_add | 125 | 329 | 25 | 8 | calls removed |
| imm/imm_addr | 171 | 431 | 21 | 4 | calls removed |
| imm/imm_logic | 97 | 326 | 21 | 4 | calls removed |
| imm/imm_mov | 112 | 279 | 12 | 0 | calls removed |
| mem/addr_modes | 256 | 441 | 36 | 19 | calls removed |
| mem/array_index | 259 | 607 | 36 | 15 | calls removed |
| mem/global_data | 255 | 452 | 35 | 12 | calls removed |
| mem/global_zero | 186 | 358 | 28 | 7 | calls removed |
| mem/loadstore | 193 | 369 | 25 | 8 | calls removed |
| mem/ptr_arith | 213 | 504 | 29 | 8 | calls removed |
| mem/rec_layout | 522 | 851 | 50 | 25 | calls removed |
| mem/uni_layout | 244 | 459 | 32 | 15 | calls removed |
| scalar/arith_i16 | 89 | 250 | 13 | 0 | calls removed |
| scalar/arith_i32 | 86 | 235 | 13 | 0 | calls removed |
| scalar/arith_i64 | 85 | 201 | 13 | 0 | calls removed |
| scalar/arith_i8 | 89 | 250 | 13 | 0 | calls removed |
| scalar/arith_u16 | 89 | 250 | 13 | 0 | calls removed |
| scalar/arith_u32 | 86 | 247 | 13 | 0 | calls removed |
| scalar/arith_u64 | 85 | 201 | 13 | 0 | calls removed |
| scalar/arith_u8 | 90 | 239 | 13 | 0 | calls removed |
| scalar/divrem_i32 | 149 | 285 | 13 | 0 | calls removed |
| scalar/divrem_i64 | 146 | 268 | 13 | 0 | calls removed |
| scalar/divrem_u32 | 93 | 230 | 10 | 0 | calls removed |
| scalar/divrem_u64 | 86 | 203 | 10 | 0 | calls removed |
| scalar/mul_i32 | 56 | 156 | 8 | 0 | calls removed |
| scalar/mul_i64 | 55 | 148 | 8 | 0 | calls removed |
| scalar/mul_u32 | 56 | 163 | 8 | 0 | calls removed |
| scalar/mul_u64 | 55 | 141 | 8 | 0 | calls removed |
| vec/autovec_loop | 375 | 563 | 10 | 0 | calls removed |
| vec/vec_cmp_select | 2111 | 2093 | 169 | 109 | calls removed |
| vec/vec_f32x2 | 488 | 347 | 37 | 16 | calls removed |
| vec/vec_f32x3 | 543 | 369 | 33 | 9 | calls removed |
| vec/vec_f32x4 | 1328 | 514 | 71 | 16 | calls removed |
| vec/vec_f32x5 | 1683 | 593 | 88 | 16 | calls removed |
| vec/vec_f32x8 | 1630 | 846 | 139 | 16 | calls removed |
| vec/vec_f64x2 | 457 | 328 | 37 | 16 | calls removed |
| vec/vec_i16x4 | 865 | 609 | 78 | 23 | calls removed |
| vec/vec_i16x8 | 2931 | 896 | 146 | 23 | calls removed |
| vec/vec_i32x4 | 1642 | 586 | 78 | 23 | calls removed |
| vec/vec_i64x2 | 980 | 400 | 44 | 23 | calls removed |
| vec/vec_i8x16 | 2120 | 1215 | 42 | 25 | calls removed |
| vec/vec_lane_ops | 3104 | 3384 | 107 | 39 | calls removed |
| vec/vec_mem | 2347 | 1011 | 78 | 19 | calls removed |
| vec/vec_scalar_mix | 1480 | 1234 | 67 | 20 | calls removed |
| vec/vec_u16x8 | 2925 | 926 | 146 | 23 | calls removed |
| vec/vec_u32x4 | 1623 | 587 | 78 | 23 | calls removed |
| vec/vec_u64x2 | 980 | 400 | 44 | 23 | calls removed |
| vec/vec_u8x16 | 2119 | 1199 | 42 | 25 | calls removed |

## vec/vec_cmp_i64, added by #3271 after the tables above

The case arrived on `dev` with its own goldens on every target while this lane
was open. Merging it and re-running layer B with the merged compiler moved
exactly that golden on each ISA and nothing else (367 pass, 1 fail, 0 skip
per target before the bless); every previously blessed golden kept its
content. Same classification as the rest of the corpus: the fold sites lose
their calls.

| target | lines removed | lines added | calls before | calls after | frame | class |
| --- | --- | --- | --- | --- | --- | --- |
| x86_64-linux | 433 | 297 | 33 | 13 | same | calls removed |
| aarch64-linux | 269 | 164 | 33 | 13 | larger | calls removed |
| riscv64-linux | 690 | 679 | 33 | 13 | smaller | calls removed |
