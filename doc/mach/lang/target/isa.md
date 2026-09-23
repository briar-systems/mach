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

## val VEC_OP_CMP

```mach
pub val VEC_OP_CMP:  VecOp = 12
```

## val VEC_OP_TRUNC

```mach
pub val VEC_OP_TRUNC:    VecOp = 13
```

lane-wise conversions: the operation fixes each side's lane kind and
signedness, and a cell also names the source lane width

## val VEC_OP_SEXT

```mach
pub val VEC_OP_SEXT:     VecOp = 14
```

## val VEC_OP_ZEXT

```mach
pub val VEC_OP_ZEXT:     VecOp = 15
```

## val VEC_OP_FP_TRUNC

```mach
pub val VEC_OP_FP_TRUNC: VecOp = 16
```

## val VEC_OP_FP_EXT

```mach
pub val VEC_OP_FP_EXT:   VecOp = 17
```

## val VEC_OP_FP_TO_SI

```mach
pub val VEC_OP_FP_TO_SI: VecOp = 18
```

## val VEC_OP_FP_TO_UI

```mach
pub val VEC_OP_FP_TO_UI: VecOp = 19
```

## val VEC_OP_SI_TO_FP

```mach
pub val VEC_OP_SI_TO_FP: VecOp = 20
```

## val VEC_OP_UI_TO_FP

```mach
pub val VEC_OP_UI_TO_FP: VecOp = 21
```

## val VEC_OP_MUL_WIDE_S

```mach
pub val VEC_OP_MUL_WIDE_S: VecOp = 22
```

a widening multiply: both operands share a lane type, and the result lanes are
twice as wide and hold the full product under the operation's signedness. a
cell names the result lane width and the operand lane width; its scalar row is
the unfused path, where each operand is extended and then multiplied

## val VEC_OP_MUL_WIDE_U

```mach
pub val VEC_OP_MUL_WIDE_U: VecOp = 23
```

## val VEC_OP_WIDEN_S

```mach
pub val VEC_OP_WIDEN_S: VecOp = 24
```

a lane-halving extension: half the operand's integer lanes (its low or high
half), each extended to twice the width under the operation's signedness. a
cell names the result lane width and the operand lane width; its scalar row
is the unfused path, one extension per lane into a rebuilt vector (#3738)

## val VEC_OP_WIDEN_U

```mach
pub val VEC_OP_WIDEN_U: VecOp = 25
```

## val VEC_OP_SHL_U

```mach
pub val VEC_OP_SHL_U:   VecOp = 26
```

the lane-wise shifts of integer lanes, keyed by direction, by signedness for
a right shift, and by the count's form, since the packed answer differs along
each (#3740): a uniform count (`_U`) is one scalar applied to every lane, the
form every baseline set has an instruction for, and a per-lane count (`_V`)
is a vector of counts, one per lane. a count at or above the lane width
saturates in every form, as the scalar shift does (#3756)

## val VEC_OP_SHR_U_U

```mach
pub val VEC_OP_SHR_U_U: VecOp = 27
```

## val VEC_OP_SHR_S_U

```mach
pub val VEC_OP_SHR_S_U: VecOp = 28
```

## val VEC_OP_SHL_V

```mach
pub val VEC_OP_SHL_V:   VecOp = 29
```

## val VEC_OP_SHR_U_V

```mach
pub val VEC_OP_SHR_U_V: VecOp = 30
```

## val VEC_OP_SHR_S_V

```mach
pub val VEC_OP_SHR_S_V: VecOp = 31
```

## val VEC_OP_LAST

```mach
pub val VEC_OP_LAST:    VecOp = VEC_OP_SHR_S_V
```

## rec PackedForm

```mach
pub rec PackedForm;
```

a cell is the operation, the result lane kind and width, and the operand
lane width; an operation that keeps its lanes repeats lane_bits there. ext
is the extension bits the packed instruction needs, none for the baseline:
a gated row takes part only when the selected target holds every bit, and
the same cell may then also carry a baseline scalar row for when it does not

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

## val SLOT_READ_NONE

```mach
pub val SLOT_READ_NONE: u32 = 0
```

## val SLOT_READ_1_2_4_8

```mach
pub val SLOT_READ_1_2_4_8: u32 = 1 + 2 + 4 + 8
```

## val INT_WIDTHS_1_2_4_8

```mach
pub val INT_WIDTHS_1_2_4_8: u32 = 1 + 2 + 4 + 8
```

the scalar integer widths a target realizes, encoded like vec_mem_widths:
the bit for a width is that width; every ISA declares the 64-bit set today
and none the 128-bit one (#3511)

## val INT_WIDTHS_1_2_4_8_16

```mach
pub val INT_WIDTHS_1_2_4_8_16: u32 = 1 + 2 + 4 + 8 + 16
```

## def MulWideForm

```mach
pub def MulWideForm: u8
```

how a target realizes the full product of two lane-width integers: no
instruction (the schoolbook over extended operands), a three-address high
multiply beside the low one, or a fixed register pair (#3511, Q11); a
declaration, never derived from mul_hi_width

## val MULW_NONE

```mach
pub val MULW_NONE:          MulWideForm = 0
```

## val MULW_THREE_ADDRESS

```mach
pub val MULW_THREE_ADDRESS: MulWideForm = 1
```

## val MULW_FIXED_PAIR

```mach
pub val MULW_FIXED_PAIR:    MulWideForm = 2
```

## rec MachineModel

```mach
pub rec MachineModel;
```

## fun is_convert_op

```mach
pub fun is_convert_op(op: VecOp) bool;
```

## fun is_widen_op

```mach
pub fun is_widen_op(op: VecOp) bool;
```

## fun is_widen_half_op

```mach
pub fun is_widen_half_op(op: VecOp) bool;
```

## fun is_shift_op

```mach
pub fun is_shift_op(op: VecOp) bool;
```

## fun vector_domain_len

```mach
pub fun vector_domain_len() u32;
```

the retained (operation, lane kind, lane width, operand lane width) domain:
every cell the vocabulary admits, in one fixed order, so a table can be
checked against it

## fun vector_domain_cell

```mach
pub fun vector_domain_cell(index: u32, cell: *ScalarForm) bool;
```

## val CONVERT_CELL_COUNT

```mach
pub val CONVERT_CELL_COUNT: u32 = 52
```

a machine with a vector unit decides every retained cell; one without decides
them all as the scalar expansion by declaring no unit

## val WIDEN_CELL_COUNT

```mach
pub val WIDEN_CELL_COUNT: u32 = 12
```

## val SHIFT_CELL_COUNT

```mach
pub val SHIFT_CELL_COUNT: u32 = 24
```

## fun scalar_conversion_rows

```mach
pub fun scalar_conversion_rows(m: *MachineModel, rows: *ScalarForm, at: u32) u32;
```

the scalar rows for every conversion cell the model's packed table does not
claim, written from `at`; an ISA registers its packed conversions first and
declares the rest through this, so each conversion cell is decided once

## fun scalar_widening_rows

```mach
pub fun scalar_widening_rows(m: *MachineModel, rows: *ScalarForm, at: u32) u32;
```

the same for the widening multiplies and the lane-halving extensions: a
cell the packed table leaves keeps the per-lane path

## fun scalar_shift_rows

```mach
pub fun scalar_shift_rows(m: *MachineModel, rows: *ScalarForm, at: u32) u32;
```

the same for the shifts: a cell the packed table leaves is shifted lane by
lane through the scalar shift, which saturates the same way

## fun ct_mul_rows_admit

```mach
pub fun ct_mul_rows_admit(m: *MachineModel, dit_mode: bool) ct.CtMulMask;
```

the cells a model's rows admit once each condition is decided: ALWAYS
holds, EXTENSION holds when the model selects every named extension, and
DIT_MODE holds when the caller says the mode is guaranteed on

## fun ct_mul_rows_under

```mach
pub fun ct_mul_rows_under(m: *MachineModel, cond: ct.CtMulCond) ct.CtMulMask;
```

the cells whose rows carry one condition, decided or not: what a DIT_MODE
multiply would need before it is admitted. a realized cell (#3511) needs the
condition when some cell of its realization does, so the set is the rows of
the condition plus every realized cell admitted with all rows held that is
not admitted with the condition's rows withheld

## rec ExtensionView

```mach
pub rec ExtensionView;
```

what a selected target says about extensions: the bits it selects and the
vocabulary that names them

## fun extension_view

```mach
pub fun extension_view(m: *MachineModel) ExtensionView;
```

## fun extension_domain

```mach
pub fun extension_domain(m: *MachineModel) u64;
```

every extension bit the model's vocabulary names

## fun extension_bit

```mach
pub fun extension_bit(m: *MachineModel, name: str, len: usize) u64;
```

the bit the model's vocabulary gives `name[0..len]`, 0 when it names no such extension

## fun extension_bundle

```mach
pub fun extension_bundle(m: *MachineModel, name: str, len: usize) u64;
```

what a manifest's `extensions` entry `name[0..len]` selects: a level's
members when the model publishes one by that spelling, else the row's bit,
0 when the vocabulary has neither

## fun spell_levels

```mach
pub fun spell_levels(m: *MachineModel, buf: *u8, cap: usize);
```

the model's levels, `, `-separated in buf, empty when it publishes none

## fun extension_close

```mach
pub fun extension_close(m: *MachineModel, bits: u64) u64;
```

`bits` closed over the model's vocabulary: every extension a selected one implies

## rec DecoratorExtension

```mach
pub rec DecoratorExtension;
```

what `#[extensions(name)]` may mean: the selected isa's row when it has one,
else any registered row of that name, which admits nothing here. a row only
a target selects is never a decorator's, and `target_only` says why

## fun decorator_extension

```mach
pub fun decorator_extension(reg: *IsaRegistry, selected: ExtensionView, name: str, len: usize) DecoratorExtension;
```

## fun spell_extensions

```mach
pub fun spell_extensions(m: *MachineModel, buf: *u8, cap: usize);
```

the model's vocabulary, `, `-separated, or `none`, NUL-terminated in buf

## fun extension_known

```mach
pub fun extension_known(reg: *IsaRegistry, name: str, len: usize) bool;
```

what a registry's vocabularies say of one name: whether any registered isa
names it, and the names every vocabulary holds, deduplicated, for a refusal

## fun spell_all_extensions

```mach
pub fun spell_all_extensions(reg: *IsaRegistry, buf: *u8, cap: usize);
```

every registered vocabulary's names, each table once, `, `-separated in buf

## fun ct_mul_forms_refusal

```mach
pub fun ct_mul_forms_refusal(m: *MachineModel) opt[str];
```

why a model's constant-time multiply rows are malformed; the extension
domain is every extension bit the target model knows

## fun vector_domain_complete

```mach
pub fun vector_domain_complete(m: *MachineModel) bool;
```

## fun packed_width

```mach
pub fun packed_width(m: *MachineModel, op: VecOp, is_float: bool, lane_bits: u32, from_bits: u32) u32;
```

a conversion's register holds its wider side, so that side sets the extent

## fun vector_form

```mach
pub fun vector_form(m: *MachineModel, op: VecOp, is_float: bool, lane_bits: u32, from_bits: u32) VectorForm;
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

## fun reads_slot_operand

```mach
pub fun reads_slot_operand(m: *MachineModel, bytes: u32) bool;
```

## fun models_lifetime_holes

```mach
pub fun models_lifetime_holes(m: *MachineModel) bool;
```

## fun moves_cross_bank

```mach
pub fun moves_cross_bank(m: *MachineModel, bytes: u32) bool;
```

## fun realizes_int_width

```mach
pub fun realizes_int_width(m: *MachineModel, bytes: u32) bool;
```

the only reader of int_widths: whether the target realizes a scalar integer
of this many bytes

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

## def AsmWritesSpFn

```mach
pub def AsmWritesSpFn: fun(str) bool
```

whether an asm body may write the stack pointer; true for a body it cannot parse

## def AsmCtScanFn

```mach
pub def AsmCtScanFn: fun(str, *ct.AsmSecret, u32, ct.CtMulMask, bool, *A.Allocator) err[fail.Fail]
```

the constant-time scan of an asm body: the tracked bindings (`ct.AsmSecret`, by the name
their `{name}` operand spells), the target's multiply admission and shift trust

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
pub def EmitModuleFn: fun(*A.Allocator, *BackendTarget, *unit_input.Unit, *debug_input.ModuleDebug, **u8, *u32) err[fail.Fail]
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

## fun with_page_size

```mach
pub fun with_page_size(vt: *IsaVTable, page_size: u64);
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
returns: AsmReturnsFn, ct_scan: AsmCtScanFn, writes_sp: AsmWritesSpFn);
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

version 2: catalog id 4 is reserved with no row and the tags after it moved up (#3226)

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

