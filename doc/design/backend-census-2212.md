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
