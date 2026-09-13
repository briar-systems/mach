# Backend description census (#2212, roadmap N1)

Source census of the backend on `dev` at `614cc4ec`, taken for epic #2212
("complete instruction, effect and target ownership contracts"). It answers,
for each retained ISA and the SPIR-V emitter, four questions:

1. Where is each instruction form, operand class, effect and encoding described,
   and which description is authoritative versus duplicated or derived?
2. What is the VReg/PReg identity model, and where can a raw integer still stand
   in for one?
3. Who owns a resolved `Target`, who borrows it, and where can a borrow outlive
   its owner?
4. Which interfaces do N3 to N6 consume, and therefore freeze?

Every claim names a file and a function or constant. Counts are grep counts
over production code, with the pattern stated where it is not obvious; lines
inside `test "..."` blocks and `t_`/`ut_` helpers are excluded the same way
`test/census.sh` excludes them.

Gap classes used throughout:

- **ADM** authoritative description missing: a fact the backend relies on that is
  stated nowhere as data, only implied by control flow.
- **DUP** duplicated description: one decision stated at two or more sites that
  must agree and are kept in agreement only by tests or by luck.
- **NIE** nominal identity not enforced: a register, class or index identity
  carried as a bare integer where the type checker could have refused a mix-up.
- **BLU** borrow lifetime unproven: a pointer into registry storage whose
  validity depends on an ordering nothing checks.

Out of scope: mos6502 (withdrawn from the v5 matrix, `doc/design/v5-release-contract.md`
"Retained target matrix"), the object writers (N2, #3113), and any codegen change.

## 1. The shared contract every ISA is measured against

### 1.1 Instruction currency

- `src/lang/target/isa.mach:86` `rec Inst { opcode: u16; dst, src1, src2: Operand;
  size: u8; flags: u16; clobbers: u32; src_line: u32; src_file: str }` is the
  shared instruction value. `src/lang/target/isa.mach:71` `rec Operand` carries
  registers as `reg: i32`, memory as `base: i32` / `index: i32`.
- `src/lang/target/isa.mach:1698` `inst_blank` is the only zeroing constructor
  (#2215 closed).
- `Inst.clobbers` has exactly one writer (`inst_blank`, `= 0`) and **zero readers**
  repo-wide (`grep '\.clobbers\b'` excluding the `assembly.clobbers` callback).
  It is a dead field that reads as "no clobbers" for every instruction. **ADM**
  for all four register machines: any consumer that starts reading it fails open.
- Register identity inside `Operand.reg`: `src/lang/target/isa.mach:842`
  `regid_make(class_id, index) = (class << 8) | index`, `:846 regid_class`,
  `:850 regid_index`. Only two class ids exist, `:56 REG_CLASS_ID_GP = 0` and
  `:57 REG_CLASS_ID_XMM = 1`; the third `RC_FLAGS` class both x86_64 and aarch64
  declare has no regid class id.

### 1.2 Machine-opcode catalog (MIR level)

- `src/lang/be/codegen/mir.mach:222` `rec MirOpDescriptor { op; name; class;
  flags: MirFlags; ct_class: MirCtClass; reg_operands; operand_banks: str; fused_cbr }`,
  `:233` `SELECTION_CATALOG_COUNT = 89`, `:235` `SELECTION_CATALOG` with 89 rows.
- Effects declared per MIR opcode: `MirFlags` at `:174-198` (terminator, compare,
  two-address, memory `MIRF_MEMORY`, addressed, address-producer, float-unit,
  shift/divide classes), constant-time class `MirCtClass` at `:202-208`
  (`MCT_INT_MUL`, `MCT_INT_DIVMOD`, `MCT_VAR_SHIFT`, `MCT_FLOAT`), and the
  per-operand bank pattern `operand_banks` (grammar at `:606-623`
  `validate_bank_pattern`, read by `:635 operand_bank`).
- Counts over the 89 rows: 90 `operand_banks:` fields (89 rows + the unknown
  descriptor), 4 rows carry `MIRF_MEMORY`, 29 rows carry a non-`MCT_NONE`
  constant-time class.
- `:625 validate_operand_banks` runs from `src/lang/target/isa.mach:1121`
  `descriptor_valid`, so a malformed bank pattern is refused at registration.
- This catalog is the **only** authoritative effect description in the backend
  for the codegen path. It is keyed by MIR opcode, not machine opcode; after
  `rules.select` retags a MIR opcode to an ISA opcode (`x64.MOV`, `arm64.LDR`,
  `riscv.ADD`), the row is found through the ISA opcode only where the ISA's own
  `Opcode` space is a subset of the MIR space (see each ISA section).
- Instruction-level memory effects at MIR: `src/lang/be/codegen/mir.mach:792`
  `MirInstr.memory_flags` (`MEMORY_VOLATILE`, `:790`) and `writes_secret: bool`.

### 1.3 Target family split

- `src/lang/target/isa.mach:544` `rec IsaVTable { id; name; elf_machine;
  pointer_width; reloc: *RelocSeam; machine: *RegMachine; emitter: *ModuleEmitter;
  model: *MachineModel; defs: *TargetDefs; envs; env_count }`.
- `:365 rec RegMachine` (select, encode, is_reg_move, is_trap_terminator,
  reserved/reload-scratch register lists, scratch/frame/stack/div/shift register
  ids as `i32`, optional `assembly: AssemblyCapabilities`, `dwarf_reg`, `cv_reg`,
  `frame_dist`) built by `:558 reg_machine(...)` positionally, so an omitted slot
  is an arity error.
- `:390 rec ModuleEmitter { emit_module: EmitModuleFn }` built by `:738 module_emitter`.
- `:1113 descriptor_valid` and `:1267 register` refuse a descriptor that has both
  or neither family, a partial assembly group, or a reloc seam on an emitter.
  `:1638 backend_family` panics on a contradictory descriptor that `register`
  should have refused.
- `:157 rec MachineModel` is the declared-capability record (pointer/GPR widths,
  register classes, vector shape, `packed_gaps`, `ct_trust_mul`,
  `ct_trust_var_shift`, `has_mul`, `has_float`, `flen_bits`, `auto_vectorize`),
  read through `:220 packed_width`, `:244 packed_lane_cap`, `:259-277` the
  `moves_*`/`fits_vector_register` predicates.

### 1.4 Inline-assembly effect model (shared)

- `src/lang/target/isa/asm.mach:66` `rec Mnemonic { name; code; form; writes;
  implicit; flags; words; implicit_mem }` is the per-mnemonic row every ISA's
  grammar publishes; `:179 rec Grammar` binds rows, directives, `decode/parse/emit/
  patch/slot/writes_sp/no_fall/ct_class/note_bytes/decl_reg` callbacks and the
  `all_gp`/`all_fp` clobber bounds.
- `src/lang/ct.mach:70` `rec AsmClass { op: CtOp; known; cond_flags; cond_reg;
  flags_stated; writes_flags; defines_flags; opaque_flags; reads_flags }` with the
  constructors `:82 asm_unknown` (conservative default: unknown, writes flags),
  `:96-157 flags_untouched/defined/written/opaque/read`, `asm_op`,
  `asm_branch_flags`, `asm_branch_reg`, `untouched_branch_reg`.
- The four hooks a register machine registers: `src/lang/target/isa.mach:283`
  `AsmClobbersFn: fun(str, *u32, *u32)`, `:285 AsmReturnsFn: fun(str) bool`,
  `:287 AsmCtScanFn: fun(str, *str, u32, bool, bool, *A.Allocator)`, and the
  `EmitAsmFn`; installed through `:586 with_assembly`, consumed at
  `src/lang/be/codegen/regalloc.mach:2089` `asm_instr_clobbers` and
  `src/lang/be/codegen/ctvalidate.mach:334` (`tgt.arch.assembly.ct_scan`).
- The effect description per asm instruction is therefore `ct.AsmClass` as
  returned by each ISA's `asm_ct_class(code, flags)`. It models flags, branch
  kind and latency class; it does **not** model memory read/write or clobbers
  (those come from `Mnemonic.writes`/`implicit`/`implicit_mem` via
  `src/lang/target/isa/asm.mach:892 fold_clobbers`).

### 1.5 Byte accounting (the working conformance invariant)

- `src/lang/be/codegen/encode.mach:290 sink_claim`, `:338 sink_claims`,
  `:343 sink_unaccounted`, `:226 note_inst_count`. Each ISA calls a
  `check_accounted` after the prologue and after every MIR instruction
  (x86_64 `src/lang/target/isa/x64/encode.mach:2263`, aarch64
  `src/lang/target/isa/arm64/encode.mach:2882`, riscv
  `src/lang/target/isa/riscv/encode.mach:2393`, which also checks after
  relaxation). Any byte not claimed by exactly one printer notification fails
  the build naming the opcode.

## 2. x86_64

Files: `src/lang/target/isa/x64.mach`, `x64/{encode,printer,register,reloc,rules,probe}.mach`.

| aspect | authoritative site | duplicates / derived | gap |
|---|---|---|---|
| opcode set | `x64.mach:54 def Opcode: u16`, `:56-194` 135 constants (`^pub val [A-Z0-9_]*: *Opcode`) | `x64/printer.mach:257 mnemonic_of` (134 arms), `x64/encode.mach:1616 encode_inst_one` (49 arms), `x64/encode.mach:4096 X64_MNEMONICS` (60 rows, asm parser only) | DUP x3 |
| instruction form / operand shape | none as data; only the 33 per-opcode encoder functions `(mi: *isa.Inst, buf)` in `x64/encode.mach` | range predicates `x64.mach:205 is_fp_opcode`, `x64/encode.mach:362 is_jcc`, `:366 is_setcc`, `:381 is_sse_mov`, `:385 is_sse_alu` depend on hand-assigned contiguity (`SETP/SETNP` already break it, `:367`) | ADM |
| instruction value | `isa.Inst` consumed by encoder and printer alike (`x64/printer.mach:45 note_inst(buf, mi: *isa.Inst)`) | none | ok |
| encoding | 66 `fun encode_*` + 41 `fun emit_*` in `x64/encode.mach`; entry `:2169 encode_x64` → `:2198 encode_function` → `:3981 mir_to_inst` → `:1581 encode_inst` | SSE opcode-byte selection stated 4 ways: `:1431 sse_alu_opcode`+`:1439 sse_alu_prefix`, `:2530 float_alu_opcode`, `:3230 packed_prefix66`+`:3236 packed_opbyte`, `:3425 packed_arith_opcode` | DUP |
| width ladder | none | `x64/encode.mach:93 size_to_w`, `:3969 effective_width`+`:3974 width_flags`, `:4917 width_flags_for`, `:4250 x64_size_keyword`, `x64/printer.mach:248 size_prefix` (five copies; the last two disagree on the 16-byte row) | DUP |
| condition codes | none | `x64/encode.mach:346 cc_to_x86`, `:2449 mir_setcc_opcode`, `:3135 float_setcc_opcode`, `:3519 i64_ordering_setcc`, `:3911 fused_jcc_opcode`, `x64/printer.mach:279-300` (six copies) | DUP |
| flags effect (codegen path) | none; `x64/encode.mach:2458 emit_flag_cmp` / `:2490 encode_compare` / `:3859 encode_cbr` / `:3920 encode_fused_cbr` assume the adjacent CMP set the flags | asm path: `x64/encode.mach:6910 asm_ct_class` (53 opcodes), `:7073 probed_rows` (26 measured rows), `x64/probe.mach:28 run` (26 asm bodies) | ADM (codegen), DUP x3 (asm) |
| memory effect | none anywhere in the x64 tree (`is_load`/`is_store`/`MIRF_MEMORY`: 0 hits); inferred from `Operand.kind == OPK_MEM` at `x64/encode.mach:113` | asm: `Mnemonic.implicit_mem` on 4 rows (`push/pop/pushfq/popfq`) only | ADM |
| sub-register aliasing | none as data; width rides `Operand.size` (`x64/encode.mach:108 needs_rex_for_byte_reg`, `:242 emit_prefix_reg`) and the allocator sees one 16-entry GP file (`x64/register.mach:119`) | naming only: `x64.mach:212 reg_name` (64 rows) vs `x64/encode.mach:4166 x64_reg_region` (56 rows) | ADM, DUP |
| clobbers | `Inst.clobbers` never written (see 1.1); asm: `x64/encode.mach:4900 asm_clobbers` over `Mnemonic.implicit` | — | ADM |
| latency provenance | codegen: catalog `ct_class` (1.2); asm: `asm_ct_class` | — | ok |
| register file | `x64/register.mach:117-127` classes `gpr/16`, `xmm/14`, `flags/1` | `x64.mach:46 FP_COUNT = 16` vs class len 14; `x64/encode.mach:2522-2523` `FLOAT_SCRATCH0/1 = (1 << 8) \| 14/15` (raw class id and index) | DUP, NIE |
| XMM identity | `x64.mach:28-43 XMM0..XMM15` are bare indices, not regids; callers must wrap with `regid_make` (`x64/register.mach:244`) while `RAX..R15` are usable as regids directly | — | NIE |
| MIR preg → isa regid | `x64/encode.mach:4014 mir_to_operand`: bare `op.preg::i32` (5 such casts in production) | — | NIE |
| rules | `x64/rules.mach:80 RULES` 60 rows, 1 guard (`:36 load_widens`), 2 expanders; names no physical register | — | ok |
| asm hooks | `x64/register.mach:152` registers all four; grammar `x64/encode.mach:4819 x64_grammar` | `x64/encode.mach:4875 x64_decl_reg` yields `(bank u8, index u32)`, a third register-identity encoding beside regid and MIR preg | NIE |
| independent control | `x64/probe.mach` executes the 26 probed rows on the host CPU (`:437`); layer B corpus goldens `test/golden/x86_64-linux` | — | ok |

Raw-integer register sites in production (31): 16 in `x64.mach:292-307 fp_reg_name`,
2 in `x64/encode.mach:2522-2523`, 10 `isa.make_mem(... -1 ...)` "no base" sentinels
(`x64.mach:203 MEM_BASE_ABS = -2` is named, `-1` is not), `x64/encode.mach:2137
is_rip_mem`, `:4399/:4407 op.reg = -1`.

## 3. aarch64

Files: `src/lang/target/isa/arm64.mach`, `arm64/{encode,printer,register,reloc,rules}.mach`.

This is the one register machine not on `isa.Inst`. `arm64/printer.mach:63`
declares its own `pub rec Inst { base: u32; form: u8; dst, src1, src2, src3:
isa.Operand; flags: u16; vec_lane: u32; sym_mod: u8; target: u8; block: u32 }`;
`grep isa.Inst src/lang/target/isa/arm64/` is empty. The encoder builds bare
words through 25 `pack_*` functions and notifies the printer with a
`printer.Inst` at 29 sites (`printer.inst(` = `printer.note_inst` = 29, 1:1;
`emit_word` = 32 call sites, all paired, 0 unpaired).

| aspect | authoritative site | duplicates / derived | gap |
|---|---|---|---|
| opcode set | four disjoint enumerations: `arm64.mach:82-115` 31 selected opcodes (`def Opcode: u16`), `arm64/printer.mach:23-44` 22 `FORM_*`, `arm64/encode.mach:31-172` 154 base-word constants, `arm64/encode.mach:2984-3006` 23 `A64P_*` asm patterns + `:3011-3057` 47 `A64M_*` | — | DUP x4 |
| instruction form | `printer.Inst.form` (22 forms), rendered by `arm64/printer.mach:147 render_body` (22 arms) | encoder has no form enum; `arm64/encode.mach:490 Alu3Shape`/`:508 alu3_shape` (25 arms), `:654 LdStShape` with `:668 ldst_shape`/`:683 ldur_shape`/`:695 ldst_reg_shape` re-derive shape from base words | DUP |
| memory-access rows (#2766) | `arm64/encode.mach:424 rec A64Access`, `:442 int_access` (4 widths × load/store), `:462 fp_access` (5 widths × load/store); a width with no row is refused | `arm64/reloc.mach:195 ldst_lo12_scale` and `:205 ldst_instruction_scale` re-decode the scale out of emitted word bits | DUP |
| load vs store | `arm64/encode.mach:707 ldst_loads` (31-term disjunction) | `arm64/printer.mach:519 ldst_is_load` derives it from the first letter of `ldst_mnem(base)`; `:554 render_ldstp` repeats the trick; `arm64/encode.mach:781 emit_ldstp` has its own 4-term disjunction | DUP x3 |
| mnemonics | none as data | ten printer if-ladders (`arm64/printer.mach:174 alu3_mnem` 27 arms over 63 base words, `:255 addsub_imm_mnem`, `:486 ldst_mnem` 15 arms over 60 words, `:571 neon_3same_mnem`, `:597 neon_arrangement`, `:647 neon_element`, `:844 barrier_name`, `:892 pstate_field_name`, `:1015 sym_mod_prefix`, `:1133 cond_name`) keyed by 201 distinct hex literals that never reference the 154 named encoder constants; `A64_MNEMONICS` name column (`arm64/encode.mach:3067`, 48 rows) | ADM, DUP |
| condition codes | `arm64/encode.mach:100-114 CC_*` | `:2351 cmp_cond`, `:2465 fused_cbr_cond`, `:1846 fp_cond`, `arm64/printer.mach:1133 cond_name`; `printer.mach:52 FLAG_COND = 0x000F` is the only statement of the field width on the printer side | DUP |
| flags effect | codegen: implicit in `arm64/encode.mach:2389 emit_flag_cmp` → `:2411 encode_compare` → `:643 emit_cset` (writes `i.flags = cc`); asm: `arm64/encode.mach:7276 asm_ct_class` (47 codes + `A64P_BCOND`) | — | ADM (codegen) |
| memory effect | load/store bit only (above); no per-instruction read/write description | — | ADM |
| physical aliasing (w/x, b/h/s/d/q, v arrangement) | none as data | `arm64/printer.mach:1108 emit_reg_id` (size → prefix), `:597 neon_arrangement`, `:647 neon_element`; encoder `:1012 pick(use64, ...)`, `:1017 is64`, `:1897 neon_size`, `:1909 neon_lane_size`, `:2023 neon_3same_size_ok` | ADM, DUP |
| clobbers | `Inst.clobbers` unused (printer.Inst has no such field); asm: `arm64/encode.mach:3059-3063 A64_CALLER_SAVED/A64_SMCCC_CLOBBER/A64_LR_READ` in `Mnemonic.implicit`, `:4025 asm_clobbers` | — | ADM |
| latency provenance | catalog `ct_class` (1.2); asm `asm_ct_class` (LSLV/LSRV/ASRV → `CT_OP_VAR_SHIFT`) | — | ok |
| register file | `arm64/register.mach:79,85-95` classes `gpr/31`, `vector/30`, `flags/1`; `arm64.mach:6-43 X0..X30 + IP0/IP1/FP/LR/SP`, `:45-76 V0..V31` as bare `i32` numbers | `arm64.mach:80 VECTOR_COUNT = 30` vs `V0..V31` (32): `V30/V31` reserved by omission for `arm64/encode.mach:116-117 FP_SCRATCH0/1`, stated nowhere | DUP |
| number ↔ regid | `arm64/printer.mach:101 gp_id`, `:97 gp`, `:105 vec` (no `vec_id`), `arm64/encode.mach:1051 req_gpr`, `:1665 req_vec`, `:544 shaped_reg` | `arm64/register.mach:61 arm64_dwarf_reg` re-derives DWARF numbering with `DWARF_V0 = 64` | NIE |
| raw register literals | `arm64/encode.mach:92-95 ZR/SP_REG/SCRATCH0/SCRATCH1 = 31/31/16/17` restate `arm64.SP/IP0/IP1`; `:2601-2602`, `:2629`, `:2631`, `:2648`, `:2655`, `:2689`, `:2696` use `29`/`30` for `arm64.FP`/`arm64.LR`; `:3200-3203` `op_reg(31, 8)`; `:3215 n > 30`, `:4021 n > 31` parser bounds; `arm64/printer.mach:1096 is_reg31`, `:1118` | 24 sites | NIE |
| byte accounting | `arm64/printer.mach:127 INST_BYTES = 4`, `:129 note_inst` claims a fixed 4-byte back-off, correct only for one-word emitters; `arm64/encode.mach:998 read_word`/`:1005 write_word` patch branches without re-notifying | — | ok (guarded by `check_accounted`) |
| rules | `arm64/rules.mach:81 RULES` 87 rows, 3 guards, 5 expanders; names no physical register | `arm64/rules.mach:369` restates `87` as a literal in the coverage test (deliberate tripwire) | ok |
| asm hooks | `arm64/register.mach:122 with_assembly` (4), `:124 with_dwarf_regs`, `:125 with_frame_dist`, `:130 with_local_got_kinds` | — | ok |
| independent control | layer B goldens `test/golden/aarch64-linux` (external disassembler) | no in-tree reference decoder (riscv has one) | — |

## 4. riscv64 and riscv32

Files: `src/lang/target/isa/riscv.mach`, `riscv/{inst,encode,printer,register,reloc,rules,attributes,refcore}.mach`.
One backend, two registrations: `riscv/register.mach:92 register_riscv64` and
`:97 register_riscv32` are one-line wrappers over `:102 build_riscv(reg, arch_id,
name, xw, ...)` and differ only in arch id, name and `xw` (8 vs 4). Both share
`rules.select`, `encode.encode_riscv64`, the reloc seam, the attributes hooks
and `model.flen_bits = 64` (`:175`, unconditional); only `xbank_widths` is
xlen-gated (`:180-181`).

| aspect | authoritative site | duplicates / derived | gap |
|---|---|---|---|
| opcode set | `riscv/inst.mach:3 def MachOp: u16`, 140 opcodes (`MOP_NONE=0` … `CSRRCI=140`, `:170 MOP_LAST`) | `riscv.mach:97-116 def Opcode: u16` (18 selected opcodes, a second namespace bridged at `riscv/encode.mach:2374` and dispatched at `:2072 encode_mir_instr`); `riscv/encode.mach:2573 RV_MNEMONICS` 107 rows | DUP |
| per-opcode form (format, opcode7, funct3, funct7) | **no table in `inst.mach`** (0 `rec`s); the forward description is `riscv/encode.mach:2746 rv_fields` (70 arms + AMO range, absent for every FP arithmetic/convert/move opcode) | inverse: `:383 classify_r`, `:419 classify_fp`, `:459 classify_amo`, `:478 classify_i`, `:529 classify_s`, `:545 classify_b`, `:556 classify_u`, `:562 classify_j` (140 distinct returns); MIR call sites hardcode `F3_*`/`F7_*` a third time (111 `emit_[risbuj](st, ...)` sites, e.g. `:933`, `:1371`) | ADM, DUP x3 |
| operand shape | `riscv/encode.mach:247 shape_of` (23 arms → 19 `SH_*`), consumed by `:215 build_inst` to squeeze every form into `isa.Inst.dst/src1/src2` | — | ok |
| instruction value | `isa.Inst` built by `build_inst` and printed by `riscv/printer.mach:292 mnemonic` (140 rows) | — | ok |
| xlen admission | `riscv/inst.mach:182 xlens` (10-branch range ladder), `:204 admits` | `admits` is called at exactly one site, `riscv/encode.mach:3221` (inline asm). The MIR path enforces xlen independently by width: `:650 alu_opcode`, `:655 alu_imm_opcode`, `:661-663` inside `int_mem_funct3` | DUP (and `xlens` unenforced for codegen) |
| memory-access rows (#2766) | `riscv/encode.mach:660 int_mem_funct3` (4 widths × load/store, refuses `width > xl`), `:1521 fp_mem_funct3` (2 rows, no xlen) | the same funct3 values restated per named opcode inside `rv_fields` (`:2801-2812`) and inverted in `classify_i`/`classify_s` | DUP |
| load vs store | none as a predicate over `MachOp`; range arithmetic in `shape_of` (`:253-254`) | printer re-derives store-ness from `mi.dst.kind == OPK_MEM` (`riscv/printer.mach:77 render_operands`); encoder threads an unrelated `is_load: bool` | ADM, DUP |
| flags effect | n/a (no flags register; `riscv/register.mach:201-203` sets `div_reg/div_hi_reg/shift_count_reg = -1`) | — | ok |
| ordering (aq/rl) | `riscv/inst.mach:208-209 FLAG_AQ/FLAG_RL` on `Inst.flags`, set at `riscv/encode.mach:277` from `:369 amo_ordering_flags(funct7)` | asm parser has its own `RVF_AQ/RVF_RL = 0x0100/0x0200` (`:2566-2567`, `:2694 rv_decode_amo`); the raw funct7 bits are a third spelling | DUP x3 |
| fence | `FENCE`/`PAUSE` are `SH_NONE`; the pred/succ sets are fixed immediates at `riscv/encode.mach:3237-3238` and never modeled | — | ADM |
| physical aliasing | none for GPRs; FPRs are one flat 64-bit class; NaN boxing exists only in the reference core (`riscv/refcore.mach:703 box_s`, `:707 unbox_s`) | — | ok |
| clobbers | `Inst.clobbers` never written; asm: `riscv/encode.mach:3520 asm_clobbers`, `:138 CALLER_SAVED_GP = 0xF003FCE2` as one raw bitmask not derived from the ABI module | — | ADM, DUP |
| latency provenance | catalog `ct_class`; asm `riscv/encode.mach:5880 asm_ct_class` (94 MachOps named; 13 M-extension mnemonics variable-latency, 6 variable shifts) | — | ok |
| register file | `riscv/register.mach:102-115` classes `gpr/GPR_COUNT`, `fpr/FPR_COUNT`; `riscv.mach:93 GPR_COUNT = 32`, `:95 FPR_COUNT = 30` | `riscv.mach:60-91 F0..F31` and `riscv/printer.mach:256 fp_name` cover 32; `F30/F31` reserved by omission for `riscv/encode.mach:113-114 FP_SCRATCH/FP_SCRATCH2`, stated nowhere | DUP |
| ABI register names | none in `register.mach` | `riscv/printer.mach:220 gp_name`/`:256 fp_name` (index → name, 64 rows) vs `riscv/encode.mach:2474 asm_reg_index_region` (name → index, 33 rows); `:3502 asm_fp_reg_index` does derive from `fp_name` | DUP |
| raw register literals | `riscv/encode.mach:131-136 RZERO/RRA/RSP/RFP/SCRATCH/SCRATCH2 = 0/1/2/8/5/6` restate `riscv.ZERO/RA/SP/FP/SCRATCH_REG/SCRATCH_REG2` (`riscv.mach:39-58`); `:113-114` restate `F31/F30` | 170 production literal sites in total (64 in `riscv.mach`, 64 in `printer.mach`, 41 in `encode.mach`, 1 in `register.mach`) | NIE |
| number ↔ regid | `riscv/encode.mach:207 gpr(idx)` (class 0 implicit), `:211 fpr(idx)`, `:671 req_gpr`, `:1500 req_fpr`; `riscv/register.mach:75 riscv64_dwarf_reg` | — | NIE |
| rounding mode | `riscv/encode.mach:101-108 RM_*`, `:1762 int_to_fp_rm` | `riscv/printer.mach:71 rounding_operand` re-derives `rtz` from the opcode range | DUP |
| encoding | `riscv/encode.mach:152-179 pack_r/i/s/b/u/j` (12 production call sites, 0 with literal funct fields); relaxation `:2226 relax_function_riscv64` → `:2245 relax_function_bounded` with bound `:2231 relax_round_bound = 2n+1`; no compressed encoding (`:185 WORD_SIZE = 4`) while `has_compressed` still feeds `EF_RISCV_RVC` (`riscv/register.mach:69`) and the `c2p0` attribute (`riscv/attributes.mach:569`) | — | ADM (claims C it cannot emit) |
| extension vocabulary (#3127) | emitted string is built at `riscv/attributes.mach:558 riscv64_build_attributes` (`i m a f d [c]`, `:563-569`) | the canonical single-letter order `:61 SINGLE_ORDER = "mafdqlcbkjtpvn"` with a hard-coded `i < 14` bound at `:80 single_rank`; classes at `:447 ext_class` and `:180 parse_arch` use raw ASCII codes | DUP |
| rules | `riscv/rules.mach:21 RULES` 64 rows (33 retag, 31 pass, 0 expand), 5 guarded rules over 2 guard fns; `guard_identity_copy` (`:139`) reads `tgt.model.gpr_width` (#2865 fixed); names no physical register | `:118 name: "riscv64"` is shared by the riscv32 registration (read nowhere in production) | ok |
| independent control | `riscv/refcore.mach:897 exec_reference`, an RV32IMFD interpreter that imports nothing from `inst`/`encode`/`isa` and decodes raw words (~97 instructions; refuses RV64-only, AMO, CSR, system, `:1064`); layer B goldens `test/golden/riscv64-linux`, `riscv32` | — | ok |

## 5. SPIR-V (whole-module emitter)

Files: `src/lang/target/isa/spirv.mach`, `spirv/{defs,emit,cfg,types,register}.mach`,
`src/lang/target/abi/spirv.mach`, `src/lang/target/of/spv.mach`.
Registered through `spirv/register.mach:75 isa.module_emitter(emit.emit_module)`;
no select, encode or regalloc hook. The family fork is honoured at
`src/lang/be/codegen.mach:144` (`emits_whole_module` → `whole_module_image`) and
`src/lang/be/codegen/ctvalidate.mach:28` (an `oblivious` function on an emitter
is refused outright, not validated).

| aspect | authoritative site | duplicates / derived | gap |
|---|---|---|---|
| opcode set | `spirv.mach:32-122` 91 `OP_*` constants (no operand count, result-type flag or word count per row) | `spirv/defs.mach:57-59` restate `148`, `86`, `87` as literals beside `OP_DOT`, `OP_SAMPLED_IMAGE`, `OP_IMAGE_SAMPLE_IMPLICIT_LOD` | DUP |
| published op/type definitions | `spirv/defs.mach:52 register_defs`: 38 `isa.op_def` rows (3 core + 35 `GLSL.std.450`), 3 `isa.type_def` rows, one `TypeRefuseFn` (`:115 refuse_image`, 6 shapes) | `DefStorage` sizes `[38]`/`[3]` (`:40-41`) and the literals `38, 3` in `:111 isa.target_defs(...)` | DUP |
| word count | computed at emission `spirv.mach:348 inst_word = (n+1) << 16 \| opcode` | the `arity` column of each `OpDef` is never consulted for sizing | ADM |
| MIR → SPIR-V mapping | `spirv/emit.mach:2612 emit_instr`, a linear `if` chain (~60 arms) with the int/float pairing in `:2589 by_class` | — | ADM (not data) |
| environments | `spirv.mach:146-151 ENV_PROFILES` (4, with capability ceilings) | rebuilt as `[4]isa.Environment` at `spirv/register.mach:67-73`; SPIR-V version chosen in `EnvProfile.version` and again in `of/spv.mach:154,159` | DUP |
| register-machine residue (#2963) | `abi/spirv.mach:11 ARG_REG_COUNT = 16`, `:40 arg_regs` identity convention (argument i → nominal register i); `spirv/register.mach:25-32` two 16-wide `RegClass` rows; `spirv/emit.mach:76 rec Regs` 16-slot pre-coloured shadow file | the 16 limit is stated at 12 named sites (`types.mach:51,53`, `emit.mach:1495,1672,2534,2720,2889,3205`, `mir/abi.mach:33 MAX_GP_ARG_REGS`, `abi/spirv.mach:11`, `register.mach:27,30`) plus 10 bare `[16]` bounds in `emit.mach`; a separate arity cap of 12 at `emit.mach:2801` | DUP, NIE |
| pointer parameters (#2940) | `spirv/emit.mach:2164 spv_type_of` has no `IRT_PTR` arm and returns 0; `:1500-1503 emit_function` then refuses "unsupported parameter type" | `spirv/types.mach:451 type_fn` silently returns 0 for `n > MAX_FN_PARAMS`, a third statement of the limit | ADM |
| structured control flow | `spirv/cfg.mach:63 analyze` with 7 refusal statuses (`:26-33`), merges emitted only by `spirv/emit.mach:1691 emit_merge` | — | ok |
| effects | none: no flags, no clobbers, no latency class; `ctvalidate` refuses rather than models (`ctvalidate.mach:28`) | — | ADM (by design until N5 defines the emitter guarantee) |
| independent control | `test/lib/layers.py:44 layer_a` runs `spirv-val`; layer B uses `spirv-dis` goldens `test/golden/spirv` | — | ok |

## 6. VReg/PReg identity model

### 6.1 The types as they are

| domain | carrier | site |
|---|---|---|
| MIR virtual register | `u32` | `src/lang/be/codegen/mir.mach:754 MirOperand.vreg`, `:872 MirVReg.id` |
| MIR physical register (class-tagged regid, widened) | `u32` | `mir.mach:754 MirOperand.preg`, `:872 MirVReg.assigned` |
| MIR memory base | `vreg`/`preg` fields of the same record; base is a preg iff `vreg == MIR_VREG_NIL` | `mir.mach:1086 op_mem_preg`, `:1102 op_mem_value`, `regalloc.mach:2344` |
| MIR memory index | `index: u32` meaning a vreg or a preg according to `index_is_preg: bool` | `mir.mach:754`; 37 production sites read `index_is_preg` |
| the nil sentinel | one value, `mir.mach:748 MIR_VREG_NIL = 0xFFFFFFFF`, serves vreg, preg and index; 22 production comparisons test a preg or index against `MIR_VREG_NIL` | `regalloc.mach:2344-2350`, `mir.mach:1022-1150` constructors |
| isa register id | `i32` regid | `src/lang/target/isa.mach:71 Operand.reg/base/index`, `:59 Register.id`, `RegMachine.scratch_reg/...`, `MachineModel.frame_ptr_reg/...`, `abi.ParamSlot.reg`, `abi.ParamPiece.reg` |
| MIR class index (index into `model.reg_classes`) | `u32` | `mir.mach:752 REG_CLASS_GP = 0`, `MirVReg.class`, `regalloc.mach:142 fp_class_index` (found dynamically) |
| isa class id (the regid high byte) | `i32` | `isa.mach:56-57 REG_CLASS_ID_GP/XMM` |
| asm-parser register | `i32` regid in `asm.Operand.reg`; `(bank: u8, index: u32)` pair from `DeclRegFn` | `src/lang/target/isa/asm.mach:43`, `:177` |
| DWARF/CodeView register | `fun(i32) i32` | `src/lang/target/arch.mach:9-11` |

`def` in mach is a transparent alias ("there is no nominal distinction",
`doc/language/def.md:3-4`), so `def VRegId: u32` would enforce nothing. Nominal
identity needs single-field records, which is a thread-through of every `u32`
vreg/preg parameter in the allocator (21 `fun ...(v: u32 ...)` signatures in
`regalloc.mach` alone) and is the non-mechanical remainder of #3114 (section 9).

### 6.2 Where a raw integer stands in (NIE tally, production only)

| site class | count | examples |
|---|---|---|
| `preg::i32` / `assigned::i32` / `index::i32` / `base::i32` bare casts between the u32 and i32 carriers | 48 | `regalloc.mach` 18, `mos6502/encode.mach` 10, `x64/encode.mach` 5, `riscv/encode.mach` 5, `arm64/encode.mach` 5, `rules.mach` 2, `encode.mach` 2, `mir/abi.mach` 1 |
| `regid_make(` call sites (the only sanctioned constructor) | 33 | regalloc 9, abi packs 10, ISA files 11, `mir/abi.mach` 2, `isa.mach` 1 |
| `regid_class(`/`regid_index(` decode sites | 86 | regalloc 33, `x64/encode.mach` 16, arm64 printer+encode 13, riscv 8, rest 16 |
| `op_preg(<literal>)` / `op_vreg(<literal>)` in production | 0 | (17 hits are all in `t_*` helpers and `arm64/rules.mach:612 vec_op_select_fails`, a test-only helper without the prefix) |
| `make_reg(<literal>, ...)` in production | 0 | — |
| adjacent `u32` parameters where a vreg and a preg can be swapped silently | 1 signature, 21 vreg-typed | `regalloc.mach:1925 assign(ctx, v: u32, preg: u32, reg_class: u32)` |
| MIR preg → isa regid with no conversion function | 4 ISAs | `x64/encode.mach:4014 mir_to_operand`, `arm64/encode.mach:1051 req_gpr`, `riscv/encode.mach:671 req_gpr`, all `op.preg::i32` |
| named ISA register constants that are indices, not regids | x86_64 XMM (16), aarch64 X/V (63), riscv X/F (64) | `x64.mach:28-43`, `arm64.mach:6-76`, `riscv.mach:6-91` |
| encoder-local restatements of those constants | 12 | `arm64/encode.mach:92-95,116-117`, `riscv/encode.mach:113-114,131-136`, `x64/encode.mach:2522-2523` |

### 6.3 What is enforced today

- `regalloc.mach:1925 assign` panics if the regid class disagrees with the
  live-range class.
- `regalloc.mach:2360 verify_allocation` re-checks every assigned regid against
  its class and the reserved set.
- `regalloc.mach:2322 verify_rewritten_operands` (4.30.0, #3117 closed) refuses a
  surviving vreg, a preg outside every class (`:2287 rewritten_preg_ok`,
  `:2300 special_gp_register`), a bank mismatch against the operand's
  `required_bank` (`:2316 operand_bank_matches`, captured at selection by
  `rules.mach:80 capture_operand_banks`), and a non-GP memory base or index.
- None of these is a type-level refusal; all are runtime walks after the fact.

## 7. Target registry: ownership and borrow lifetime

### 7.1 Ownership chain

- `src/lang/session.mach:95` `Session.registry: target.TargetRegistry` **by value**;
  `src/lang/session.mach:214` `session.dnit` → `target.registry_dnit`.
- `src/lang/target.mach:90 rec TargetRegistry { isa: isa.IsaRegistry; os; abi; of;
  debug; state: u8; version: u32 }`, all sub-registries by value.
- `src/lang/target/isa.mach:778 rec IsaRegistry { entries: [8]IsaRegistryEntry;
  count; alloc; alloc_ready }` — the entries are an **inline array**.
- `src/lang/target/isa.mach:1267 register` copies the caller's descriptor into
  `entries[count]` and re-points every internal pointer at entry-owned storage:
  `:1488 entry.descriptor.model = ?entry.model`, `:1500 .machine = ?entry.machine`,
  `:1504 .emitter`, `:1508 .reloc`, `:1591 .defs`, with `own_copy` duplicates of
  `reg_classes`, reserved/reload register lists, `packed_gaps`, `envs`, op/type
  defs and their name strings (`:973 own_copy`, `:986 own_space`), all freed by
  `:991 release_entry`. So a registered `IsaVTable` is **self-referential into the
  inline array that holds it**.
- `src/lang/target.mach:211 register_all` publishes once (`REGISTRY_FRESH` →
  `BUILDING` → `READY`, `version = REGISTRY_VERSION`) and refuses a partial or
  foreign registry. Ownership of descriptor storage is therefore immutable after
  publication, which is the property #3116 wanted; it is enforced by state, not
  by type.

### 7.2 Borrowers

- `src/lang/target.mach:338 resolve` returns `resolved.Target` by value. It holds
  six borrowed pointers into the registry (`src/lang/target/resolved.mach:11`:
  `os: *OsVTable`, `isa: *IsaVTable`, `arch: *RegMachine`, `abi: *AbiVTable`,
  `of: *OfVTable`, `debug: *DebugVTable`) and one **copied** `model: MachineModel`
  (`target.mach:471`) whose `reg_classes`, `reserved_regs` and `packed_gaps`
  pointers still borrow entry storage.
- `src/lang/driver/project.mach:222 Project.target: target.Target` by value, with
  `Project.s: *session.Session`; `project.mach:206 TargetTuple.target_defs:
  *isa.TargetDefs` borrows an entry's `defs` for every union tuple.
- Every middle-end and backend context borrows `*target.Target` from the project:
  `src/lang/me/pipeline.mach:52,243`, `src/lang/me/lower/context.mach:121,133`,
  `src/lang/me/transform/licm.mach:30`, `sroa.mach:47,86`, `scalarize.mach:35,46`,
  `src/lang/be/codegen/mir/context.mach:31`, `src/lang/be/codegen.mach:89
  codegen_module(s, tgt: *target.Target, ...)`.
- `src/lang/target/resolved.mach:39 backend_target` copies the borrowed vtables'
  scalar fields into an `isa.BackendTarget` **value** with no pointers; this is
  the one place the borrow is severed, and it is what `select`/`encode`/
  `emit_module` receive.
- Editor: `src/lang/editor.mach:108 AnalysisRequest.standalone_target: *target.Target`
  is caller-owned; the synthesized host target is a stack value
  (`editor.mach:1409 var host: target.Target`, fix c86dac09) consumed inside the
  same call (`:1423-1427`), never retained.

### 7.3 Where a borrow can outlive its owner (BLU)

1. **No generation stamp.** `resolved.Target` carries nothing identifying the
   registry it was resolved against, and `backend_target` checks nothing. A
   `Target` used after `session.dnit` (or against a re-initialized registry) is
   undetectable. #3116's acceptance ("a resolved target used against a different
   registry generation fails closed in a test") is not implemented; the
   `TargetRegistry.version` field only gates `resolve` itself
   (`target.mach:342`).
2. **Session is copied by value with its inline registry.**
   `src/lang/build/engine.mach:1181 pcg_worker_init`: `w.sess = @p.s`. The copy's
   `entries[i].descriptor.model` still points into the **original** session's
   entry (set at registration), and the copy's `reg_classes`/`reserved` pointers
   alias the original's heap blocks. It is sound today only because
   `pcg_worker_teardown` (`:1187`) never calls `session.dnit` on the copy and the
   original outlives every worker; a `session.dnit(?w.sess)` would double-free
   every entry. Nothing in the types or in `registry_dnit` detects an aliased copy.
3. **Inline entries make `IsaVTable` pointers address-stable only while `Session`
   is not moved.** `session.init` returns `Session` by value
   (`src/lang/session.mach:107`) before any registration, so the move is
   harmless in practice, and every production registration goes through
   `driver.setup_registry(?s.registry)` on the settled session. Unproven, not
   broken.
4. **Test-only self-referential records.** `src/lang/me/ir/verify.mach:1242 rec
   TEnv { registry: target.TargetRegistry; ...; tgt: resolved.Target }` holds the
   owner and the borrower in one value; copying a `TEnv` dangles `tgt`. Same
   shape at the 14 `fin { registry_dnit(?reg) }` test sites in
   `src/lang/be/codegen/mir/abi.mach`, `obj.mach`, `emit.mach`, `linker.mach`,
   all of which drop the target before the registry by scope order.

Which of "immutable ownership" or "a generation stamp" closes item 1 is the
owner's call named in #2212 ("choose immutable ownership rather than a redundant
version scheme where sufficient"); section 9 records it as non-mechanical.

Landed 2026-09-12 (section 10 item 5, immutable ownership, no stamp): the
registry is `mach.lang.target.registry.TargetRegistry`, born by `registry_new`
(heap, one allocator for the block and every entry) or `registry_init` (in
place, fixtures), published once by `register_all`, released once by
`registry_dnit`, which is terminal: a released registry refuses `register_all`
and `resolve`, so re-initialization under a borrower cannot happen. `Session`
holds it by pointer and `pcg_worker_init`'s session copy borrows that pointer,
which closes item 2 by construction and makes item 3 moot. `resolved.Target`
records `registry`, `resolved.live` refuses a borrow whose registry is released,
and `isel.run`, `encode.run`, `codegen_unit` and the four link entries run it
before following a vtable (`backend_target` panics rather than follow a dead
borrow). Item 4 stands as written: the fixture form is in place because the
lane-locked `regalloc.mach`/`verify.mach` fixtures hold it, and a copy of an
in-place registry is still expressible; retiring `registry_init` for
`registry_new` at every fixture is the follow-up that removes that.

## 8. Gap tally per ISA

Counts are of distinct sites or site families named in sections 2 to 7.

| ISA | ADM | DUP | NIE | BLU |
|---|---|---|---|---|
| x86_64 | 5 (form/shape, codegen flags, memory, aliasing, clobbers) | 9 (mnemonic x3, SSE bytes, width ladder x5, cond codes x6, flag facts x3, GP names, XMM count) | 4 (`FLOAT_SCRATCH0/1`, XMM indices, `op.preg::i32`, decl_reg triple) | — |
| aarch64 | 6 (private `Inst`, mnemonics, codegen flags, memory, aliasing, clobbers) | 10 (opcode x4, shape, access scale, load/store x3, mnemonics, cond codes, branch/nullary words, NEON size, vector count) | 3 (`ZR/SP_REG/SCRATCH*`, 24 literal sites, `op.preg::i32`) | — |
| riscv64/32 | 6 (form table, load/store predicate, fence, clobbers, C claimed not emitted, xlens unenforced) | 11 (opcode x2, fields x3, mem funct3, aq/rl x3, names, numbers, FPR count, rounding, extension order) | 3 (`RZERO..SCRATCH2`, 170 literal sites, `op.preg::i32`) | — |
| SPIR-V | 4 (word count, mapping as code, pointer type, effects) | 5 (opcode literals, def counts, environments, version, the 16 limit x22) | 1 (synthetic 16-register bank) | — |
| shared | 1 (`Inst.clobbers` dead) | — | 4 (shared NIL sentinel, `index_is_preg`, `assign` signature, `def` non-nominal) | 4 (items 7.3.1-4) |

## 9. Mechanical fixes landed on `feat/2212-census`

Each is a pure restatement: the same value, named at its authoritative site
instead of restated as a literal. Codegen output is unchanged; the proof is the
layer B corpus (section 11).

| commit | gap | change |
|---|---|---|
| `94335e87` | aarch64 NIE/DUP | `arm64/encode.mach` `ZR`, `SP_REG`, `SCRATCH0`, `SCRATCH1`, `FP_SCRATCH0/1` now read `arm64.SP/IP0/IP1/V31/V30`; new `FP_REG`/`LR_REG` from `arm64.FP/LR` replace the eight `29`/`30` literals in `emit_prologue`, `emit_epilogue`, `save_callee_gp`, `restore_callee_gp` |
| `5ab1f9c0` | riscv NIE/DUP | `riscv/encode.mach` `RZERO`, `RRA`, `RSP`, `RFP`, `SCRATCH`, `SCRATCH2`, `FP_SCRATCH`, `FP_SCRATCH2` now read `riscv.ZERO/RA/SP/FP/SCRATCH_REG/SCRATCH_REG2/F31/F30` |
| `02d37b7e` | x86_64 NIE | `x64/encode.mach` `FLOAT_SCRATCH0/1 = (1 << 8) \| 14/15` became `float_scratch0()/float_scratch1()` returning `isa.regid_make(isa.REG_CLASS_ID_XMM, x64.XMM14/15)`; a function because a global initialiser must be a constant expression and the regid layout belongs to `regid_make` alone |
| `45d60099` | riscv DUP | `riscv/attributes.mach single_rank` loops to `str_len(SINGLE_ORDER)` instead of a literal `14` |
| `46b3214b` | SPIR-V DUP | `spirv/defs.mach` `OP_DEF_COUNT`/`TYPE_DEF_COUNT` size both `DefStorage` and `target_defs`; the three core opcodes cite `spirv.OP_DOT`, `OP_SAMPLED_IMAGE`, `OP_IMAGE_SAMPLE_IMPLICIT_LOD` |

Not fixed mechanically, and why:

- the 170 riscv and 24 aarch64 name-table literals (`gp_name`, `fp_name`,
  `asm_reg_index_region`, `emit_reg_id`) are the name tables themselves; the
  fix is one table per ISA that both the printer and the asm parser read, which
  is a description change, not a rename;
- `FPR_COUNT = 30` / `VECTOR_COUNT = 30` / xmm class `len 14` reserve the top
  registers by omission; stating the reservation as data (`reserved_regs`) would
  change what the allocator sees and needs the identity bar, not the corpus bar;
- the `[16]` bounds in `spirv/emit.mach` are inside the region N4 (#2963)
  replaces; naming them now only adds merge conflict to that work.

## 10. Non-mechanical remainder of N1

Ordered by what the roadmap row names. Each is a design change with a stated
owner decision or a proven bar beyond "goldens unchanged".

1. **aarch64 onto the shared instruction value.** `arm64/printer.mach:63 rec Inst`
   is the last private representation. The printer forms, the encoder's 154 base
   words and the 201 printer literals collapse into one form table keyed by base
   word only if `isa.Inst` grows a fourth operand or aarch64 keeps a form byte;
   that is #2212's open question 1 (per-ISA shape vs one generic type) and needs
   the owner's ruling before code.
2. **Per-opcode description tables where none exist** (ADM rows above):
   x86_64 form/flags/memory for the codegen path; a riscv `inst.mach` table
   (format, opcode7, funct3, funct7, xlen mask, mnemonic, load/store) that
   `rv_fields`, `classify_*`, `shape_of`, `printer.mnemonic` and `RV_MNEMONICS.code`
   all read; an aarch64 base-word table. The #2766 access rows are the template.
   Bar: byte-identical corpus plus `llvm-mc` conformance per #2118's acceptance.
3. **`isa.Inst.clobbers`.** Either delete the field or make every encoder
   populate it and add a reader; leaving a zero-filled effect field for N5 to
   discover is the fail-open shape #2212 forbids. N5 should rule.
4. **Nominal `VRegId`/`PRegId`.** `def` cannot express it; the carriers become
   single-field records and thread through `MirOperand`, `MirVReg.assigned`,
   the 21 `v: u32` allocator signatures, every ISA's `req_gpr`/`req_vec`/
   `mir_to_operand`, and `abi.ParamSlot.reg`. Same change adds a `MIR_PREG_NIL`
   and replaces `index`+`index_is_preg` with a tagged operand. Bar: identity
   (byte-identical) on every target, plus a negative test that assigning a vreg
   to a preg field is a type error (#3114 acceptance).
5. **Registry ownership.** Section 7.3: either stamp a generation into
   `resolved.Target` and check it in `backend_target`, or make the registry a
   heap-owned immutable object that `Session` and `PcgWorker` hold by pointer so
   the by-value copy at `engine.mach:1181` becomes impossible. #2212 prefers the
   second where sufficient; it is sufficient here. Bar: the fail-at-N allocator
   leak check and a fail-closed test for a target used after `registry_dnit`.
   Landed 2026-09-12 as the second form; section 7.3 records the shape.
6. **riscv admission on the codegen path.** `inst.admits` guards only inline
   asm; the MIR path relies on width arithmetic. One admission point, and the
   `has_compressed` claim (`EF_RISCV_RVC`, `c2p0`) removed until an emitter
   exists. This is the seam N3 (#3127) builds on.
7. **SPIR-V opcode table.** An `OpDef`-shaped row (opcode, arity, result-type)
   for the ~60 `emit_instr` arms, so word counts and the int/float pairing are
   data. Sequenced after N4 so the value ABI is not rebuilt twice.

## 11. Frozen interfaces for N3 to N6

These are the exact types and functions the later increments consume. A change
to any of them is a roadmap revision, not a local refactor. Everything not
listed here (encoder internals, printer tables, the per-ISA constants renamed
in section 9) may move under the byte-identity bar without notice.

### N3 (#3120 vector capability catalog, #3127 RISC-V selection)

Amended 2026-09-11 to the shapes N3 landed (PRs #3257, #3258 and the phase 2
PR). Two deviations from the list as frozen were accepted by the owner: the
`PackedGap` record became `PackedForm`, because its meaning flipped from absent
to present, and the RISC-V attribute hook gained the selected extension bits plus
a validate slot, because the selection cannot reach the emitted string or the
link admission any other way without a second capability channel.

- `isa.MachineModel` (`src/lang/target/isa.mach:172`) and its accessors
  `packed_width`, `packed_lane_cap`, `moves_unaligned_gp`, `moves_vector_memory`,
  `moves_cross_bank`, `fits_vector_register`, plus `rec PackedForm` (`:136`, the
  positive packed row: operation, lane kind, lane width), `rec ScalarForm` (`:145`,
  the declared scalar expansion, same key), `VectorForm` with `FORM_UNDECLARED`,
  `FORM_SCALAR`, `FORM_PACKED` (`:151`) and the `VEC_OP_*`/`VEC_MEM_*`/`XBANK_*`
  vocabularies: the declared-capability record, now positive and complete. An ISA
  with a vector unit declares every retained cell in exactly one of
  `packed_forms` and `scalar_forms`; `vector_form` (`:367`) is the one decision
  accessor and `vector_domain_complete` (`:337`) is what registration enforces, so
  a new domain member is a registration refusal for every ISA that has not decided
  it. A machine with `has_v128 == false` declares every cell scalar by that fact
  and may hold no rows. Field meanings may not change; rows may be added.
- `vecform.decide` (`src/lang/me/vecform.mach:95`) and `realizes_packed` (`:103`):
  how an IR operator reads the catalog. `scalarize.detect_undeclared`
  (`src/lang/me/pass/scalarize.mach:137`) and the pipeline's `declared_check`
  (`src/lang/me/pipeline.mach:219`) refuse an undeclared shape in every `simd`
  mode; `simd = "require"` then refuses a declared scalar row on top.
- `target.mach:341 resolve` and `resolved.Target.model` (the per-target copy,
  narrowed for a RISC-V selection at `target.mach:406` through
  `riscv/register.mach:95 select_features`): the one place a capability reaches
  the backend; N3 added no second capability channel. `MachineModel.riscv_extensions`
  is the selected extension bits on that copy and the fingerprint covers it. It
  stays ISA-named on the shared record on purpose: the parse, the narrowing and
  the field are one seam, and generalizing the field alone would leave the
  `resolve` fork in place. When a second ISA gains a selection vocabulary the three
  move together into vtable hooks (parse, narrow) over an ISA-interpreted
  `features` field, and that is a roadmap revision.
- `rules.GuardFn: fun(*isa.BackendTarget, *mir.MirFunction, *mir.MirInstr) bool`,
  `rules.Rule`, `rules.RulePack`, `RuleGate` (`src/lang/be/codegen/rules.mach:18-63`):
  guards see the machine through `BackendTarget.model` only.
- `mir.MirOpDescriptor.operand_banks` and `mir.operand_bank` (`mir.mach:635`),
  `mir.selection_reachable_float`, `mir.selection_lane`: the catalog columns that
  decide packed vs scalar vs refused.
- `riscv/features.mach`: `parse` / `parse_at` (`:116`, `:133`; the span names the
  refused token), `rec Selection` (`:75`), `rec Span` (`:111`), the `I..ZIFENCEI`
  bits, `DEFAULT32`/`DEFAULT64`, and `EXTS` (`:48`, the admission and emission
  table). `riscv/inst.mach:210 required_extensions`, `:183 xlens`, `:205 admits`:
  the extension-admission seam, total over the opcode catalog.
- `riscv/attributes.mach:553 riscv64_build_attributes(alloc, xlen_bits,
  extension_bits, float_arg_bits, has_compressed, out_len)`, `:591
  validate_selected(bytes, len, xlen_bits, selected, flags)`, `:641
  riscv64_merge_attributes`, `:165 parse_arch`, `rec Ext/Arch/Attrs`, and the
  `of.BuildAttributesFn` (`of.mach:1237`, carrying the selected bits) /
  `MergeAttributesFn` / `ValidateAttributesFn` (`:1240`) / `MachineFlagsFn` slots
  installed together by `isa.with_attributes(s, build, merge, validate)`
  (`isa.mach:830`), `with_elf_attributes`, `with_machine_flags`: the emitted
  extension string, its merge rule and the admission of a foreign object against
  the selection. The three hooks are declared together or not at all.
- `isa.lookup`, `isa.arch_id_for`, `isa.float_absence_note` (`isa.mach:994`, the
  front end's way to name the F extension a refused float needs without naming an
  ISA), `target.mach:700 tuple_capability` and `TupleCapabilitySpec`: where an
  `isa = "rv32imc"` spelling is refused or resolved, and every refusal names the
  token and the selection string.

### N4 (#2963 module-emitter value ABI, #2940 pointer-to-record)

- `isa.ModuleEmitter`, `isa.EmitModuleFn: fun(*A.Allocator, *isa.BackendTarget,
  *unit_input.Unit, **u8, *u32)`, `isa.module_emitter`, `isa.emitter_isa`
  (`isa.mach:390,352,738,729`): the whole-module family contract.
- `isa.BackendTarget` (`isa.mach:342`) and `resolved.backend_target`
  (`resolved.mach:39`): the pointer-free view an emitter receives; N4 may add
  fields, not pointers.
- `isa.emits_whole_module`, `isa.backend_family`, `isa.is_whole_module`
  (`isa.mach:1621-1662`) and the fork sites `codegen.mach:144`,
  `ctvalidate.mach:28`: the family test every caller uses.
- `abi.ParamClass` (`CLASS_AGG = 8`), `abi.ParamSlot`, `abi.ParamPiece`,
  `abi.AggLayout`, `abi.ClassifyFn`/`ArgPassingFn`/`RetPassingFn`
  (`src/lang/target/abi.mach:93-140`) and `abi/spirv.mach` `classify_arg`/
  `classify_return`/`arg_regs`: the contract N4 replaces with a value-oriented
  one. Register machines keep the `reg: i32` carrier unchanged.
- `src/lang/be/codegen/mir/abi.mach:45 abi_gp_arg_reg` and `MAX_GP_ARG_REGS`:
  the shared consumer that must stop reading a bank for emitters.
- `isa.TargetDefs`/`OpDef`/`TypeDef`/`TypeRefuseFn`, `isa.with_defs`,
  `isa.Environment`, `isa.with_environments`, `environment_lookup`,
  `environment_profile` (`isa.mach:396-542`): the published op/type/env surface
  L6 will read; names and tags are stable.
- `mir.SEL_CLASS_SPIRV_ONLY`, `MIR_VEC_BUILD`, `MIR_AGG_LOAD`, `MIR_AGG_STORE`:
  the emitter-only opcodes and their catalog rows.

### N5 (#3126 secrecy against final machine effects)

- `mir.MirOpDescriptor` columns `flags`, `ct_class`, `operand_banks` and the
  readers `mir.has`, `mir.ct_class`, `mir.ct_op`, `mir.operand_bank`: the only
  per-opcode effect description; N5 extends rows, it does not add a parallel
  table.
- `ct.CtOp`, `ct.CtCap`, `ct.AsmClass` and its constructors
  (`src/lang/ct.mach:5-157`), `ct.target_provides`: the effect vocabulary shared
  by the MIR validator and the asm scan.
- `isa.AsmCtScanFn`, `isa.AsmClobbersFn`, `isa.AsmReturnsFn`
  (`isa.mach:283-287`), `asm.Grammar.ct_class: CtClassFn`, `asm.Mnemonic`
  (`writes`, `implicit`, `implicit_mem`), and each ISA's `asm_ct_class(code,
  flags)`: the closed per-instruction effect description inline asm uses.
- `isa.Inst`, `isa.Operand`, `isa.inst_blank`, `regid_make/class/index`: the
  physical-register stream after allocation; the `clobbers` field is frozen as
  **unpopulated** until item 10.3 rules. Amended 2026-09-12 (#3126, accepted as
  additive like N3's): `isa.Inst.src3`, blank by `inst_blank`, because aarch64
  `madd`/`msub` read three registers (Rn, Rm, Ra) and a notification that drops
  the fourth register cannot be walked soundly; no existing reader changes,
  x86_64 and riscv64 never set it.
- `mir.MirInstr.writes_secret`, `memory_flags`, `mir.MirVReg.secret`,
  `mir.MirOperand.required_bank`, `regalloc.verify_rewritten_operands`
  (`regalloc.mach:2322`): the post-rewrite facts N5 validates against.
- `encode.sink_claim`/`sink_claims`/`sink_unaccounted`/`note_inst_count`/
  `AsmNote`, `encode.encode_driver`/`EncodeHooks` (`src/lang/be/codegen/encode.mach`),
  and the three per-ISA `check_accounted`: the invariant that ties every emitted
  byte to one instruction notification, which is how N5 reaches late-introduced
  instructions.
- `ctvalidate.run(tgt, m)` (`ctvalidate.mach:23`) and its position in
  `codegen.mach:131 codegen_scratch`: the early checker N5 keeps as a diagnostic.
- `MachineModel.ct_trust_mul`, `ct_trust_var_shift`: the target trust
  declarations.

### N6 (#3110 cross-module inlining with correct effects)

- `mir.MirAsm`, `mir.MirAsmBind`, `mir.asm_bind`, `mir.MirInstr.asm_block`
  (`mir.mach:768-792`) and `ir.clone_asm_payload` (used at
  `src/lang/me/pass/inline.mach:621,744`): the asm payload that survives a clone.
- `asm.Grammar.no_fall: NoFallThroughFn`, `writes_sp: WritesSpFn`,
  `asm.returns(g, body)`, `asm.clobbers(g, body, gp, fp)`
  (`src/lang/target/isa/asm.mach:169-171,925,949`) and `isa.AssemblyCapabilities`
  / `RegMachine.has_assembly`: the block-level facts that decide whether an asm
  body may be inlined and what it clobbers.
- `ct.AsmClass` via `asm_ct_class` (as in N5): the ordering/latency facts of the
  eight atomic wrappers are read from here, not inferred from body length.
- `mir.MIRF_MEMORY`, `mir.MirInstr.memory_flags` (`MEMORY_VOLATILE`),
  `mir.MirFunction.naked`/`oblivious`, `mir.emits_instr`: the effect bits an
  inlined body must keep.
- `rules.emit`, `rules.ExpansionBuilder`, `rules.ExpandFn`
  (`rules.mach:32-42,144`): the only sanctioned way to introduce instructions
  after selection.
