# mach.lang.target.isa

## val ARCH_UNKNOWN

```mach
pub val ARCH_UNKNOWN: u32 = catalog_arch.UNKNOWN
```

## val ARCH_X86_64

```mach
pub val ARCH_X86_64:  u32 = catalog_arch.X86_64
```

## val ARCH_AARCH64

```mach
pub val ARCH_AARCH64: u32 = catalog_arch.AARCH64
```

## val ARCH_RISCV64

```mach
pub val ARCH_RISCV64: u32 = catalog_arch.RISCV64
```

## val ARCH_SPIRV

```mach
pub val ARCH_SPIRV:   u32 = catalog_arch.SPIRV
```

## val ARCH_RISCV32

```mach
pub val ARCH_RISCV32: u32 = catalog_arch.RISCV32
```

## val ISA_LABEL

```mach
pub val ISA_LABEL:       u16 = 0xFFFF
```

## val ISA_PCOPY

```mach
pub val ISA_PCOPY:       u16 = 0xFFFD
```

## val ISA_PCOPY_FENCE

```mach
pub val ISA_PCOPY_FENCE: u16 = 0xFFFC
```

## val ISA_USE

```mach
pub val ISA_USE:         u16 = 0xFFFB
```

## val OPK_NONE

```mach
pub val OPK_NONE:  OperandKind = 0
```

## val OPK_REG

```mach
pub val OPK_REG:   OperandKind = 1
```

## val OPK_IMM

```mach
pub val OPK_IMM:   OperandKind = 2
```

## val OPK_MEM

```mach
pub val OPK_MEM:   OperandKind = 3
```

## val OPK_LABEL

```mach
pub val OPK_LABEL: OperandKind = 4
```

## val OPK_SYM

```mach
pub val OPK_SYM:   OperandKind = 5
```

## val REG_CLASS_ID_GP

```mach
pub val REG_CLASS_ID_GP:  i32 = 0
```

## val REG_CLASS_ID_XMM

```mach
pub val REG_CLASS_ID_XMM: i32 = 1
```

## def SymModifier

```mach
pub def SymModifier: u8
```

## val SYM_MOD_NONE

```mach
pub val SYM_MOD_NONE:     SymModifier = 0
```

## val SYM_MOD_PCREL_HI

```mach
pub val SYM_MOD_PCREL_HI: SymModifier = 1
```

## val SYM_MOD_PCREL_LO

```mach
pub val SYM_MOD_PCREL_LO: SymModifier = 2
```

## val SYM_MOD_GOT_HI

```mach
pub val SYM_MOD_GOT_HI:   SymModifier = 3
```

## val SYM_MOD_GOT_LO

```mach
pub val SYM_MOD_GOT_LO:   SymModifier = 4
```

## val SYM_MOD_GOT

```mach
pub val SYM_MOD_GOT: SymModifier = 5
```

a whole pc-relative reference to the symbol's GOT slot (x86-64 GOTPCREL)

## rec Operand

```mach
pub rec Operand;
```

## rec Inst

```mach
pub rec Inst;
```

## fun declare_scalar_rows

```mach
pub fun declare_scalar_rows(reg: *IsaRegistry, m: *target_model.Machine, explicit: *target_model.ScalarForm, explicit_len: u32, families: target_model.ScalarFamily) err[fail.Fail];
```

declare the model's scalar table: `explicit`, then every cell of `families`
its packed table leaves, family by family, in storage from the registry's
allocator sized to exactly those rows. the caller releases it with
release_scalar_rows once the model is registered, which copies it

reg: the registry whose allocator backs the rows
m: the model; its packed table is final, its scalar table is set
explicit: the rows the model names itself
explicit_len: how many
families: the families whose unclaimed cells are scalar rows

## fun release_scalar_rows

```mach
pub fun release_scalar_rows(reg: *IsaRegistry, m: *target_model.Machine);
```

free the table declare_scalar_rows gave the model

## fun vocabulary

```mach
pub fun vocabulary(reg: *IsaRegistry) res[extension.Vocabulary, fail.Fail];
```

every extension vocabulary the registry's instruction sets declare, each table once

## def AsmCtScanFn

```mach
pub def AsmCtScanFn: fun(str, *ct.AsmSecret, u32, ct.CtMulMask, bool, *A.Allocator) err[ct.AsmRefusal]
```

the constant-time scan of an asm body: the tracked bindings (`ct.AsmSecret`, by the name
their `{name}` operand spells), the target's multiply admission and shift trust

## def DwarfRegFn

```mach
pub def DwarfRegFn: catalog_arch.DwarfRegFn
```

## def CvRegFn

```mach
pub def CvRegFn: catalog_arch.CvRegFn
```

## def RegFileFn

```mach
pub def RegFileFn: fun(*target_model.Register) i32
```

## rec BackendTarget

```mach
pub rec BackendTarget;
```

## rec RegMachine

```mach
pub rec RegMachine;
```

## rec ModuleEmitter

```mach
pub rec ModuleEmitter;
```

## def RelocSeam

```mach
pub def RelocSeam: target_of.RelocationCapabilities
```

## fun with_defs

```mach
pub fun with_defs(vt: *IsaVTable, d: *target_definition.Table);
```

## fun with_page_size

```mach
pub fun with_page_size(vt: *IsaVTable, page_size: u64);
```

## fun with_environments

```mach
pub fun with_environments(vt: *IsaVTable, envs: *Environment, count: u32, open: u64);
```

`open` is what a target naming no environment is granted: with no ceiling
over it, every extension an environment could guarantee

## fun environment_lookup

```mach
pub fun environment_lookup(vt: *IsaVTable, name: str) u32;
```

## fun environment_profile

```mach
pub fun environment_profile(vt: *IsaVTable, env_id: u32) u32;
```

## fun environment_extensions

```mach
pub fun environment_extensions(vt: *IsaVTable, env_id: u32) u64;
```

the extensions the target's environment guarantees, which its selection holds
beside the ones it names

## rec Environment

```mach
pub rec Environment;
```

an execution environment an isa defines: its name, its profile, and the
extensions of the isa's vocabulary it guarantees (spirv's `zero_init_workgroup`
from vulkan1.3)

## val ENV_NONE

```mach
pub val ENV_NONE: u32 = 0xFFFFFFFF
```

## rec IsaVTable

```mach
pub rec IsaVTable;
```

## fun reg_machine

```mach
pub fun reg_machine(select: SelectFn, encode: EncodeFn,
is_reg_move: IsRegMoveFn, is_trap_terminator: IsTrapTerminatorFn,
reserved_regs: *target_model.Register, reserved_reg_count: i32,
reload_scratch_regs: *target_model.Register, reload_scratch_count: i32,
scratch_reg: i32, scratch_reg2: i32,
frame_ptr_reg: i32, stack_ptr_reg: i32, incoming_arg_base: i64,
div_reg: i32, div_hi_reg: i32, shift_count_reg: i32) RegMachine;
```

## fun with_assembly

```mach
pub fun with_assembly(m: *RegMachine, emit: EmitAsmFn, clobbers: AsmClobbersFn,
returns: resolved.AsmReturnsFn, ct_scan: AsmCtScanFn, writes_sp: AsmWritesSpFn);
```

## fun with_dwarf_regs

```mach
pub fun with_dwarf_regs(m: *RegMachine, f: DwarfRegFn);
```

## fun with_codeview_regs

```mach
pub fun with_codeview_regs(m: *RegMachine, f: CvRegFn);
```

## fun with_frame_dist

```mach
pub fun with_frame_dist(m: *RegMachine, f: FrameDistFn);
```

## fun with_int_imm_rule

```mach
pub fun with_int_imm_rule(m: *RegMachine, f: resolved.IntImmFitsFn);
```

## fun with_const_operand_rule

```mach
pub fun with_const_operand_rule(m: *RegMachine, f: ReadsConstFn);
```

## fun reloc_seam

```mach
pub fun reloc_seam(apply_reloc: target_of.ApplyRelocFn, reloc_traits: target_of.RelocTraitsFn,
elf_reloc_type: target_of.ElfRelocTypeFn) RelocSeam;
```

## fun with_elf_attributes

```mach
pub fun with_elf_attributes(s: *RelocSeam, attributes: *target_of.ElfAttributes);
```

## fun with_local_got_kinds

```mach
pub fun with_local_got_kinds(s: *RelocSeam, f: target_of.LocalGotKindFn);
```

## fun declares_local_got

```mach
pub fun declares_local_got(tgt_isa: *IsaVTable) bool;
```

## fun local_got_kind

```mach
pub fun local_got_kind(tgt_isa: *IsaVTable, kind: target_of.RelocKind) bool;
```

## fun with_branch_thunks

```mach
pub fun with_branch_thunks(s: *RelocSeam, reach: target_of.BranchReachFn, thunk: target_of.BranchThunkFn);
```

## fun declares_branch_thunks

```mach
pub fun declares_branch_thunks(tgt_isa: *IsaVTable) bool;
```

## fun branch_reach

```mach
pub fun branch_reach(tgt_isa: *IsaVTable, kind: target_of.RelocKind) opt[target_of.BranchReach];
```

the reach of a direct branch a thunk can extend, none for any other kind or
an instruction set that places no thunks

## fun with_machine_flags

```mach
pub fun with_machine_flags(s: *RelocSeam, f: target_of.MachineFlagsFn);
```

## fun declares_machine_flags

```mach
pub fun declares_machine_flags(tgt_isa: *IsaVTable) bool;
```

## fun machine_flags

```mach
pub fun machine_flags(tgt_isa: *IsaVTable, float_arg_bits: u32,
has_compressed: bool) u32;
```

## fun with_normalize_image

```mach
pub fun with_normalize_image(s: *RelocSeam, f: target_of.NormalizeImageFn);
```

## fun with_resolve_reloc_operand

```mach
pub fun with_resolve_reloc_operand(s: *RelocSeam, f: target_of.ResolveRelocOperandFn);
```

## fun with_attributes

```mach
pub fun with_attributes(s: *RelocSeam, build: target_of.BuildAttributesFn, merge: target_of.MergeAttributesFn, validate: target_of.ValidateAttributesFn);
```

## fun declares_attributes

```mach
pub fun declares_attributes(tgt_isa: *IsaVTable) bool;
```

## fun build_attributes

```mach
pub fun build_attributes(tgt_isa: *IsaVTable, model: *target_model.Machine, alloc: *A.Allocator, float_arg_bits: u32,
has_compressed: bool, out_len: *u32) res[*u8, fail.Fail];
```

## fun validate_attributes

```mach
pub fun validate_attributes(tgt_isa: *IsaVTable, model: *target_model.Machine,
bytes: *u8, len: u32, flags: u32) err[fail.Fail];
```

## fun merge_attributes

```mach
pub fun merge_attributes(tgt_isa: *IsaVTable, alloc: *A.Allocator, acc: *u8, acc_len: u32,
add: *u8, add_len: u32, out_len: *u32) res[*u8, fail.Fail];
```

## fun object_target

```mach
pub fun object_target(vt: *IsaVTable, out: *target_of.ObjectTarget);
```

## fun machine_isa

```mach
pub fun machine_isa(id: u32, name: str, elf_machine: u32, pointer_width: u32,
model: *target_model.Machine, machine: *RegMachine,
reloc: *RelocSeam) IsaVTable;
```

## fun emitter_isa

```mach
pub fun emitter_isa(id: u32, name: str, pointer_width: u32,
model: *target_model.Machine, emitter: *ModuleEmitter) IsaVTable;
```

## fun module_emitter

```mach
pub fun module_emitter(emit_module: EmitModuleFn, has_assembly: bool) ModuleEmitter;
```

## rec IsaRegistry

```mach
pub rec IsaRegistry;
```

## fun regid_make

```mach
pub fun regid_make(class_id: i32, index: i32) i32;
```

## fun regid_class

```mach
pub fun regid_class(regid: i32) i32;
```

## fun regid_index

```mach
pub fun regid_index(regid: i32) i32;
```

## fun make_none

```mach
pub fun make_none() Operand;
```

## fun make_reg

```mach
pub fun make_reg(id: i32, size: u8) Operand;
```

## fun make_imm

```mach
pub fun make_imm(value: i64, size: u8) Operand;
```

## fun make_mem

```mach
pub fun make_mem(base: i32, disp: i32, index: i32, scale: u8, size: u8) Operand;
```

## fun make_block_label

```mach
pub fun make_block_label(id: u32) Operand;
```

## fun make_local_label

```mach
pub fun make_local_label(forward: bool) Operand;
```

a target inside the same expansion, resolved by the encoder that emitted
it: not a block of the function and not a symbol. the direction is what
a walk needs: a forward skip only joins ahead, a backward one is a loop

## fun make_numbered_local_label

```mach
pub fun make_numbered_local_label(forward: bool, number: u32) Operand;
```

a local label with a number, so a listing can spell the jump as `1f` or
`1b` and its definition as `1:`; the encoder still patches the displacement

## fun local_label_number

```mach
pub fun local_label_number(op: *Operand) u32;
```

## fun label_is_block

```mach
pub fun label_is_block(op: *Operand) bool;
```

## fun label_is_forward_local

```mach
pub fun label_is_forward_local(op: *Operand) bool;
```

## fun make_sym

```mach
pub fun make_sym(sym_id: u32, size: u8) Operand;
```

## fun registry_init_with_allocator

```mach
pub fun registry_init_with_allocator(alloc: *A.Allocator) IsaRegistry;
```

## fun registry_dnit

```mach
pub fun registry_dnit(reg: *IsaRegistry);
```

## fun registry_validate

```mach
pub fun registry_validate(reg: *IsaRegistry) err[fail.Fail];
```

## fun register

```mach
pub fun register(reg: *IsaRegistry, vt: *IsaVTable) err[fail.Fail];
```

## fun lookup

```mach
pub fun lookup(reg: *IsaRegistry, name: str) opt[*IsaVTable];
```

## fun registered_count

```mach
pub fun registered_count(reg: *IsaRegistry) u32;
```

## fun registered

```mach
pub fun registered(reg: *IsaRegistry, idx: u32) opt[*IsaVTable];
```

## fun has_codegen

```mach
pub fun has_codegen(vt: *IsaVTable) bool;
```

## fun emits_whole_module

```mach
pub fun emits_whole_module(vt: *IsaVTable) bool;
```

## fun has_assembly

```mach
pub fun has_assembly(vt: *IsaVTable) bool;
```

whether source may carry `asm` blocks for this instruction set, whichever backend family it has

## fun emits_relocations

```mach
pub fun emits_relocations(vt: *IsaVTable) bool;
```

## fun make_sym_mod

```mach
pub fun make_sym_mod(sym_id: u32, size: u8, mod: SymModifier) Operand;
```

## fun make_sym_addend

```mach
pub fun make_sym_addend(sym_id: u32, size: u8, mod: SymModifier, addend: i64) Operand;
```

## fun inst_blank

```mach
pub fun inst_blank() Inst;
```

