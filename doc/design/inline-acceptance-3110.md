# Inlining acceptance inventory for #3110

Roadmap item N6. One row per acceptance bullet of #3110: what `dev` does at the
start of the phase 1 lane (`853d4c17`) with the test that shows it, what the
three candidate commits on `archive/3218-candidate-era` add, and a verdict.
Phase 1 (#3270) covered the compiler policy with local fixtures shaped like the
eight `std.sync.atomic` wrappers (short asm bodies with volatile memory
effects). Phase 2, recorded in the `Phase 2` paragraphs below, asserts the same
claims on the wrappers std ships: `dev` at `83d3c1c9d` pins std 2.0.0
(`e6fc41251`), whose `std.sync.atomic` carries the eight `#[inline]`
annotations (S4a, std PR #633), and the evidence is a link-suite case that
links and runs them on the three native ELF targets with the call counts read
from the linked image, plus three driver tests that read the pinned
`dep/std/src/sync/atomic.mach` itself.

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

Phase 2: `test/link/cases/3110-atomic-inline` builds a program that calls each
of the eight wrappers from the pinned std across the module boundary and reads
the linked image with `llvm-objdump`: the release cell's
`calls from case text:` line is `load=0 store=0 cas=0 fetch_add=0 fetch_sub=0
exchange=0 fence=0 spin_hint=0` over every function the case defines (`main`
and `case.*`), on `x86_64-linux`, `aarch64-linux` and `riscv64-linux`; the
debug cell is the control with `load=4 store=4 cas=3 fetch_add=3 fetch_sub=2
exchange=2 fence=2 spin_hint=5`, the counts the source has. Mutation control:
`#[noinline]` on `fetch_add` in the case's std copy reads `fetch_add=3` and
`call std.sync.atomic.fetch_add` in the probe sequence. At the IR level
`driver:inline_std_atomic_wrappers_lose_their_calls_and_keep_their_asm_in_order`
counts zero `OP_CALL` and eight `OP_ASM` in the importer.

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

Phase 2: the same case's `probe:` block lists the atomic instructions of
`case.main.probe` in text order, which is the order the eight wrappers were
called in. The goldens (`expect.<target>.release.txt`) record, per ISA:

| ISA | sequence the importer's text carries |
| --- | --- |
| x86_64 | `xchgq` (store), `lock cmpxchgq` (cas), `lock xaddq` (fetch_add), `lock xaddq` after `neg` (fetch_sub), `xchgq` (exchange), `mfence`, `pause`; the load is a plain aligned `mov` |
| aarch64 | `ldar` (load), `stlr` (store), then `ldaxr`/`stlxr`/`dmb ish` for cas, fetch_add, fetch_sub and exchange, `dmb ish` (fence), `yield` |
| riscv64 | `fence`/`ld`/`fence` (load), `amoswap.d.aqrl zero` (store), `lr.d.aqrl`/`sc.d.aqrl` (cas), `amoadd.d.aqrl` twice, `amoswap.d.aqrl` (exchange), `fence`, `fence w, 0` (the `pause` hint) |

The program the case runs uses every wrapper from two threads in shapes whose
correctness depends on the atomicity and the ordering the wrappers promise (a
fetch_add counter, a cas loop counter, a fetch_sub countdown, a spinlock from
exchange and store, and a producer/consumer handshake through store/load with
a fence), and its `added=40000 swapped=40000 remaining=0 guarded=40000 torn=0`
line is part of the golden. In IR,
`driver:inline_std_atomic_wrappers_lose_their_calls_and_keep_their_asm_in_order`
checks each of the eight inlined `OP_ASM` payloads in call order for its own
x86-64 instruction.

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

Phase 2:
`driver:release_text_is_byte_identical_with_and_without_g_for_the_std_atomic_wrappers`
runs the same comparison over an importer of the pinned std's eight wrappers:
zero calls in both builds and identical `.text` bytes.

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

Phase 2: `driver:std_atomic_wrapper_body_edit_relowers_the_importer` runs three
rounds over the pinned `atomic.mach` as the std dependency: the shipped text,
then `fence`'s `mfence` edited to `lfence`, then an edit to a `noinline`
function appended beside the wrappers. The importer's `Q_LOWER` revision moves
on the body edit while the provider's `Q_LOWERED_SURFACE` does not, and the
importer's seventh inlined asm carries `lfence`; the edit beside the wrappers
moves the provider's `Q_LOWER` and leaves the importer's.

### Pure versus atomic controls

`dev`: there is no purity model in the inliner or the pipeline; an inlining
decision reads size and flags only, and after inlining every later pass reads
`ir/effect.mach`, where `OP_ASM` is never discardable, CSE-safe or movable.
Nothing classifies a short asm body as pure.

Candidate: unchanged.

Verdict: accepted on `dev`. The controls are the effect legs above: an inlined
atomic-shaped wrapper whose result is unused is not removed and two identical
ones are not merged.

Phase 2: the link case's `contend` calls `fetch_add` and `fetch_sub` for their
effect only, and its counts come out right on all three targets; the release
goldens show the `lock xaddq` / `ldaxr`+`stlxr` / `amoadd.d.aqrl` still in the
importer's text, not dropped as a pure call with an unused result.

### The eight atomic wrappers

`std.sync.atomic` at the std phase 1 pinned (`168a9f76`, 1.0.1) defined `load`,
`store`, `cas`, `fetch_add`, `fetch_sub`, `exchange`, `fence` and `spin_hint`
as unannotated `pub fun` bodies of one `asm` block under `$if` on the build
arch, each well under 25 live instructions after `mem2reg`. Under the ported
policy every one is extracted into `Q_INLINE_BODIES` and inlined at its
importers with no std-side annotation; `#[inline]` only overrides the size
bar, which they clear. The local fixtures in the phase 1 tests mirror their
shape (a `{ptr}`/`{result}` binding pair around one memory instruction).

Phase 1 verdict: compiler policy covered by the effect legs above; the
std-side annotations were Track C's S4 and not needed for elimination.

Phase 2: std 2.0.0 (`e6fc41251`, pinned by `dev` at `83d3c1c9d`) carries the
eight `#[inline]` annotations (S4a, std PR #633; its MIGRATION.md records that
the release policy already inlines them unannotated and the annotations pin
the decision). The wrappers themselves are now the subject:

| evidence | where | what it shows |
| --- | --- | --- |
| link case `3110-atomic-inline`, release cells | `test/link/cases/3110-atomic-inline/expect.{x86_64,aarch64,riscv64}-linux.release.txt` | zero direct calls to any `std.sync.atomic.*` symbol from the case's text; the ISA's atomic instruction sequence in call order in `probe`; the program's single-thread results and two-thread counts |
| link case, debug cells | `expect.<target>.debug.txt` | the control: eight `call std.sync.atomic.*` in `probe`, per-wrapper call counts matching the source |
| link case, `by-address=100` | the goldens | a call through a function value the optimizer cannot see through (`pick` is `noinline`) reaches the provider's one strong definition, which no inlined copy replaces |
| `driver:inline_std_atomic_wrappers_lose_their_calls_and_keep_their_asm_in_order` | `src/lang/driver/tests.mach` | zero `OP_CALL`, eight `OP_ASM` in call order, each payload the wrapper's own x86-64 instruction |
| `driver:release_text_is_byte_identical_with_and_without_g_for_the_std_atomic_wrappers` | same | `-g` moves no inlining decision on the real wrappers |
| `driver:std_atomic_wrapper_body_edit_relowers_the_importer` | same | an edit to `atomic.mach`'s body re-lowers the importer and reaches its text; an edit beside the wrappers does not |

The three driver tests read `dep/std/src/sync/atomic.mach` and
`dep/std/src/types/bool.mach` from the repository at test time and place them
verbatim in a `std` path dependency of the scaffold, so they measure the file
std ships at the pin rather than a restatement of it. Mutation controls:
`#[noinline]` on `fetch_add` in the case's std copy fails the release goldens
(`fetch_add=3`, `call std.sync.atomic.fetch_add` in the sequence), and
`#[noinline]` on `cas` in the pinned `dep/std` fails the three driver tests
(exits 8, 50 and 10: calls present).

The link case's `aarch64-linux` goldens were blessed from a `qemu-aarch64` run
of the producer on the x86-64 host (the observable is the instruction text
and the program's deterministic output); the leg itself executes natively on
CI's arm runner. `riscv64-linux` executes under qemu on both, which
`test/engines.conf` marks as compute evidence only: the ordering claim on that
ISA rests on the instruction sequence the case reads from the image.

Verdict: every #3110 bullet is demonstrated on the wrappers std ships, on the
three native ELF targets.

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

## The windows and darwin columns, blessed after the merge

`x86_64-windows`, `x86_64-darwin` and `aarch64-darwin` decode on the pr lane
but were not re-blessed in #3270, so all 92 layer B cells per column failed
against `dev` at `da4fe7146` while layer A stayed green (184 pass, 92 fail,
92 skip on each column before the bless; every failure a `layer b o2` cell).
The tables below use the same method as the linux ones, computed by comparing
each blessed golden with its `HEAD` version, except that lines removed and
added are `git diff --numstat` counts so a reviewer reproduces them with the
commit's own stat. Every golden's call count before and after is identical to
its `x86_64-linux` or `aarch64-linux` counterpart, which is what the
target-independent case contract predicts: the fold sites are the same source
on every column and each one loses the same calls.

## x86_64-windows: 92 goldens changed

| classification | cases |
| --- | --- |
| calls removed | 92 |

calls before 4337, after 1804, cases reaching zero calls 31
frame direction: frame larger 15, frame same 28, frame smaller 49

| golden | lines removed | lines added | calls before | calls after | class |
| --- | --- | --- | --- | --- | --- |
| bits/logic_u32 | 133 | 441 | 22 | 5 | calls removed |
| bits/logic_u64 | 137 | 430 | 22 | 5 | calls removed |
| bits/shift_i32 | 206 | 471 | 28 | 11 | calls removed |
| bits/shift_i64 | 183 | 433 | 28 | 11 | calls removed |
| bits/shift_u32 | 201 | 468 | 27 | 10 | calls removed |
| bits/shift_u64 | 178 | 430 | 27 | 10 | calls removed |
| call/call_chain | 797 | 1122 | 172 | 154 | calls removed |
| call/call_float_regs | 595 | 723 | 87 | 30 | calls removed |
| call/call_indirect | 666 | 909 | 69 | 24 | calls removed |
| call/call_int_regs | 907 | 946 | 123 | 24 | calls removed |
| call/call_mixed | 647 | 1055 | 53 | 35 | calls removed |
| call/call_rec_edge | 714 | 1044 | 87 | 50 | calls removed |
| call/call_rec_large | 560 | 1168 | 134 | 97 | calls removed |
| call/call_rec_small | 612 | 1051 | 71 | 30 | calls removed |
| call/call_recursive | 580 | 647 | 53 | 13 | calls removed |
| call/call_ret_large | 1234 | 1979 | 138 | 96 | calls removed |
| call/call_ret_small | 642 | 1174 | 88 | 42 | calls removed |
| call/call_variadic | 408 | 966 | 158 | 141 | calls removed |
| cmp/branch_nest | 142 | 180 | 3 | 0 | calls removed |
| cmp/cmp_i32 | 114 | 232 | 7 | 0 | calls removed |
| cmp/cmp_i64 | 139 | 254 | 7 | 0 | calls removed |
| cmp/cmp_u32 | 114 | 232 | 7 | 0 | calls removed |
| cmp/cmp_u64 | 139 | 254 | 7 | 0 | calls removed |
| cmp/loop_shapes | 115 | 217 | 6 | 0 | calls removed |
| cmp/short_circuit | 597 | 606 | 36 | 11 | calls removed |
| comptime/ct_runtime_agree | 418 | 726 | 54 | 37 | calls removed |
| convert/bitcast | 47 | 223 | 11 | 0 | calls removed |
| convert/ext_sign | 83 | 331 | 15 | 0 | calls removed |
| convert/ext_zero | 83 | 329 | 15 | 0 | calls removed |
| convert/f2f | 72 | 245 | 11 | 0 | calls removed |
| convert/f2i | 273 | 874 | 54 | 21 | calls removed |
| convert/f2u_high | 233 | 684 | 25 | 0 | calls removed |
| convert/i2f | 288 | 885 | 37 | 4 | calls removed |
| convert/trunc | 64 | 209 | 9 | 0 | calls removed |
| float/arith_f32 | 1128 | 909 | 65 | 46 | calls removed |
| float/arith_f64 | 1120 | 902 | 65 | 46 | calls removed |
| float/cmp_f32 | 416 | 672 | 20 | 2 | calls removed |
| float/cmp_f64 | 467 | 943 | 26 | 7 | calls removed |
| float/special_f32 | 755 | 811 | 117 | 84 | calls removed |
| float/special_f64 | 833 | 883 | 127 | 94 | calls removed |
| frame/frame_align | 711 | 781 | 79 | 24 | calls removed |
| frame/frame_large | 244 | 511 | 16 | 0 | calls removed |
| frame/frame_spill | 424 | 782 | 32 | 15 | calls removed |
| imm/imm_add | 119 | 385 | 25 | 8 | calls removed |
| imm/imm_addr | 232 | 556 | 21 | 4 | calls removed |
| imm/imm_logic | 115 | 396 | 21 | 4 | calls removed |
| imm/imm_mov | 92 | 280 | 12 | 0 | calls removed |
| mem/addr_modes | 673 | 1049 | 36 | 19 | calls removed |
| mem/array_index | 588 | 1009 | 36 | 15 | calls removed |
| mem/global_data | 284 | 569 | 35 | 12 | calls removed |
| mem/global_zero | 329 | 615 | 28 | 7 | calls removed |
| mem/loadstore | 227 | 572 | 25 | 8 | calls removed |
| mem/ptr_arith | 308 | 669 | 29 | 8 | calls removed |
| mem/rec_layout | 849 | 1302 | 50 | 25 | calls removed |
| mem/uni_layout | 330 | 641 | 32 | 15 | calls removed |
| scalar/arith_i16 | 101 | 329 | 13 | 0 | calls removed |
| scalar/arith_i32 | 97 | 325 | 13 | 0 | calls removed |
| scalar/arith_i64 | 100 | 309 | 13 | 0 | calls removed |
| scalar/arith_i8 | 101 | 329 | 13 | 0 | calls removed |
| scalar/arith_u16 | 100 | 328 | 13 | 0 | calls removed |
| scalar/arith_u32 | 96 | 324 | 13 | 0 | calls removed |
| scalar/arith_u64 | 100 | 309 | 13 | 0 | calls removed |
| scalar/arith_u8 | 100 | 328 | 13 | 0 | calls removed |
| scalar/divrem_i32 | 160 | 411 | 13 | 0 | calls removed |
| scalar/divrem_i64 | 145 | 369 | 13 | 0 | calls removed |
| scalar/divrem_u32 | 111 | 292 | 10 | 0 | calls removed |
| scalar/divrem_u64 | 100 | 263 | 10 | 0 | calls removed |
| scalar/mul_i32 | 65 | 198 | 8 | 0 | calls removed |
| scalar/mul_i64 | 56 | 179 | 8 | 0 | calls removed |
| scalar/mul_u32 | 64 | 197 | 8 | 0 | calls removed |
| scalar/mul_u64 | 57 | 180 | 8 | 0 | calls removed |
| vec/autovec_loop | 709 | 896 | 10 | 0 | calls removed |
| vec/vec_cmp_i64 | 484 | 481 | 33 | 13 | calls removed |
| vec/vec_cmp_select | 824 | 985 | 169 | 109 | calls removed |
| vec/vec_f32x2 | 352 | 280 | 37 | 16 | calls removed |
| vec/vec_f32x3 | 398 | 383 | 33 | 9 | calls removed |
| vec/vec_f32x4 | 566 | 370 | 71 | 16 | calls removed |
| vec/vec_f32x5 | 1905 | 1683 | 88 | 16 | calls removed |
| vec/vec_f32x8 | 2957 | 2419 | 139 | 16 | calls removed |
| vec/vec_f64x2 | 314 | 306 | 37 | 16 | calls removed |
| vec/vec_i16x4 | 516 | 331 | 78 | 23 | calls removed |
| vec/vec_i16x8 | 855 | 434 | 146 | 23 | calls removed |
| vec/vec_i32x4 | 736 | 589 | 78 | 23 | calls removed |
| vec/vec_i64x2 | 436 | 398 | 44 | 23 | calls removed |
| vec/vec_i8x16 | 1311 | 1578 | 42 | 25 | calls removed |
| vec/vec_lane_ops | 1027 | 1036 | 107 | 39 | calls removed |
| vec/vec_mem | 1842 | 1918 | 78 | 19 | calls removed |
| vec/vec_scalar_mix | 825 | 960 | 67 | 20 | calls removed |
| vec/vec_u16x8 | 855 | 434 | 146 | 23 | calls removed |
| vec/vec_u32x4 | 744 | 591 | 78 | 23 | calls removed |
| vec/vec_u64x2 | 436 | 398 | 44 | 23 | calls removed |
| vec/vec_u8x16 | 1343 | 1610 | 42 | 25 | calls removed |

## x86_64-darwin: 92 goldens changed

| classification | cases |
| --- | --- |
| calls removed | 92 |

calls before 4337, after 1804, cases reaching zero calls 31
frame direction: frame larger 37, frame same 45, frame smaller 10

| golden | lines removed | lines added | calls before | calls after | class |
| --- | --- | --- | --- | --- | --- |
| bits/logic_u32 | 139 | 421 | 22 | 5 | calls removed |
| bits/logic_u64 | 137 | 409 | 22 | 5 | calls removed |
| bits/shift_i32 | 190 | 469 | 28 | 11 | calls removed |
| bits/shift_i64 | 138 | 415 | 28 | 11 | calls removed |
| bits/shift_u32 | 185 | 464 | 27 | 10 | calls removed |
| bits/shift_u64 | 135 | 412 | 27 | 10 | calls removed |
| call/call_chain | 651 | 933 | 172 | 154 | calls removed |
| call/call_float_regs | 568 | 673 | 87 | 30 | calls removed |
| call/call_indirect | 676 | 900 | 69 | 24 | calls removed |
| call/call_int_regs | 950 | 941 | 123 | 24 | calls removed |
| call/call_mixed | 603 | 925 | 53 | 35 | calls removed |
| call/call_rec_edge | 707 | 985 | 87 | 50 | calls removed |
| call/call_rec_large | 477 | 984 | 134 | 97 | calls removed |
| call/call_rec_small | 612 | 1026 | 71 | 30 | calls removed |
| call/call_recursive | 565 | 556 | 53 | 13 | calls removed |
| call/call_ret_large | 1317 | 1999 | 138 | 96 | calls removed |
| call/call_ret_small | 678 | 1126 | 88 | 42 | calls removed |
| call/call_variadic | 389 | 841 | 158 | 141 | calls removed |
| cmp/branch_nest | 143 | 177 | 3 | 0 | calls removed |
| cmp/cmp_i32 | 126 | 229 | 7 | 0 | calls removed |
| cmp/cmp_i64 | 137 | 238 | 7 | 0 | calls removed |
| cmp/cmp_u32 | 126 | 229 | 7 | 0 | calls removed |
| cmp/cmp_u64 | 137 | 238 | 7 | 0 | calls removed |
| cmp/loop_shapes | 123 | 210 | 6 | 0 | calls removed |
| cmp/short_circuit | 597 | 595 | 36 | 11 | calls removed |
| comptime/ct_runtime_agree | 369 | 628 | 54 | 37 | calls removed |
| convert/bitcast | 54 | 224 | 11 | 0 | calls removed |
| convert/ext_sign | 82 | 320 | 15 | 0 | calls removed |
| convert/ext_zero | 83 | 320 | 15 | 0 | calls removed |
| convert/f2f | 69 | 236 | 11 | 0 | calls removed |
| convert/f2i | 321 | 860 | 54 | 21 | calls removed |
| convert/f2u_high | 233 | 657 | 25 | 0 | calls removed |
| convert/i2f | 207 | 809 | 37 | 4 | calls removed |
| convert/trunc | 60 | 197 | 9 | 0 | calls removed |
| float/arith_f32 | 1076 | 849 | 65 | 46 | calls removed |
| float/arith_f64 | 1033 | 807 | 65 | 46 | calls removed |
| float/cmp_f32 | 412 | 626 | 20 | 2 | calls removed |
| float/cmp_f64 | 491 | 969 | 26 | 7 | calls removed |
| float/special_f32 | 716 | 730 | 117 | 84 | calls removed |
| float/special_f64 | 803 | 825 | 127 | 94 | calls removed |
| frame/frame_align | 677 | 681 | 79 | 24 | calls removed |
| frame/frame_large | 245 | 510 | 16 | 0 | calls removed |
| frame/frame_spill | 321 | 571 | 32 | 15 | calls removed |
| imm/imm_add | 124 | 387 | 25 | 8 | calls removed |
| imm/imm_addr | 239 | 564 | 21 | 4 | calls removed |
| imm/imm_logic | 129 | 393 | 21 | 4 | calls removed |
| imm/imm_mov | 92 | 284 | 12 | 0 | calls removed |
| mem/addr_modes | 541 | 917 | 36 | 19 | calls removed |
| mem/array_index | 633 | 1014 | 36 | 15 | calls removed |
| mem/global_data | 293 | 564 | 35 | 12 | calls removed |
| mem/global_zero | 309 | 583 | 28 | 7 | calls removed |
| mem/loadstore | 192 | 479 | 25 | 8 | calls removed |
| mem/ptr_arith | 391 | 684 | 29 | 8 | calls removed |
| mem/rec_layout | 802 | 1257 | 50 | 25 | calls removed |
| mem/uni_layout | 327 | 615 | 32 | 15 | calls removed |
| scalar/arith_i16 | 108 | 331 | 13 | 0 | calls removed |
| scalar/arith_i32 | 116 | 332 | 13 | 0 | calls removed |
| scalar/arith_i64 | 108 | 313 | 13 | 0 | calls removed |
| scalar/arith_i8 | 108 | 331 | 13 | 0 | calls removed |
| scalar/arith_u16 | 107 | 330 | 13 | 0 | calls removed |
| scalar/arith_u32 | 115 | 331 | 13 | 0 | calls removed |
| scalar/arith_u64 | 108 | 313 | 13 | 0 | calls removed |
| scalar/arith_u8 | 107 | 330 | 13 | 0 | calls removed |
| scalar/divrem_i32 | 159 | 406 | 13 | 0 | calls removed |
| scalar/divrem_i64 | 147 | 368 | 13 | 0 | calls removed |
| scalar/divrem_u32 | 116 | 293 | 10 | 0 | calls removed |
| scalar/divrem_u64 | 110 | 270 | 10 | 0 | calls removed |
| scalar/mul_i32 | 71 | 196 | 8 | 0 | calls removed |
| scalar/mul_i64 | 58 | 176 | 8 | 0 | calls removed |
| scalar/mul_u32 | 71 | 196 | 8 | 0 | calls removed |
| scalar/mul_u64 | 59 | 177 | 8 | 0 | calls removed |
| vec/autovec_loop | 679 | 859 | 10 | 0 | calls removed |
| vec/vec_cmp_i64 | 597 | 510 | 33 | 13 | calls removed |
| vec/vec_cmp_select | 877 | 930 | 169 | 109 | calls removed |
| vec/vec_f32x2 | 326 | 262 | 37 | 16 | calls removed |
| vec/vec_f32x3 | 372 | 316 | 33 | 9 | calls removed |
| vec/vec_f32x4 | 514 | 328 | 71 | 16 | calls removed |
| vec/vec_f32x5 | 1882 | 1437 | 88 | 16 | calls removed |
| vec/vec_f32x8 | 2869 | 2088 | 139 | 16 | calls removed |
| vec/vec_f64x2 | 317 | 251 | 37 | 16 | calls removed |
| vec/vec_i16x4 | 499 | 308 | 78 | 23 | calls removed |
| vec/vec_i16x8 | 834 | 393 | 146 | 23 | calls removed |
| vec/vec_i32x4 | 816 | 582 | 78 | 23 | calls removed |
| vec/vec_i64x2 | 488 | 427 | 44 | 23 | calls removed |
| vec/vec_i8x16 | 1256 | 1545 | 42 | 25 | calls removed |
| vec/vec_lane_ops | 1196 | 1150 | 107 | 39 | calls removed |
| vec/vec_mem | 1784 | 1770 | 78 | 19 | calls removed |
| vec/vec_scalar_mix | 815 | 926 | 67 | 20 | calls removed |
| vec/vec_u16x8 | 834 | 393 | 146 | 23 | calls removed |
| vec/vec_u32x4 | 820 | 586 | 78 | 23 | calls removed |
| vec/vec_u64x2 | 488 | 427 | 44 | 23 | calls removed |
| vec/vec_u8x16 | 1256 | 1545 | 42 | 25 | calls removed |

## aarch64-darwin: 92 goldens changed

| classification | cases |
| --- | --- |
| calls removed | 92 |

calls before 4337, after 1804, cases reaching zero calls 31
frame direction: frame larger 27, frame same 9, frame smaller 32, no frame adjustment 24

| golden | lines removed | lines added | calls before | calls after | class |
| --- | --- | --- | --- | --- | --- |
| bits/logic_u32 | 125 | 343 | 22 | 5 | calls removed |
| bits/logic_u64 | 123 | 328 | 22 | 5 | calls removed |
| bits/shift_i32 | 147 | 355 | 28 | 11 | calls removed |
| bits/shift_i64 | 136 | 327 | 28 | 11 | calls removed |
| bits/shift_u32 | 135 | 346 | 27 | 10 | calls removed |
| bits/shift_u64 | 132 | 326 | 27 | 10 | calls removed |
| call/call_chain | 332 | 598 | 172 | 154 | calls removed |
| call/call_float_regs | 653 | 555 | 87 | 30 | calls removed |
| call/call_indirect | 524 | 635 | 69 | 24 | calls removed |
| call/call_int_regs | 1103 | 1047 | 123 | 24 | calls removed |
| call/call_mixed | 294 | 503 | 53 | 35 | calls removed |
| call/call_rec_edge | 806 | 861 | 87 | 50 | calls removed |
| call/call_rec_large | 427 | 1256 | 134 | 97 | calls removed |
| call/call_rec_small | 369 | 695 | 71 | 30 | calls removed |
| call/call_recursive | 383 | 403 | 53 | 13 | calls removed |
| call/call_ret_large | 652 | 1189 | 138 | 96 | calls removed |
| call/call_ret_small | 488 | 855 | 88 | 42 | calls removed |
| call/call_variadic | 297 | 509 | 158 | 141 | calls removed |
| cmp/branch_nest | 150 | 172 | 3 | 0 | calls removed |
| cmp/cmp_i32 | 114 | 208 | 7 | 0 | calls removed |
| cmp/cmp_i64 | 145 | 231 | 7 | 0 | calls removed |
| cmp/cmp_u32 | 114 | 208 | 7 | 0 | calls removed |
| cmp/cmp_u64 | 145 | 231 | 7 | 0 | calls removed |
| cmp/loop_shapes | 110 | 179 | 6 | 0 | calls removed |
| cmp/short_circuit | 486 | 438 | 36 | 11 | calls removed |
| comptime/ct_runtime_agree | 368 | 562 | 54 | 37 | calls removed |
| convert/bitcast | 72 | 204 | 11 | 0 | calls removed |
| convert/ext_sign | 99 | 282 | 15 | 0 | calls removed |
| convert/ext_zero | 97 | 280 | 15 | 0 | calls removed |
| convert/f2f | 76 | 215 | 11 | 0 | calls removed |
| convert/f2i | 189 | 677 | 54 | 21 | calls removed |
| convert/f2u_high | 134 | 498 | 25 | 0 | calls removed |
| convert/i2f | 174 | 690 | 37 | 4 | calls removed |
| convert/trunc | 71 | 172 | 9 | 0 | calls removed |
| float/arith_f32 | 838 | 635 | 65 | 46 | calls removed |
| float/arith_f64 | 914 | 680 | 65 | 46 | calls removed |
| float/cmp_f32 | 340 | 548 | 20 | 2 | calls removed |
| float/cmp_f64 | 316 | 556 | 26 | 7 | calls removed |
| float/special_f32 | 538 | 598 | 117 | 84 | calls removed |
| float/special_f64 | 588 | 608 | 127 | 94 | calls removed |
| frame/frame_align | 469 | 493 | 79 | 24 | calls removed |
| frame/frame_large | 220 | 445 | 16 | 0 | calls removed |
| frame/frame_spill | 381 | 740 | 32 | 15 | calls removed |
| imm/imm_add | 141 | 345 | 25 | 8 | calls removed |
| imm/imm_addr | 187 | 489 | 21 | 4 | calls removed |
| imm/imm_logic | 128 | 347 | 21 | 4 | calls removed |
| imm/imm_mov | 98 | 275 | 12 | 0 | calls removed |
| mem/addr_modes | 379 | 646 | 36 | 19 | calls removed |
| mem/array_index | 396 | 635 | 36 | 15 | calls removed |
| mem/global_data | 220 | 429 | 35 | 12 | calls removed |
| mem/global_zero | 178 | 397 | 28 | 7 | calls removed |
| mem/loadstore | 218 | 441 | 25 | 8 | calls removed |
| mem/ptr_arith | 264 | 509 | 29 | 8 | calls removed |
| mem/rec_layout | 315 | 630 | 50 | 25 | calls removed |
| mem/uni_layout | 220 | 469 | 32 | 15 | calls removed |
| scalar/arith_i16 | 91 | 281 | 13 | 0 | calls removed |
| scalar/arith_i32 | 85 | 275 | 13 | 0 | calls removed |
| scalar/arith_i64 | 84 | 262 | 13 | 0 | calls removed |
| scalar/arith_i8 | 91 | 281 | 13 | 0 | calls removed |
| scalar/arith_u16 | 91 | 281 | 13 | 0 | calls removed |
| scalar/arith_u32 | 84 | 274 | 13 | 0 | calls removed |
| scalar/arith_u64 | 83 | 261 | 13 | 0 | calls removed |
| scalar/arith_u8 | 91 | 281 | 13 | 0 | calls removed |
| scalar/divrem_i32 | 165 | 342 | 13 | 0 | calls removed |
| scalar/divrem_i64 | 164 | 329 | 13 | 0 | calls removed |
| scalar/divrem_u32 | 83 | 213 | 10 | 0 | calls removed |
| scalar/divrem_u64 | 82 | 203 | 10 | 0 | calls removed |
| scalar/mul_i32 | 48 | 157 | 8 | 0 | calls removed |
| scalar/mul_i64 | 47 | 149 | 8 | 0 | calls removed |
| scalar/mul_u32 | 48 | 157 | 8 | 0 | calls removed |
| scalar/mul_u64 | 47 | 149 | 8 | 0 | calls removed |
| vec/autovec_loop | 632 | 771 | 10 | 0 | calls removed |
| vec/vec_cmp_i64 | 630 | 456 | 33 | 13 | calls removed |
| vec/vec_cmp_select | 1096 | 1026 | 169 | 109 | calls removed |
| vec/vec_f32x2 | 375 | 180 | 37 | 16 | calls removed |
| vec/vec_f32x3 | 390 | 249 | 33 | 9 | calls removed |
| vec/vec_f32x4 | 645 | 248 | 71 | 16 | calls removed |
| vec/vec_f32x5 | 1981 | 1452 | 88 | 16 | calls removed |
| vec/vec_f32x8 | 3389 | 2080 | 139 | 16 | calls removed |
| vec/vec_f64x2 | 398 | 193 | 37 | 16 | calls removed |
| vec/vec_i16x4 | 681 | 318 | 78 | 23 | calls removed |
| vec/vec_i16x8 | 1322 | 541 | 146 | 23 | calls removed |
| vec/vec_i32x4 | 769 | 400 | 78 | 23 | calls removed |
| vec/vec_i64x2 | 505 | 346 | 44 | 23 | calls removed |
| vec/vec_i8x16 | 426 | 599 | 42 | 25 | calls removed |
| vec/vec_lane_ops | 1091 | 717 | 107 | 39 | calls removed |
| vec/vec_mem | 2747 | 2992 | 78 | 19 | calls removed |
| vec/vec_scalar_mix | 698 | 552 | 67 | 20 | calls removed |
| vec/vec_u16x8 | 1322 | 541 | 146 | 23 | calls removed |
| vec/vec_u32x4 | 789 | 420 | 78 | 23 | calls removed |
| vec/vec_u64x2 | 505 | 346 | 44 | 23 | calls removed |
| vec/vec_u8x16 | 425 | 598 | 42 | 25 | calls removed |

## spirv: the module column

`spirv` decodes with `spirv-dis` and has no layer C, so a golden move here is
read more closely than on a machine column. Against `dev` at `da4fe7146`,
layer B failed 84 cells (167 pass, 84 fail, 101 declared skips; every failure
a `layer b o2` cell, layer A green). Calls are `OpFunctionCall`. Lines
removed and added are counted on id-stripped text (every `%id` replaced by
`%_`, the `; Bound:` line dropped), so id renumbering does not inflate them.

Two module-wide facts hold on every one of the 84 goldens, checked by script:
no golden gains an opcode kind it did not already use, and the only opcode
kind that disappears from any module is `OpFunctionCall` (the 31 cases that
reach zero calls). On 35 goldens the opcode multiset delta is exactly
`OpFunctionCall` removals, the bodies of helper functions dropped once they
had no caller left, and copies of the remaining helper bodies. On the other
49 the residual, listed per golden in the last column, comes from three
inlining consequences rather than from the copied bodies themselves: a
same-module callee that grew past the growth bound once its own callees
were inlined (`fold_vec` in the `vec` family, `fold32`/`fold64` in `float`)
is now inlined at fewer of its call sites, so the copied `OpCompositeExtract`
and NaN-check `OpFUnordNotEqual`/`OpSelectionMerge` counts fall while the
callee stays called; an inlined copy specialized to constant arguments folds
part of its body (`call/call_float_regs`, `frame/frame_align`,
`vec/vec_cmp_*`); and single-digit changes in a case's own expressions where
the copied loop sits between two uses (`+OpBitwiseOr 1` in `bits/logic_*`,
`OpAccessChain` and `OpTypePointer` for pointer-typed locals and the dropped
helpers' parameter types). The class rule is unchanged: every golden has fewer
calls, and the per-golden call counts match the `x86_64-linux` rows on 83
of the 84 goldens; `frame/frame_align` reads 76 to 21 against 79 to 24, three
fewer both before and after, the same 55 calls removed.

## spirv: 84 goldens changed

| classification | cases |
| --- | --- |
| calls removed | 84 |

calls (`OpFunctionCall`) before 3880, after 1557, cases reaching zero calls 31
opcode multiset delta fully accounted for by OpFunctionCall removals, dropped helper functions and inlined helper-body opcodes: 35 of 84; residual across the rest: 105 opcodes up, 1517 down

| golden | lines removed | lines added | calls before | calls after | functions before | functions after | class | residual opcode delta |
| --- | --- | --- | --- | --- | --- | --- | --- | --- |
| bits/logic_u32 | 184 | 1090 | 22 | 5 | 3 | 2 | calls removed | +OpBitwiseOr 1 |
| bits/logic_u64 | 178 | 956 | 22 | 5 | 3 | 2 | calls removed | +OpBitwiseOr 1 |
| bits/shift_i32 | 193 | 1094 | 28 | 11 | 3 | 2 | calls removed | none |
| bits/shift_i64 | 200 | 1037 | 28 | 11 | 3 | 2 | calls removed | none |
| bits/shift_u32 | 204 | 1105 | 27 | 10 | 3 | 2 | calls removed | none |
| bits/shift_u64 | 200 | 973 | 27 | 10 | 3 | 2 | calls removed | none |
| call/call_float_regs | 1192 | 1962 | 87 | 30 | 10 | 8 | calls removed | -OpFAdd 2 -OpFMul 2 -OpFSub 2 |
| call/call_int_regs | 1804 | 1946 | 123 | 24 | 13 | 5 | calls removed | -OpAccessChain 38 |
| call/call_mixed | 621 | 1256 | 53 | 35 | 14 | 7 | calls removed | +OpCompositeExtract 3 |
| call/call_rec_edge | 2710 | 3305 | 87 | 50 | 22 | 17 | calls removed | -OpAccessChain 28 |
| call/call_rec_large | 2438 | 3512 | 134 | 97 | 23 | 19 | calls removed | -OpAccessChain 14 |
| call/call_rec_small | 1692 | 2554 | 71 | 30 | 19 | 13 | calls removed | -OpAccessChain 13 |
| call/call_ret_large | 932 | 2162 | 138 | 96 | 26 | 23 | calls removed | +OpAccessChain 13 |
| call/call_ret_small | 1690 | 2272 | 88 | 42 | 28 | 22 | calls removed | +OpAccessChain 8 |
| call/call_variadic | 615 | 1450 | 158 | 141 | 24 | 21 | calls removed | none |
| cmp/branch_nest | 88 | 126 | 3 | 0 | 3 | 1 | calls removed | none |
| cmp/cmp_i32 | 142 | 403 | 7 | 0 | 3 | 1 | calls removed | -OpTypePointer 1 |
| cmp/cmp_i64 | 138 | 399 | 7 | 0 | 3 | 1 | calls removed | -OpTypePointer 1 |
| cmp/cmp_u32 | 150 | 411 | 7 | 0 | 3 | 1 | calls removed | -OpTypePointer 1 |
| cmp/cmp_u64 | 146 | 407 | 7 | 0 | 3 | 1 | calls removed | -OpTypePointer 1 |
| cmp/loop_shapes | 134 | 340 | 6 | 0 | 3 | 1 | calls removed | none |
| comptime/ct_runtime_agree | 299 | 1080 | 54 | 37 | 8 | 5 | calls removed | none |
| convert/bitcast | 344 | 611 | 11 | 0 | 6 | 1 | calls removed | none |
| convert/ext_sign | 318 | 886 | 15 | 0 | 5 | 1 | calls removed | none |
| convert/ext_zero | 314 | 854 | 15 | 0 | 5 | 1 | calls removed | none |
| convert/f2f | 197 | 626 | 11 | 0 | 4 | 1 | calls removed | none |
| convert/f2i | 801 | 2217 | 54 | 21 | 12 | 4 | calls removed | none |
| convert/f2u_high | 452 | 1565 | 25 | 0 | 8 | 5 | calls removed | none |
| convert/i2f | 297 | 2107 | 37 | 4 | 5 | 3 | calls removed | none |
| convert/trunc | 362 | 521 | 9 | 0 | 6 | 1 | calls removed | none |
| float/arith_f32 | 2421 | 1971 | 65 | 46 | 6 | 3 | calls removed | -OpFUnordNotEqual 16 -OpSelectionMerge 16 |
| float/arith_f64 | 2386 | 1940 | 65 | 46 | 6 | 3 | calls removed | -OpFUnordNotEqual 16 -OpSelectionMerge 16 |
| float/cmp_f32 | 718 | 1296 | 20 | 2 | 6 | 2 | calls removed | +OpFOrdLessThan 1 -OpFUnordNotEqual 2 -OpSelectionMerge 2 |
| float/cmp_f64 | 714 | 1457 | 26 | 7 | 7 | 4 | calls removed | +OpFConvert 5 +OpFOrdLessThan 1 -OpFUnordNotEqual 2 -OpSelectionMerge 2 |
| float/special_f32 | 1321 | 1606 | 117 | 84 | 9 | 4 | calls removed | +OpConstant 1 +OpFNegate 1 -OpFUnordNotEqual 16 -OpSelectionMerge 16 -OpTypePointer 1 |
| float/special_f64 | 1610 | 1761 | 127 | 94 | 11 | 4 | calls removed | -OpFUnordNotEqual 16 -OpSelectionMerge 16 -OpTypePointer 1 |
| frame/frame_align | 1133 | 1226 | 76 | 21 | 12 | 6 | calls removed | +OpAccessChain 1 -OpCompositeExtract 54 -OpConstant 1 |
| frame/frame_large | 335 | 934 | 16 | 0 | 5 | 1 | calls removed | +OpAccessChain 3 |
| frame/frame_spill | 670 | 1407 | 32 | 15 | 6 | 3 | calls removed | none |
| imm/imm_add | 335 | 1017 | 25 | 8 | 6 | 2 | calls removed | +OpISub 1 |
| imm/imm_addr | 335 | 1099 | 21 | 4 | 4 | 2 | calls removed | +OpAccessChain 29 |
| imm/imm_logic | 349 | 1023 | 21 | 4 | 6 | 2 | calls removed | +OpBitwiseOr 2 +OpNot 2 |
| imm/imm_mov | 181 | 608 | 12 | 0 | 4 | 1 | calls removed | +OpConstant 2 |
| mem/addr_modes | 902 | 1648 | 36 | 19 | 6 | 2 | calls removed | +OpAccessChain 9 |
| mem/array_index | 1064 | 1758 | 36 | 15 | 7 | 3 | calls removed | -OpAccessChain 6 |
| mem/loadstore | 729 | 1125 | 25 | 8 | 11 | 3 | calls removed | none |
| mem/rec_layout | 435 | 1515 | 50 | 25 | 11 | 7 | calls removed | +OpAccessChain 4 |
| scalar/arith_i16 | 173 | 771 | 13 | 0 | 3 | 1 | calls removed | none |
| scalar/arith_i32 | 173 | 771 | 13 | 0 | 3 | 1 | calls removed | none |
| scalar/arith_i64 | 169 | 723 | 13 | 0 | 3 | 1 | calls removed | none |
| scalar/arith_i8 | 173 | 771 | 13 | 0 | 3 | 1 | calls removed | none |
| scalar/arith_u16 | 179 | 777 | 13 | 0 | 3 | 1 | calls removed | none |
| scalar/arith_u32 | 179 | 777 | 13 | 0 | 3 | 1 | calls removed | none |
| scalar/arith_u64 | 165 | 675 | 13 | 0 | 3 | 1 | calls removed | none |
| scalar/arith_u8 | 179 | 777 | 13 | 0 | 3 | 1 | calls removed | none |
| scalar/divrem_i32 | 172 | 770 | 13 | 0 | 3 | 1 | calls removed | none |
| scalar/divrem_i64 | 166 | 720 | 13 | 0 | 3 | 1 | calls removed | none |
| scalar/divrem_u32 | 142 | 572 | 10 | 0 | 3 | 1 | calls removed | none |
| scalar/divrem_u64 | 133 | 499 | 10 | 0 | 3 | 1 | calls removed | none |
| scalar/mul_i32 | 149 | 467 | 8 | 0 | 3 | 1 | calls removed | none |
| scalar/mul_i64 | 146 | 440 | 8 | 0 | 3 | 1 | calls removed | none |
| scalar/mul_u32 | 175 | 493 | 8 | 0 | 3 | 1 | calls removed | none |
| scalar/mul_u64 | 162 | 432 | 8 | 0 | 3 | 1 | calls removed | none |
| vec/autovec_loop | 472 | 841 | 10 | 0 | 4 | 1 | calls removed | none |
| vec/vec_cmp_i64 | 573 | 514 | 33 | 13 | 6 | 3 | calls removed | -OpCompositeExtract 26 -OpSLessThan 1 -OpSelect 2 |
| vec/vec_cmp_select | 5507 | 6128 | 169 | 109 | 12 | 7 | calls removed | -OpCompositeConstruct 1 -OpCompositeExtract 68 -OpISub 8 -OpSelect 8 |
| vec/vec_f32x2 | 476 | 290 | 37 | 16 | 5 | 2 | calls removed | +OpAccessChain 1 -OpCompositeExtract 32 |
| vec/vec_f32x3 | 406 | 308 | 33 | 9 | 5 | 2 | calls removed | +OpAccessChain 1 -OpCompositeExtract 27 -OpFAdd 1 -OpFMul 1 |
| vec/vec_f32x4 | 772 | 418 | 71 | 16 | 5 | 2 | calls removed | +OpAccessChain 1 -OpCompositeExtract 64 |
| vec/vec_f32x5 | 1685 | 1247 | 88 | 16 | 5 | 2 | calls removed | +OpAccessChain 1 -OpCompositeExtract 80 |
| vec/vec_f32x8 | 2632 | 1942 | 139 | 16 | 5 | 2 | calls removed | +OpAccessChain 1 -OpCompositeExtract 128 |
| vec/vec_f64x2 | 472 | 282 | 37 | 16 | 5 | 2 | calls removed | +OpAccessChain 1 -OpCompositeExtract 32 |
| vec/vec_i16x4 | 799 | 428 | 78 | 23 | 5 | 2 | calls removed | +OpAccessChain 1 -OpCompositeExtract 64 |
| vec/vec_i16x8 | 2810 | 1948 | 146 | 23 | 5 | 2 | calls removed | +OpAccessChain 1 -OpCompositeExtract 144 -OpCompositeInsert 8 |
| vec/vec_i32x4 | 796 | 426 | 78 | 23 | 5 | 2 | calls removed | +OpAccessChain 1 -OpCompositeExtract 64 |
| vec/vec_i64x2 | 499 | 300 | 44 | 23 | 5 | 2 | calls removed | +OpAccessChain 1 -OpCompositeExtract 32 |
| vec/vec_i8x16 | 161 | 993 | 42 | 25 | 5 | 3 | calls removed | +OpAccessChain 2 |
| vec/vec_lane_ops | 2766 | 2927 | 107 | 39 | 10 | 6 | calls removed | -OpCompositeExtract 83 |
| vec/vec_mem | 1187 | 1475 | 78 | 19 | 7 | 4 | calls removed | -OpAccessChain 2 -OpCompositeExtract 58 |
| vec/vec_scalar_mix | 1050 | 1606 | 67 | 20 | 8 | 5 | calls removed | -OpCompositeExtract 33 |
| vec/vec_u16x8 | 2808 | 1946 | 146 | 23 | 5 | 2 | calls removed | +OpAccessChain 1 -OpCompositeExtract 144 -OpCompositeInsert 8 |
| vec/vec_u32x4 | 747 | 445 | 78 | 23 | 4 | 2 | calls removed | +OpAccessChain 1 -OpCompositeExtract 64 |
| vec/vec_u64x2 | 500 | 297 | 44 | 23 | 5 | 2 | calls removed | +OpAccessChain 1 -OpCompositeExtract 32 |
| vec/vec_u8x16 | 161 | 993 | 42 | 25 | 5 | 3 | calls removed | +OpAccessChain 2 |
