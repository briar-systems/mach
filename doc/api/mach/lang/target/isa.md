# mach.lang.target.isa

## def Endian

```mach
pub def Endian: u8
```

## val ENDIAN_LITTLE

```mach
pub val ENDIAN_LITTLE: Endian = 0
```

## val ENDIAN_BIG

```mach
pub val ENDIAN_BIG:    Endian = 1
```

## val ARCH_UNKNOWN

```mach
pub val ARCH_UNKNOWN: u32 = arch.UNKNOWN
```

## val ARCH_X86_64

```mach
pub val ARCH_X86_64:  u32 = arch.X86_64
```

## val ARCH_AARCH64

```mach
pub val ARCH_AARCH64: u32 = arch.AARCH64
```

## val ARCH_RISCV64

```mach
pub val ARCH_RISCV64: u32 = arch.RISCV64
```

## val ARCH_SPIRV

```mach
pub val ARCH_SPIRV:   u32 = arch.SPIRV
```

## val ARCH_RISCV32

```mach
pub val ARCH_RISCV32: u32 = arch.RISCV32
```

## val ISA_LABEL

```mach
pub val ISA_LABEL:       u16 = 0xFFFF
```

## val ISA_ASM_MARKER

```mach
pub val ISA_ASM_MARKER:  u16 = 0xFFFE
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

## val ISA_FLAG_DST_READ

```mach
pub val ISA_FLAG_DST_READ: u16 = 0x8000
```

## val ISA_FLAG_FP_COPY

```mach
pub val ISA_FLAG_FP_COPY:  u16 = 0x4000
```

## def OperandKind

```mach
pub def OperandKind: u8
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

## rec Register

```mach
pub rec Register;
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

## rec Operand

```mach
pub rec Operand;
```

## rec Inst

```mach
pub rec Inst;
```

## rec InstBuf

```mach
pub rec InstBuf;
```

## def RegClassKind

```mach
pub def RegClassKind: u8
```

## val RC_GENERAL

```mach
pub val RC_GENERAL: RegClassKind = 0
```

## val RC_FLOAT

```mach
pub val RC_FLOAT:   RegClassKind = 1
```

## val RC_VECTOR

```mach
pub val RC_VECTOR:  RegClassKind = 2
```

## val RC_FLAGS

```mach
pub val RC_FLAGS:   RegClassKind = 3
```

## rec RegClass

```mach
pub rec RegClass;
```

## def VecOp

```mach
pub def VecOp: u32
```

## val VEC_OP_NONE

```mach
pub val VEC_OP_NONE: VecOp = 0
```

## val VEC_OP_ADD

```mach
pub val VEC_OP_ADD:  VecOp = 1
```

## val VEC_OP_SUB

```mach
pub val VEC_OP_SUB:  VecOp = 2
```

## val VEC_OP_MUL

```mach
pub val VEC_OP_MUL:  VecOp = 3
```

## val VEC_OP_DIV

```mach
pub val VEC_OP_DIV:  VecOp = 4
```

## val VEC_OP_AND

```mach
pub val VEC_OP_AND:  VecOp = 5
```

## val VEC_OP_OR

```mach
pub val VEC_OP_OR:   VecOp = 6
```

## val VEC_OP_XOR

```mach
pub val VEC_OP_XOR:  VecOp = 7
```

## val VEC_OP_NOT

```mach
pub val VEC_OP_NOT:  VecOp = 8
```

## val VEC_OP_NEG

```mach
pub val VEC_OP_NEG:  VecOp = 9
```

## val VEC_OP_SHL

```mach
pub val VEC_OP_SHL:  VecOp = 10
```

## val VEC_OP_SHR

```mach
pub val VEC_OP_SHR:  VecOp = 11
```

## val VEC_OP_CMP

```mach
pub val VEC_OP_CMP:  VecOp = 12
```

## rec PackedForm

```mach
pub rec PackedForm;
```

## rec ScalarForm

```mach
pub rec ScalarForm;
```

a row an ISA with a vector unit declares as the documented scalar expansion:
the same key as a packed row, and the two tables together must cover the
retained domain exactly once, so a cell in neither is a missing decision

## def VectorForm

```mach
pub def VectorForm: u8
```

## val FORM_UNDECLARED

```mach
pub val FORM_UNDECLARED: VectorForm = 0
```

## val FORM_SCALAR

```mach
pub val FORM_SCALAR:     VectorForm = 1
```

## val FORM_PACKED

```mach
pub val FORM_PACKED:     VectorForm = 2
```

## val VECTOR_BITS_128

```mach
pub val VECTOR_BITS_128: u32 = 128
```

## val VECTOR_LANES_UNBOUNDED

```mach
pub val VECTOR_LANES_UNBOUNDED: u32 = 0
```

## val VEC_MEM_NONE

```mach
pub val VEC_MEM_NONE: u32 = 0
```

## val VEC_MEM_4_8_16

```mach
pub val VEC_MEM_4_8_16: u32 = 4 + 8 + 16
```

## val VEC_MEM_ALL_128

```mach
pub val VEC_MEM_ALL_128: u32 = 1 + 2 + 4 + 8 + 16
```

## val XBANK_NONE

```mach
pub val XBANK_NONE: u32 = 0
```

## val XBANK_4

```mach
pub val XBANK_4: u32 = 4
```

## val XBANK_4_8

```mach
pub val XBANK_4_8: u32 = 4 + 8
```

## rec MachineModel

```mach
pub rec MachineModel;
```

## fun vector_domain_len

```mach
pub fun vector_domain_len() u32;
```

the retained (operation, lane kind, lane width) domain: every cell the
vocabulary admits, in one fixed order, so a table can be checked against it

## fun vector_domain_cell

```mach
pub fun vector_domain_cell(index: u32, op: *VecOp, is_float: *bool, lane_bits: *u32) bool;
```

## fun vector_domain_complete

```mach
pub fun vector_domain_complete(m: *MachineModel) bool;
```

a machine with a vector unit decides every retained cell; one without decides
them all as the scalar expansion by declaring no unit

## fun packed_width

```mach
pub fun packed_width(m: *MachineModel, op: VecOp, is_float: bool, lane_bits: u32) u32;
```

## fun vector_form

```mach
pub fun vector_form(m: *MachineModel, op: VecOp, is_float: bool, lane_bits: u32) VectorForm;
```

the one declared outcome for a cell: packed, the scalar expansion, or nothing

## fun packed_lane_cap

```mach
pub fun packed_lane_cap(m: *MachineModel, lane_bits: u32) u32;
```

## fun moves_unaligned_gp

```mach
pub fun moves_unaligned_gp(m: *MachineModel, bytes: u32) bool;
```

## fun moves_vector_memory

```mach
pub fun moves_vector_memory(m: *MachineModel, bytes: u32) bool;
```

## fun moves_cross_bank

```mach
pub fun moves_cross_bank(m: *MachineModel, bytes: u32) bool;
```

## fun fits_vector_register

```mach
pub fun fits_vector_register(m: *MachineModel, lane_bits: u32, lanes: u32) bool;
```

## def AsmClobbersFn

```mach
pub def AsmClobbersFn: fun(str, *u32, *u32)
```

## def AsmReturnsFn

```mach
pub def AsmReturnsFn: fun(str) bool
```

## def AsmCtScanFn

```mach
pub def AsmCtScanFn: fun(str, *str, u32, bool, bool, *A.Allocator) err[fail.Fail]
```

## def IsRegMoveFn

```mach
pub def IsRegMoveFn: fun(u32) bool
```

## def IsTrapTerminatorFn

```mach
pub def IsTrapTerminatorFn: fun(u32) bool
```

## def DwarfRegFn

```mach
pub def DwarfRegFn: arch.DwarfRegFn
```

## def CvRegFn

```mach
pub def CvRegFn: arch.CvRegFn
```

## def RegFileFn

```mach
pub def RegFileFn: fun(*Register) i32
```

## def FrameDistFn

```mach
pub def FrameDistFn: fun(*MachineModel, *mir.MirFunction, *u32, *u32)
```

## rec BackendIdentity

```mach
pub rec BackendIdentity;
```

## rec BackendAbi

```mach
pub rec BackendAbi;
```

## rec BackendOs

```mach
pub rec BackendOs;
```

## rec BackendTarget

```mach
pub rec BackendTarget;
```

## def SelectFn

```mach
pub def SelectFn: fun(*A.Allocator, *BackendTarget, *mir.MirFunction) err[fail.Fail]
```

## def EncodeFn

```mach
pub def EncodeFn: fun(*A.Allocator, *BackendTarget, *mir.MirModule) res[encoding.EncoderOutput, fail.Fail]
```

## def EmitAsmFn

```mach
pub def EmitAsmFn: fun(*A.Allocator, *BackendTarget, *mir.MirModule,
*writer.Writer) res[encoding.EncoderOutput, fail.Fail]
```

## def EmitModuleFn

```mach
pub def EmitModuleFn: fun(*A.Allocator, *BackendTarget, *unit_input.Unit, **u8, *u32) err[fail.Fail]
```

## rec AssemblyCapabilities

```mach
pub rec AssemblyCapabilities;
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
pub def RelocSeam: of.RelocationCapabilities
```

## rec OpDef

```mach
pub rec OpDef;
```

## val TYPE_OPERAND_WORD

```mach
pub val TYPE_OPERAND_WORD: u32 = 0xFFFFFFFF
```

## val NO_TYPE_CTOR

```mach
pub val NO_TYPE_CTOR: u32 = 0xFFFFFFFF
```

## def TypeRefuseFn

```mach
pub def TypeRefuseFn: fun(*u32, u32) str
```

## rec TypeDef

```mach
pub rec TypeDef;
```

## rec TargetDefs

```mach
pub rec TargetDefs;
```

## val NO_OP_SET

```mach
pub val NO_OP_SET: u32 = definition.NO_OP_SET
```

## val NO_OPCODE

```mach
pub val NO_OPCODE: u32 = definition.NO_OPCODE
```

## fun op_def

```mach
pub fun op_def(set: str, name: str, set_tag: u32, opcode: u32, arity: u32) OpDef;
```

## fun type_def

```mach
pub fun type_def(name: str, tag: u32, operands: *u32, arity: u32, refuse: TypeRefuseFn) TypeDef;
```

## fun target_defs

```mach
pub fun target_defs(ops: *OpDef, op_count: u32, types: *TypeDef, type_count: u32) TargetDefs;
```

## fun type_def_of

```mach
pub fun type_def_of(defs: *TargetDefs, name: str) *TypeDef;
```

## fun type_def_of_region

```mach
pub fun type_def_of_region(defs: *TargetDefs, src: str, off: usize, len: usize) *TypeDef;
```

## fun type_def_by_tag

```mach
pub fun type_def_by_tag(defs: *TargetDefs, tag: u32) *TypeDef;
```

## fun op_def_of

```mach
pub fun op_def_of(defs: *TargetDefs, set_name: str, op_name: str) *OpDef;
```

## fun op_def_of_region

```mach
pub fun op_def_of_region(defs: *TargetDefs, set_src: str, set_off: usize, set_len: usize,
nam_src: str, nam_off: usize, nam_len: usize) *OpDef;
```

## fun op_set_exists_region

```mach
pub fun op_set_exists_region(defs: *TargetDefs, src: str, off: usize, len: usize) bool;
```

## fun op_set_exists

```mach
pub fun op_set_exists(defs: *TargetDefs, name: str) bool;
```

## fun with_defs

```mach
pub fun with_defs(vt: *IsaVTable, d: *TargetDefs);
```

## fun with_environments

```mach
pub fun with_environments(vt: *IsaVTable, envs: *Environment, count: u32);
```

## fun environment_lookup

```mach
pub fun environment_lookup(vt: *IsaVTable, name: str) u32;
```

## fun environment_profile

```mach
pub fun environment_profile(vt: *IsaVTable, env_id: u32) u32;
```

## rec Environment

```mach
pub rec Environment;
```

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
reserved_regs: *Register, reserved_reg_count: i32,
reload_scratch_regs: *Register, reload_scratch_count: i32,
scratch_reg: i32, scratch_reg2: i32,
frame_ptr_reg: i32, stack_ptr_reg: i32, incoming_arg_base: i64,
div_reg: i32, div_hi_reg: i32, shift_count_reg: i32) RegMachine;
```

## fun with_assembly

```mach
pub fun with_assembly(m: *RegMachine, emit: EmitAsmFn, clobbers: AsmClobbersFn,
returns: AsmReturnsFn, ct_scan: AsmCtScanFn);
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

## fun reloc_seam

```mach
pub fun reloc_seam(apply_reloc: of.ApplyRelocFn, reloc_traits: of.RelocTraitsFn,
elf_reloc_type: of.ElfRelocTypeFn) RelocSeam;
```

## fun with_elf_attributes

```mach
pub fun with_elf_attributes(s: *RelocSeam, attributes: *of.ElfAttributes);
```

## fun with_local_got_kinds

```mach
pub fun with_local_got_kinds(s: *RelocSeam, f: of.LocalGotKindFn);
```

## fun declares_local_got

```mach
pub fun declares_local_got(tgt_isa: *IsaVTable) bool;
```

## fun local_got_kind

```mach
pub fun local_got_kind(tgt_isa: *IsaVTable, kind: of.RelocKind) bool;
```

## fun with_machine_flags

```mach
pub fun with_machine_flags(s: *RelocSeam, f: of.MachineFlagsFn);
```

## def MachineFlagsFn

```mach
pub def MachineFlagsFn: of.MachineFlagsFn
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
pub fun with_normalize_image(s: *RelocSeam, f: of.NormalizeImageFn);
```

## fun with_resolve_reloc_operand

```mach
pub fun with_resolve_reloc_operand(s: *RelocSeam, f: of.ResolveRelocOperandFn);
```

## fun with_attributes

```mach
pub fun with_attributes(s: *RelocSeam, build: of.BuildAttributesFn, merge: of.MergeAttributesFn, validate: of.ValidateAttributesFn);
```

## def BuildAttributesFn

```mach
pub def BuildAttributesFn: of.BuildAttributesFn
```

## def MergeAttributesFn

```mach
pub def MergeAttributesFn: of.MergeAttributesFn
```

## fun declares_attributes

```mach
pub fun declares_attributes(tgt_isa: *IsaVTable) bool;
```

## fun build_attributes

```mach
pub fun build_attributes(tgt_isa: *IsaVTable, model: *MachineModel, alloc: *A.Allocator, float_arg_bits: u32,
has_compressed: bool, out_len: *u32) res[*u8, fail.Fail];
```

## fun validate_attributes

```mach
pub fun validate_attributes(tgt_isa: *IsaVTable, model: *MachineModel,
bytes: *u8, len: u32, flags: u32) err[fail.Fail];
```

## fun merge_attributes

```mach
pub fun merge_attributes(tgt_isa: *IsaVTable, alloc: *A.Allocator, acc: *u8, acc_len: u32,
add: *u8, add_len: u32, out_len: *u32) res[*u8, fail.Fail];
```

## fun object_target

```mach
pub fun object_target(vt: *IsaVTable, out: *of.ObjectTarget);
```

## fun machine_isa

```mach
pub fun machine_isa(id: u32, name: str, elf_machine: u32, pointer_width: u32,
model: *MachineModel, machine: *RegMachine,
reloc: *RelocSeam) IsaVTable;
```

## fun emitter_isa

```mach
pub fun emitter_isa(id: u32, name: str, pointer_width: u32,
model: *MachineModel, emitter: *ModuleEmitter) IsaVTable;
```

## fun module_emitter

```mach
pub fun module_emitter(emit_module: EmitModuleFn) ModuleEmitter;
```

## rec IsaRegistryEntry

```mach
pub rec IsaRegistryEntry;
```

## rec IsaRegistry

```mach
pub rec IsaRegistry;
```

## val ARCH_CATALOG_VERSION

```mach
pub val ARCH_CATALOG_VERSION: u8 = 2
```

version 2: the withdrawn MOS 6502 row left the catalog and the tags after it moved up (#3226)

## fun arch_id_for

```mach
pub fun arch_id_for(name: str) u32;
```

## fun float_absence_note

```mach
pub fun float_absence_note(name: str) str;
```

why a selection has no floating-point unit, for a diagnostic that already names
the selection: the letters a RISC-V string would need, or empty when the name
carries no extension vocabulary the front end could ask the user to add

## fun arch_name_for

```mach
pub fun arch_name_for(id: u32) str;
```

## fun arch_catalog_len

```mach
pub fun arch_catalog_len() usize;
```

## fun arch_catalog_name

```mach
pub fun arch_catalog_name(index: usize) str;
```

## fun arch_fingerprint_tag

```mach
pub fun arch_fingerprint_tag(id: u32) u8;
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

## fun make_label

```mach
pub fun make_label(name: str) Operand;
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

## val LOCAL_LABEL_FWD

```mach
pub val LOCAL_LABEL_FWD:  i64 = -1
```

## val LOCAL_LABEL_BACK

```mach
pub val LOCAL_LABEL_BACK: i64 = -2
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

## fun buf_init

```mach
pub fun buf_init(alloc: *A.Allocator) InstBuf;
```

## fun buf_append

```mach
pub fun buf_append(buf: *InstBuf, inst: Inst);
```

## fun buf_dnit

```mach
pub fun buf_dnit(buf: *InstBuf);
```

## fun registry_init_with_allocator

```mach
pub fun registry_init_with_allocator(alloc: *A.Allocator) IsaRegistry;
```

## fun registry_init

```mach
pub fun registry_init() IsaRegistry;
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

## fun emits_relocations

```mach
pub fun emits_relocations(vt: *IsaVTable) bool;
```

## def BackendKind

```mach
pub def BackendKind: u8
```

## val BACKEND_NATIVE

```mach
pub val BACKEND_NATIVE:       BackendKind = 0
```

## val BACKEND_WHOLE_MODULE

```mach
pub val BACKEND_WHOLE_MODULE: BackendKind = 1
```

## rec BackendFamily

```mach
pub rec BackendFamily;
```

## fun backend_family

```mach
pub fun backend_family(vt: *IsaVTable) BackendFamily;
```

## fun is_native

```mach
pub fun is_native(f: *BackendFamily) bool;
```

## fun is_whole_module

```mach
pub fun is_whole_module(f: *BackendFamily) bool;
```

## fun as_native

```mach
pub fun as_native(f: *BackendFamily) *RegMachine;
```

## fun make_sym_mod

```mach
pub fun make_sym_mod(sym_id: u32, size: u8, mod: SymModifier) Operand;
```

## fun make_sym_addend

```mach
pub fun make_sym_addend(sym_id: u32, size: u8, mod: SymModifier, addend: i32) Operand;
```

## fun inst_blank

```mach
pub fun inst_blank() Inst;
```

