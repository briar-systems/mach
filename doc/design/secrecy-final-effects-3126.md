# Secrecy against final machine effects (#3126), phase 1: inventory

Roadmap item N5. This document inventories every backend stage that runs after
the early constant-time walk (`ctvalidate.run`), states what dev validates at
each stage today and with which test, names the concrete late expansions that
can introduce a secret-dependent branch, address or variable-latency use on
x86_64, aarch64 and riscv64, states the supported guarantee for the whole-module
emitter, records what the candidate commit `702bdfb2` adds, and specifies
phase 2 (the post-expansion validator) precisely enough to build.

Line numbers cite dev at `2b44f81c6` unless a function name is given instead.

## 1. Leakage model and target assumptions

The model has three observation channels, taken from `doc/language/secrecy.md`
and implemented identically by the three static checkers (sema gates, the MIR
walk in `ctvalidate.mach`, the asm walk in `asm.mach:ct_scan`):

1. **control-flow trace**: the sequence of taken branches and indirect targets.
   A secret may not decide a conditional branch, a conditional select that the
   target implements as a branch, or an indirect branch or call target.
2. **memory-address trace**: the sequence of effective addresses. A secret may
   not be a base or index of any load or store. Secret *contents* at a public
   address are fine.
3. **operand-dependent latency**: the retirement time of one instruction as a
   function of its operands. Integer divide and remainder and every
   floating-point operation are variable-latency on every target
   (`ct.CT_CAP_ALWAYS`). Integer multiply is variable-latency unless the target
   declares `MachineModel.ct_trust_mul`; a register-count shift is
   variable-latency unless it declares `ct_trust_var_shift`
   (`ct.target_provides`). Today all three ISAs declare `ct_trust_mul = false`
   and `ct_trust_var_shift = true` (`x64/register.mach`, `arm64/register.mach`,
   `riscv/register.mach`).

Taint is a two-point lattice per value (public, secret). A value is secret if
it was declared so at the boundary (a `^` parameter, a load from secret
storage, a secret call result) or computed from a secret operand. The only
downgrade is the explicit `:>T` cast, lowered to `MIR_DECLASSIFY`. Unknowns
fail closed: an opcode without a catalog row, an operand outside the operand
catalog, a register outside the declared range, an inline-asm mnemonic the
grammar has not classified, and an inline-asm data directive are all refused,
never treated as public or constant-time.

Target assumptions the model makes, which no static check can discharge:

- The ISA's documented data-independent timing for the instructions the
  catalog classes `MCT_NONE` (moves, adds, logic, immediate shifts, compares,
  conditional selects without branches, loads and stores at a public address)
  holds on the silicon the program runs on. `ct_trust_*` are declarations, not
  measurements.
- Cache, branch-predictor and prefetcher state are the only microarchitectural
  channels modelled, and only through the address and control traces above.
  Port contention, frequency scaling, and speculative execution are out of
  model.
- A register's secrecy is a property of the register id, not of a width. All
  three ISAs address sub-registers (`al`/`ax`/`eax`/`rax`, `w0`/`x0`) through
  one `regid_make(class, index)` id with a separate width, so an alias write
  taints the whole id. This is sound (an over-approximation) and free.
- Flags are a fourth taint domain on x86_64 and aarch64 (riscv64 has none).
  The generated code never lets flags live across the boundary between two MIR
  instructions (the encoder emits every `cmp`/`test` and its consuming
  `jcc`/`setcc`/`csinc`/`b.cond` from one `MirInstr`), so at MIR level flags
  are invisible; inline asm is the only place flags cross instruction
  boundaries and the asm walk models them. Phase 2 must verify the first
  clause on the emitted stream rather than assume it.
- Memory is one taint domain in inline asm (one monotone bit), but in
  generated code every spill slot is a disjoint, frame-relative extent keyed by
  the vreg it holds (`add_spill_slot(ctx, v, size)`), so slot-granular memory
  taint is sound there. Stores through a data-register base are public-address
  stores of possibly-secret contents; the contents are not tracked through
  memory and a later load from the same address is trusted to carry the
  secrecy the IR gave it (`writes_secret` on the load, `MirVReg.secret` on
  the result).
- `#[oblivious]` is a per-function contract; a call out is a boundary, not a
  hole. ABI inputs are trusted to carry the declared secrecy in the declared
  carriers (this is what `MirAbiInput` records).

## 2. Pipeline position of the early walk

`codegen.mach:codegen_scratch`: `lower.lower_ir` (includes `bulk.expand`
memcpy/memzero loops and phi lowering) → `looprotate.run` → **`ctvalidate.run`**
→ `legalize.run` → `isel.run` → `regalloc.run` → `compute_emission_order_module`
→ `frame.run` → `encode.run`. The whole-module path (`whole_module_image`,
`codegen.mach:237`) runs `ctvalidate.run` on every unit module before
`emit_module`, which is where the SPIR-V refusal fires.

What the early walk establishes, and its tests (`ctvalidate.mach`):

| property | mechanism | test |
|---|---|---|
| taint re-derived as a monotone fixpoint over vregs and pregs | `apply_instr` | `ctvalidate.rederive:phi_join_branch`, `rederive:inlined_return_address`, `rederive:rotated_loop_backedge` |
| secret `MIR_CBR` condition, secret `MIR_CALL` target, secret fused-CBR operand refused | `check_instr` | `control:secret_indirect_call`, `rederive:*` |
| secret memory base or index refused, including a preg base | `mem_base_secret`, `mem_index_secret` | `failclosed:secret_memory_base_preg` |
| variable-latency class gated on the target's trust | `mir.ct_op` + `ct.target_provides` | `varlat:secret_multiply_capability`, `varlat:secret_shift_count_only`, `varlat:secret_divide_and_precolor` |
| `MIR_DECLASSIFY` is the only barrier; `MIR_CALL` clears preg taint | `apply_instr` | `barrier:declassify_stops_taint` |
| unknown opcode, out-of-range register, unknown operand kind refused | `validate_operand_ranges`, `ct_op` error | `failclosed:unknown_opcode_rejected`, `failclosed:out_of_range_registers_rejected`, `failclosed:operand_kind_outside_the_catalog_rejected` |
| inline asm scanned through the grammar's closed table, refused without one | `check_asm` → `tgt.arch.assembly.ct_scan` | `opaque:inline_asm_forbidden`; per-ISA `*.encode.asm_ct_scan:*` |
| whole-module emitter refused | `run` | `scope:whole_module_backend_refused`; end to end `mach.lang.driver:oblivious_spirv_refused` |
| end-to-end oracle | driver | `mach.lang.driver:codegen_terminal_oracle_rejects_mutations` (`CODEGEN_CASE_SECRET_MUL`), `integer_vector_division_accepts_public_and_rejects_either_secret_operand` |

Everything in sections 3 to 9 runs on MIR the walk never re-inspects.

## 3. Legalization (`legalize.mach`)

Fires only for `width > max_alu_width` (i128 on all three ISAs), `!has_mul`
(riscv without M), `!has_var_shift` (none of the three), and wide int↔float
conversion (refused on every 8-byte-lane ISA at `conv_prepare`).

Every expansion is straight-line masked arithmetic: no `MIR_CBR`/`MIR_BR` is
emitted anywhere in the file, memory pieces re-displace the original base
(`addr_displaced`), and variable-count shifts become a barrel of immediate
shifts selected by masks (`emit_shift_barrel`). The one variable-latency
instruction legalization introduces is `emit_divmod_zero_fixup`, which emits
`MIR_DIV_U probe, #1, divisor` as a divide-by-zero trap probe for a wide
divide; it derives from the denominator of a `MIR_DIV_*`/`REM_*` that the
early walk already refused on any secret operand, so it fires only on public
data.

Two real defects, both fixed in this lane:

- `new_vreg` created every temporary with `secret = false` and `alloc_lanes`
  copied `class` but not `secret` from the wide vreg, so the lanes of a secret
  i128 became public vregs after the walk. No leak today (nothing after
  legalization branches or addresses on those lanes), but the seed set the
  register allocator's coalescing guard (`try_coalesce`, `dv.secret !=
  sv.secret`) and any later re-derivation consume was wrong: the fail-open
  shape. Fix: `alloc_lanes` copies the wide vreg's `secret`. Test:
  `legalize:lanes_of_a_secret_wide_value_stay_secret`.
- `expand_mov` erased a wide `MIR_DECLASSIFY` into `MIR_MOV` lanes, so the
  barrier did not survive legalization. Fix: the pieces keep the source opcode
  (every ISA already selects `MIR_DECLASSIFY` at any width). Test:
  `legalize:a_wide_declassify_keeps_its_barrier_per_lane`.

Verdict: **validated by construction, not by test**, on all three ISAs. The
constructive argument (no branch opcode, no new base register, one divide
probe behind an already-refused class) is checked by the existing
`legalize:expands_runtime_shift_as_a_masked_barrel` and `expands_a_wide_divide`
tests only incidentally (they assert no `MIR_BR`/`MIR_CBR` in the pieces).
Phase 2 validates it directly by reaching the emitted stream.

## 4. Instruction selection (`rules.mach`, `<isa>/rules.mach`)

All three packs are retag-only plus a small number of `RULE_EXPAND` rules that
copy the source operands verbatim (`x64` `expand_neg`/`expand_not`, aarch64
vector compare operand swap). No vregs are created. `combine_cbr`/`fuse_pair`
fuses `MIR_CMP_cc v, a, b ; MIR_CBR v` into `MIR_SEL_CBR_cc a, b, T, E`; the
walk already checks the fused forms. There is no `MIR_SELECT` opcode and no
jump-table lowering, so selection never turns a value into a branch.

**The catalog cannot be consulted after selection.** ISA opcode constants
(`x64.Opcode` 0..134, `arm64.Opcode` 0..16 and `0x100..`, `riscv.Opcode`
0..17) are numerically aliased onto generic MIR rows, and `SELECTION_CATALOG`
has no ISA rows, so `mir.ct_op`/`mir.has` on a post-isel instruction return the
aliased generic row: `x64.LEA(2)` reads as `MIR_MUL` (INT_MUL), `x64.IDIV(8)`
as `MIR_AND` (NONE); `arm64.MUL(4)`/`riscv.MUL(4)` as `MIR_DIV_U`;
`arm64.LSL(8)`/`riscv.SLL(8)` as `MIR_AND` (NONE); `riscv.MULHU(17)` as
`MIR_CMP_LT_S`. Today only regalloc consults the catalog post-isel
(`MIRF_SEL_DIVIDE`, `SEL_SHIFT`, `TWO_ADDRESS`, `FLOAT_UNIT`; none set on the
aliased rows), so the hazard is latent. It rules out "run `ctvalidate` again
after selection" as any part of phase 2: the final validator must key on the
emitted `isa.Inst` in the ISA's own opcode space. The namespace collision
itself is #2212 item 10.2 and is not touched here.

Verdict: **validated** (no new value-dependent branch, address or latency
class is introduced) on all three ISAs; **gap** in that the effect table is
unreadable after this stage.

## 5. Register allocation, physical aliases, spill and reload (`regalloc.mach`)

- Rewrite: every vreg operand becomes its assigned preg (`rewrite_instr`);
  `verify_rewritten_operands` refuses a surviving vreg, a preg outside every
  class, a bank violation, and a non-GP memory base or index.
- Coalescing: `try_coalesce` refuses to merge webs whose seed secrecy differs.
  This guard had no test; `regalloc.coalesce_copies:refuses_to_merge_a_secret_with_a_public_web`
  is added in this lane.
- Spill: `emit_reload` emits `MIR_MOV preg(tmp), op_mem_slot(v, 0)` and
  `emit_store` the reverse, width `spill_width`. `tmp` is drawn from the ISA's
  reload scratch list (x86_64 `r8/r9/r10`, aarch64 `x9/x10/x11`, riscv64
  `t2/t3/t4`) and is GP-only (the emitters refuse an FP scratch:
  `regalloc.rewrite_instr:spilled_update_reloads_previous_value_once`).
  FP-bank spills stay as `op_mem_slot` operands folded by the encoder through
  `xmm14`/`v31,v30`/`f31,f30`.
- Slot addressing: a slot operand resolves at encode to `rbp`/`x29`/`s0` (or
  `rsp`/`sp` when `frame.realign != 0`) plus a constant (`slot_base_reg`,
  `frame_slot_offset`), never to a data register. The slot key is the spilled
  vreg's own id, so memory taint per slot is derivable from `MirVReg.secret`.
- What a physical validator must model that the vreg walk did not: the reload
  scratch is a physical alias of the spilled value for the length of one
  instruction; a spilled secret reloaded into `r10` and then used as a base
  would be a secret address the vreg walk cannot see because the MEM operand
  named the vreg. Today this cannot arise (the vreg walk refused the secret
  base before allocation), but nothing after allocation checks it. Test added
  in this lane pinning the facts phase 2 seeds from:
  `regalloc.rewrite_instr:a_spilled_secret_keeps_its_slot_key_through_the_scratch`.

Verdict per ISA: **gap** (timing-preserving by argument, unvalidated by any
post-allocation check) on x86_64, aarch64, riscv64.

## 6. Flags

Generated code never lets flags cross a `MirInstr` boundary: x86_64
`encode_compare` emits `cmp; setcc; movzx` and `encode_fused_cbr` emits
`[mov r11, imm]; cmp; jcc; [jmp]`; aarch64 `emit_flag_cmp` + `csinc` / `b.cond`;
riscv64 has no flags (`slt`/`sltu`/`xor`+`sltiu`, `beq`…`bgeu`). The vreg walk
therefore sees a compare as a value producer and a fused branch as an operand
consumer, which is exact for generated code. Inline asm is the only place a
`cmp` and its `jcc` are separate instructions, and there the asm walk keeps a
flags-taint bit with a measured definer table (`x64.probe`).

Verdict: **validated by construction** on all three ISAs for generated code;
**validated** for inline asm (`x64.encode.asm_ct_scan:*`,
`arm64.encode.asm_ct_scan:*`, `riscv.encode.asm_ct_scan:*`). Phase 2 must
check the construction on the emitted stream (a flags-taint bit across
`isa.Inst` notifications) instead of trusting it.

## 7. Frame insertion (`frame.mach`)

Layout only: slot placement, `frame.omit`, frame distances, reserve check. It
emits no instructions. Prologue and epilogue are emitted by each encoder
(section 9). Verdict: **validated** (nothing to validate) on all three ISAs.

## 8. Relaxation

- x86_64 and aarch64: none. `patch_branch_arm64` patches in place and errors
  when a `b` exceeds ±128 MiB or a `b.cond`/`cbnz` exceeds ±1 MiB.
- riscv64: `relax_function_bounded` inserts text (`encode.insert_text`) for a
  B-type fixup out of ±4 KiB (`write_inverted_guard` + `jal x0, target`, both
  re-notified by `renote_relaxed_branch`) and for a J-type out of ±1 MiB
  (`auipc t0; jalr x0, t0`, re-notified by `renote_trampoline`). Condition
  registers are unchanged and targets are fixed labels; the trampoline clobbers
  `t0`, which is a reserved scratch, never a value carrier. `check_accounted`
  runs after relaxation, so every inserted word is claimed.

Verdict: **validated by construction** (no new value dependence) on riscv64;
not applicable on x86_64/aarch64. Phase 2 reaches the inserted instructions
through their notifications like any other.

## 9. Encoding expansion

Per ISA, every site that turns one `MirInstr` into a sequence containing a
value-dependent branch, a data-register address, or a variable-latency
instruction. All operands are pregs or slots by now (`mir_to_operand` refuses
a vreg).

### 9.1 x86_64 (`x64/encode.mach`)

| class | site | trigger | sequence |
|---|---|---|---|
| branch introduced late | `emit_u64_to_fp` | `MIR_UI_TO_FP`, `src_width >= 8` | `test r11,r11; jl; cvtsi2sd; jmp; shr/and/or; cvtsi2sd; addsd` (branches on the operand's sign bit) |
| branch introduced late | `emit_fp_to_u64` | `MIR_FP_TO_UI`, `width >= 8` | `ucomisd xmm,[rip+2^63]; jb; subsd; cvttsd2si; mov r10,imm64; add; jmp; cvttsd2si` |
| branch (constant trip) | `emit_stack_probe` | prologue when `stack_probe && base_dist > page_size` | `mov r11,total; L: sub rsp,page; mov [rsp],r11; sub r11,page; cmp; ja L` |
| branch (validated operand) | `encode_cbr`, `encode_fused_cbr`, `encode_call` reg form | `MIR_SEL_CBR*`, `MIR_CALL` | `test; jne; [jmp]`, `cmp; jcc; [jmp]`, `call reg` |
| variable latency | `encode_divide` | `MIR_SEL_DIV_S/REM_S`, `DIV_U/REM_U` | `cwd/cdq/cqo; idiv r/m`, `xor edx,edx; div r/m` (no INT_MIN/-1 or zero check) |
| variable latency | `encode_imul` | `MIR_SEL_MUL` | `imul dst, r/m` or `imul dst,dst,imm`, `movabs r11,imm64; imul dst,r11` |
| variable latency (trusted) | `encode_shift` | `MIR_SEL_SHL/SHR_*` | `shl/shr/sar dst, cl` |
| variable latency | `encode_float_*`, `encode_float_conv` | `MIR_F*`, `*_TO_FP`, `FP_TO_*` | SSE scalar and packed arithmetic, `ucomiss/sd; setcc; setp`, `cvt*` |
| address | callee saves, probe, constant pool | prologue/epilogue, float constants | `[rbp±k]`, `[rsp]`, `[rip+disp32]` only |
| memory-to-memory staging | `encode_inst`, `encode_mov` | any mem→mem | value staged through `r11` (or `r10`), addresses unchanged |

Not emitted anywhere by codegen: `cmov`, `bsf/bsr`, `popcnt`, `rep stos`, `rep
movs` (the `FLAG_REP` form exists only for the printer test), jump tables.

The two late branches sit behind `MCT_FLOAT` opcodes, which the early walk
refuses on any secret operand. The divide has no guard on x86_64 (the trap is
hardware `#DE`).

### 9.2 aarch64 (`arm64/encode.mach`)

| class | site | trigger | sequence |
|---|---|---|---|
| branch introduced late | `encode_divide` | `MIR_SEL_DIV_*`, `REM_*` | `cbnz rm,+2; brk` then signed `adds xzr,rm,#1; b.ne; movz x17,#0x8000,lsl 48; subs xzr,rn,x17; b.ne; brk` then `sdiv/udiv`, `msub` for rem (branches on divisor and dividend) |
| branch (validated operand) | `encode_cbr`, `encode_fused_cbr`, `encode_call` reg form | `MIR_SEL_CBR*`, `MIR_CALL` | `cbnz; [b]`, `subs; b.cond; [b]`, `blr` |
| variable latency | `encode_alu3(MUL)` | `arm64.MUL` | `mul` (immediates first materialized into `x16/x17`) |
| variable latency (trusted) | `encode_shift` | `arm64.LSL/LSR/ASR` reg count | `lslv/lsrv/asrv` |
| variable latency | `encode_fp_*`, `encode_fp_conv`, NEON | `MIR_F*`, conversions, `VEC_*` | `fadd…fdiv`, `fcmp; csinc`, `scvtf/ucvtf/fcvtz*`, `fmul/fdiv Vd` |
| large immediate | `emit_movewide_seq` | any imm operand | up to four `movz/movk` into `x16`/`x17` |
| large stack offset | `emit_mem_access`, `emit_fp_mem`, `emit_add_imm` | offset outside the immediate form | `movz/movk x16,#off; ldr/str rt,[base,x16]` (register-offset form with a constant in `x16`) |
| literal pool | `emit_fp_const` | float constant | `adrp x16; ldr Vd,[x16,:lo12:]` |

No relaxation, no stack probe, no loop in the prologue, no `csel` (masked
selects come from legalize/lowering; `csinc` only materializes a compare).

### 9.3 riscv64 (`riscv/encode.mach`)

| class | site | trigger | sequence |
|---|---|---|---|
| branch introduced late | `emit_divide_guard` | `MIR_SEL_DIV_*`, `REM_*` with register divisor | `[addiw t0,rs2,0]; bne rs2|t0,x0,+8; ebreak` then signed `addi t0,rs2,1; bne t0,x0,+skip; emit_min_check(... bne; ebreak)` (branches on divisor and dividend) |
| branch (validated operand) | `encode_cbr`, `encode_fused_cbr`, `encode_call` reg form | `MIR_SEL_CBR*`, `MIR_CALL` | `bne cond,x0; [jal]`, `beq/bne/blt/bltu/bge/bgeu; [jal]`, `jalr ra,rs` |
| branch (relaxation) | `relax_function_bounded` | out-of-range fixup | inverted guard + `jal`, or `auipc t0; jalr x0,t0` |
| variable latency | `encode_alu3` | `riscv.MUL`, `riscv.MULHU` | `mul/mulw`, `mulhu` |
| variable latency | `encode_divide` | `MIR_SEL_DIV_*` | `div/divu/rem/remu[w]` (M extension checked at encode, no software fallback) |
| variable latency (trusted) | `encode_shift` | `riscv.SLL/SRL/SRA` reg count | `sll/srl/sra[w]` |
| variable latency | `encode_fp_*` | `MIR_F*`, conversions | `fadd…fdiv`, `feq/flt/fle`, `fcvt.*` |
| large immediate | `emit_li` | any imm operand | `addi`/`lui+addiw`/`lui…slli…addi` chain into `t0`/`t1` |
| large stack offset | `emit_mem_access`, `emit_fp_mem` | offset outside ±2047 | `li t1,off; add t1,base,t1; ld/sd rt,0(t1)` |
| atomics | none on the MIR path | `lr/sc/amo*` reachable only from inline asm | classified `CT_OP_NONE` by `asm_ct_class`; a retry loop is caught through its branch |

No stack probe, no loop in the prologue.

### 9.4 Verdict for encoding expansion

Every late branch on all three ISAs (`emit_u64_to_fp`, `emit_fp_to_u64`,
aarch64 `encode_divide`, riscv64 `emit_divide_guard`) is reachable only from an
opcode class (`MCT_FLOAT`, `MCT_INT_DIVMOD`) that the early walk refuses on any
secret operand, and every late address is frame-relative, pc-relative, or a
constant materialized into a reserved scratch. So the physical stream is
timing-preserving **by policy** (the walk refused the originating class) and
**by construction** (no expansion creates a value dependence from nothing).
Neither is a validation. Verdict per ISA: **gap** (x86_64, aarch64, riscv64):
no test reaches the emitted instruction stream and re-derives the three
channels over physical registers.

## 10. Inline assembly

Validated through the closed per-instruction effect description
(`asm.Grammar.ct_class` → `ct.AsmClass`; `Mnemonic.writes`/`implicit`/
`implicit_mem`) with register, flags and memory taint domains and fail-closed
unknowns (`asm.mach:ct_check_item`). Tests: `*.encode.asm_ct_scan:*` on each
ISA, including `a_spill_does_not_launder_a_secret` and
`register_indirect_transfer_on_secret_refused`. The x86_64 flags definer table
is measured on the host (`x64.probe`). An asm block in an oblivious function
on an ISA without an assembly grammar is refused (`ctvalidate.check_asm`).

Coverage limit worth stating: the x86_64 and aarch64 grammars carry no
divide, multiply or float mnemonic, so on those ISAs the latency check inside
asm reaches only register-count shifts; riscv64's grammar admits the whole M
extension. Any mnemonic outside the table is refused when a secret reaches it.

Verdict: **validated** on all three ISAs (asm), and this is the effect
description phase 2 reuses for generated code.

## 11. The notification stream today

`encode.sink_claim`/`note_push`/`note_insert_after`/`check_accounted` tie every
emitted byte to exactly one instruction notification, including bytes inserted
by riscv64 relaxation. Two facts phase 2 depends on:

- **The sink exists only under `--emit-asm`.** `encode_driver_asm` attaches an
  `AsmSink` only when `asm_out != nil`; on the binary path `buf.asm == nil`,
  `sink_active` is false, and every encoder skips its `note_*` calls. There is
  no instruction stream to validate on an ordinary build today.
- **The notification payload differs per ISA.** riscv64 pushes an `AsmNote`
  carrying a full `isa.Inst` for every word (`emit_r/i/s/b/u/j` → `note`).
  x86_64 (before `702bdfb2`) and aarch64 only `sink_claim` a byte range after
  rendering; x86_64's `note_inst` has the `isa.Inst` in hand but does not
  record it, and aarch64's printer works on a private `arm64/printer.mach:Inst`
  form record, never an `isa.Inst`. Both printers render through
  `buf.asm.out` and would dereference a nil writer.

## 12. Whole-module emitter (SPIR-V): the supported guarantee

Mach does not emit the instructions that execute; the downstream compiler and
the device's timing are outside the leakage model. The supported guarantee is
**refusal**: a `#[oblivious]` function compiled for a whole-module emitter is a
compile error naming the function and the target (`ctvalidate.run`,
`unverifiable_msg`), reached from `whole_module_image` for every unit module.
Non-oblivious functions carrying secrets compile (the sema gates still hold on
the source). No physical analysis is claimed or pretended.

Tests: `ctvalidate.scope:whole_module_backend_refused`,
`mach.lang.driver:oblivious_spirv_refused`. Verdict: **refused-by-policy**,
validated.

## 13. Verdict matrix

| stage | x86_64 | aarch64 | riscv64 | spirv |
|---|---|---|---|---|
| early walk (`ctvalidate`) | validated | validated | validated | refused-by-policy |
| legalization | construction (fixed: lane seeds, declassify barrier) | same | same | n/a |
| selection | validated; catalog unreadable post-isel | same | same | n/a |
| allocation, aliases, spill/reload | gap | gap | gap | n/a |
| flags | construction (generated), validated (asm) | same | none | n/a |
| frame insertion | validated (inert) | validated | validated | n/a |
| relaxation | n/a | n/a | construction | n/a |
| encoding expansion | gap (u64↔fp branches behind FLOAT) | gap (divide guard behind DIVMOD) | gap (divide guard behind DIVMOD) | n/a |
| inline asm | validated | validated | validated | refused |

"construction" means timing-preserving by an argument over the code, with no
test that reaches the output. "gap" means the same argument holds and phase 2
must replace it with a check.

## 14. What `702bdfb2` adds, and how it was ported

The candidate is the only prior work. It was cherry-picked onto dev and
resolved against #3263 (aggregate secrecy carried through carriers):

- `encode.AsmNote` gains `byte_len`, `loc` and `inline_site`; `note_push`
  computes the extent from the accounted mark and stamps provenance from the
  sink's current `MirInstr` (`sink_set_mir`, now called around
  `encode_mir_instr` on all three ISAs; dev had it only on x86_64).
  riscv64's relaxation re-notes inherit the guard's provenance. This is what
  gives a phase 2 diagnostic a source location for an instruction the MIR never
  contained.
- The x86_64 printer pushes `ASM_NOTE_FUNC`/`BLOCK`/`INST` notes with the
  `isa.Inst`, so x86_64's stream now carries the instruction like riscv64's.
- `mir.MirAbiInput` and `MirFunction.abi_inputs`: for every `#[oblivious]`
  function, `abi.record_abi_inputs` records each incoming carrier (register or
  argument-area offset, transport width and logical width, secrecy, and for
  indirect carriers the pointee's content class and size, plus the hidden
  result pointer). This is the seed set of a physical dataflow: which pregs and
  which stack bytes are secret at entry.
- Resolution against #3263: the candidate hedged non-secret aggregate
  parameters as `known = false` because nested `^` qualifiers were erased at
  the boundary. On dev `value.param(...).secret` is `type.contains_secret`
  (deep) for aggregates, so that case no longer exists; the `known` field was
  removed and `secret` is the deep answer. `ABI_CONTENT_UNKNOWN` is kept for
  pointer parameters, whose pointee secrecy is genuinely invisible at the IR
  boundary. Tests: `abi:incoming_security_facts_preserve_carriers_and_indirect_contents`,
  `abi:incoming_copies_follow_all_parameter_captures`.

Nothing consumes `abi_inputs` yet; the candidate's own message says the
checker is unfinished, and the inventory agrees.

## 15. Phase 2: the post-expansion validator

Sized against sections 5, 9 and 11, the validator is larger than one lane. It
has five parts; each is a drop-in against a frozen interface.

1. **Notification stream on every build.** Split "notify" from "render":
   `AsmSink` with a nil writer records `AsmNote`s and claims bytes without
   rendering; `encode_driver_asm` attaches a sink whenever the module contains
   an oblivious function, not only under `--emit-asm`. The x86_64 and aarch64
   `note_inst` become notify-then-render, with rendering gated on
   `buf.asm.out != nil`. aarch64's printer form record must either be
   translated to an `isa.Inst` at notification (opcode in aarch64's own
   `Opcode` space, registers as regids, memory as base/index/disp) or the
   effect table in part 3 must accept the form record. `check_accounted` and
   `sink.refused` already fail the build if a byte escapes.

2. **Seeds.** `MirAbiInput` (this lane) for entry pregs and argument-area
   bytes; `MirVReg.secret` through `MirVReg.assigned` and `spill_slot` for
   every seeded vreg; `writes_secret` loads mark their destination preg at the
   notification that encodes them; `MIR_DECLASSIFY` (kept as an opcode through
   legalization by this lane) is the barrier. The rewrite keeps no vreg id on
   a `PREG` operand; either `rewrite_instr` retains the origin vreg in
   `MirOperand.vreg` as provenance (`verify_rewritten_operands` only checks
   `kind`) or the seeds are taken at the `MirInstr` the sink names.

3. **Effect description per emitted instruction.** Reuse
   `asm.Grammar.ct_class` (`asm_ct_class(code, flags)`) plus
   `Mnemonic.writes`/`implicit`/`implicit_mem` for every notified `isa.Inst`:
   this is the same closed table inline asm uses (acceptance bullet). Rows are
   extended, not paralleled: x86_64 and aarch64 need rows for `imul/idiv/div`,
   `mul/sdiv/udiv/msub`, the SSE scalar and packed set, `cvt*`, `ucomis*`,
   `setcc`, `movabs`, `cbnz/tbz`, `csinc`, `movz/movk`, `adrp`; riscv64 needs
   `mulh*`, the F/D set and `fcvt.*`. Every emitted opcode without a row is a
   build failure in an oblivious function (unknown effects are non-public and
   non-constant-time). `isa.Inst.clobbers` stays unpopulated; implicit writes
   come from `Mnemonic.implicit` (item 10.3 rules).

4. **The walk.** Per function, in emission order over the notification list:
   register taint keyed by regid (aliases free), a flags bit (x86_64,
   aarch64), slot-granular memory taint for `[fp/sp + const]` extents and the
   asm walk's monotone bit for everything else, `MIR_CALL` clears
   caller-saved taint and re-seeds returns. Refusals with a located diagnostic
   (`AsmNote.loc`, `inline_site`, the rendered instruction): a `cond_flags`
   branch under flags taint, a `cond_reg` branch or indirect target on a
   tainted register, a memory operand with a tainted base or index, a
   variable-latency class on a tainted operand the target does not trust, and
   any unknown row reached by taint. `ctvalidate.run` stays as the early
   diagnostic (its messages name the source construct; the physical walk names
   the instruction).

5. **Mutation controls, one per introduced-late class per ISA**, each proving
   the walk rejects a program the vreg walk accepts: (a) a late branch, by
   seeding the divisor preg of a `MIR_SEL_DIV_U` (aarch64, riscv64) or the
   source of `MIR_FP_TO_UI` (x86_64) secret at the notification level, which
   the vreg walk never sees because the MIR operand is a public vreg
   (`new_vreg`) or the seed is injected after allocation; (b) a late address,
   by spilling a seeded secret vreg and hand-rewriting the reload scratch into
   a memory base (regalloc fixture) and, on riscv64, by relaxing a branch whose
   trampoline `auipc t0` is preceded by a tainted `t0` write; (c) a late
   variable-latency use, by selecting `imul`/`mul`/`mulhu` whose operand
   preg is seeded secret with `ct_trust_mul = false`. Each control has a
   sibling that passes when the seed is public, and one that passes when the
   validator is moved back before allocation (the issue's own control).

The corpus bar for phase 2 is byte-identity: a validator must not move a
golden.

## 16. Acceptance bullets of #3126 mapped to this lane

| bullet | status |
|---|---|
| legalization, selection, allocation, spill/reload, flags, frame effects in final validation | inventory (sections 3 to 9); phase 2 parts 1 to 4 |
| whole-module emitter guarantee defined | section 12, validated |
| encoding and relaxation reach every emitted instruction | section 11 (stream exists under `--emit-asm` only); phase 2 part 1 |
| late secret-dependent branches, addresses, variable-latency uses rejected by mutation controls | phase 2 part 5; classes and sites named in section 9 |
| inline asm uses the same closed effect descriptions | section 10, validated; phase 2 part 3 reuses the table |
| unknown effects are not public or constant-time by default | early walk and asm walk, validated (`failclosed:*`, `ct_check_item`); phase 2 part 3 |
| leakage model and target assumptions documented | section 1 |
| secret-safe boundary tests with mach-std#550 | `MirAbiInput` recorded (section 14); consumer is phase 2 part 2 |
