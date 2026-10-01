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

## val VEC_OP_RANGE

```mach
pub val VEC_OP_RANGE: VecOp = 32
```

a lane range: the lanes of the operand from a constant first lane on, of the
same lane type, in a vector of at most as many lanes. a cell names the lane
kind and width, which the operation keeps; its scalar row is the lane path,
each lane extracted and the result built or written through memory (#3864)

## val VEC_OP_CONCAT

```mach
pub val VEC_OP_CONCAT: VecOp = 33
```

a lane join: the lanes of each operand in turn, all of one lane type, in one
vector of their summed count. it is how a vector wider than the register is
put back together from its register-width pieces, and how a narrowing
conversion over those pieces gathers its halves (#3589). a cell names the
lane kind and width, which the operation keeps; its scalar row is the lane
path through memory

## val VEC_OP_MUL_HIGH_S

```mach
pub val VEC_OP_MUL_HIGH_S: VecOp = 34
```

the high half of the full product of two integer lanes, in lanes of their
own width, under the operation's signedness: the upper word of the widening
multiply, whose lower word is the plain multiply. a cell names the lane width
twice; its scalar row is each lane's own high multiply (#4119)

## val VEC_OP_MUL_HIGH_U

```mach
pub val VEC_OP_MUL_HIGH_U: VecOp = 35
```

## val VEC_OP_INTERLEAVE

```mach
pub val VEC_OP_INTERLEAVE: VecOp = 36
```

a lane interleave: lane i of the low operand and lane i of the high operand
joined into one lane twice as wide, the low operand's lane in its low half.
it is how a target whose widening multiply is the low and the high multiply
pairs the two into full products. a cell names the result lane width and the
operand lane width; its scalar row is each lane joined on its own (#4119)

## val VEC_OP_WIDEN_SUM_U

```mach
pub val VEC_OP_WIDEN_SUM_U: VecOp = 37
```

a widening group sum: each result lane the zero-extended sum of the operand
integer lanes it covers, so the result holds the operand's bits in fewer,
wider lanes. a cell names the result lane width and the operand lane width;
its scalar row is the per-lane sum, and only a packed cell is ever formed,
as the fold of a count accumulated in narrow lanes (#4161)

## val VEC_OP_SIGN_MASK

```mach
pub val VEC_OP_SIGN_MASK: VecOp = 38
```

a lane sign mask: each integer lane all ones when it is negative, else zero,
in lanes of its own width. it is the high half a signed lane-halving
extension interleaves its lanes with, so both halves of one extension share
it; its scalar row is each lane shifted right by its width less one, and
only a packed cell is ever formed (#4198)

## val VEC_OP_LAST

```mach
pub val VEC_OP_LAST:      VecOp = VEC_OP_SIGN_MASK
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

## rec VectorRegisterRow

```mach
pub rec VectorRegisterRow;
```

a width the vector register file takes under an extension: with `ext`
selected a register carries `bytes`, more than the compute width
`vector_bits` the packed rows run at. a convention that places a vector by
the register it fits reads the widest selected row (#3751). `upper_clear`
says the narrower code the target emits pays for a register's upper bytes
while they hold a value, so the upper state is cleared once a value of this
width has been moved out of its register (x86-64's vzeroupper after a ymm
value, which the legacy-encoded 128-bit instructions otherwise stall on)

## rec VectorWidthRow

```mach
pub rec VectorWidthRow;
```

a compute width an extension gives the packed rows over one kind of lane:
with `ext` selected, the rows over float lanes (`is_float`) or integer ones
of the byte widths `lanes` names (a sum of 1, 2, 4 and 8, as vec_mem_widths)
run at `bits` rather than the baseline `vector_bits`. a vector of those
lanes is then realized whole in a register that wide, and type legalization
splits it only past it (#4128). the kinds widen apart on a target whose
extensions do: x86-64's avx computes binary32 and binary64 at 256 bits and
leaves integers at 128 until avx2

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

## val XBANK_2_4_8

```mach
pub val XBANK_2_4_8: u32 = 2 + 4 + 8
```

## val SLOT_READ_NONE

```mach
pub val SLOT_READ_NONE: u32 = 0
```

## val SLOT_READ_1_2_4_8

```mach
pub val SLOT_READ_1_2_4_8: u32 = 1 + 2 + 4 + 8
```

## val SHIFT_MOD_NONE

```mach
pub val SHIFT_MOD_NONE: u32 = 0
```

## val SHIFT_MOD_4

```mach
pub val SHIFT_MOD_4: u32 = 4
```

## val SHIFT_MOD_4_8

```mach
pub val SHIFT_MOD_4_8: u32 = 4 + 8
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

## val ALU_WIDTHS_4

```mach
pub val ALU_WIDTHS_4:       u32 = 4
```

alu width sets for declare_alu_widths, encoded like int_widths

## val ALU_WIDTHS_4_8

```mach
pub val ALU_WIDTHS_4_8:     u32 = 4 + 8
```

## val ALU_WIDTHS_1_2_4_8

```mach
pub val ALU_WIDTHS_1_2_4_8: u32 = 1 + 2 + 4 + 8
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

## rec AluWidth

```mach
pub rec AluWidth;
```

one width the alu computes an integer at, in bytes, legal while the model
holds every extension in `ext` (0 when it always is)

## val ALU_WIDTH_CAP

```mach
pub val ALU_WIDTH_CAP: u32 = 5
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

## fun is_mul_high_op

```mach
pub fun is_mul_high_op(op: VecOp) bool;
```

## fun is_interleave_op

```mach
pub fun is_interleave_op(op: VecOp) bool;
```

## fun is_widen_half_op

```mach
pub fun is_widen_half_op(op: VecOp) bool;
```

## fun is_widen_sum_op

```mach
pub fun is_widen_sum_op(op: VecOp) bool;
```

## fun is_sign_mask_op

```mach
pub fun is_sign_mask_op(op: VecOp) bool;
```

## fun is_shift_op

```mach
pub fun is_shift_op(op: VecOp) bool;
```

## fun is_range_op

```mach
pub fun is_range_op(op: VecOp) bool;
```

## fun is_permute_op

```mach
pub fun is_permute_op(op: VecOp) bool;
```

the lane rearrangements: a range of one vector's lanes and a join of several

## fun half_native

```mach
pub fun half_native(m: *MachineModel, op: VecOp, is_float: bool, lane_bits: u32, from_bits: u32) bool;
```

the target realizes this scalar f16 cell with its own instruction under the
selected extensions; false is the inlined expansion

## fun vector_register_bytes

```mach
pub fun vector_register_bytes(m: *MachineModel) u32;
```

the widest vector one register carries under the selected extensions: the
compute width, or a selected row's wider register

## fun vector_register_bits

```mach
pub fun vector_register_bits(m: *MachineModel) u32;
```

the widest vector register the selected extensions give, in bits: the
layout aligns a vector as wide as one to it (#4128)

## fun vector_upper_clear

```mach
pub fun vector_upper_clear(m: *MachineModel, bytes: u32) bool;
```

a value `bytes` wide moved out of its register leaves upper state the
target clears afterwards

## fun vector_width

```mach
pub fun vector_width(m: *MachineModel, is_float: bool, lane_bits: u32) u32;
```

the width in bits the packed rows over lanes of this kind and width run at
under the selected extensions: the widest selected row's, or `vector_bits`

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

## def ScalarFamily

```mach
pub def ScalarFamily: u32
```

the families of retained cells a model leaves to the scalar path wherever
its packed table does not claim them: the conversions; the widening
multiplies with their high halves, the lane interleave, the lane-halving
extensions with their sign masks and the widening group sums; the shifts, lane by lane through the
scalar shift, which saturates the same way; the lane ranges and joins; and
the f16 lane arithmetic and comparisons, each lane's scalar f16 operation (#3802)

## val SCALAR_CONVERSIONS

```mach
pub val SCALAR_CONVERSIONS: ScalarFamily = 1
```

## val SCALAR_WIDENING

```mach
pub val SCALAR_WIDENING:    ScalarFamily = 2
```

## val SCALAR_SHIFTS

```mach
pub val SCALAR_SHIFTS:      ScalarFamily = 4
```

## val SCALAR_PERMUTES

```mach
pub val SCALAR_PERMUTES:    ScalarFamily = 8
```

## val SCALAR_HALF_LANES

```mach
pub val SCALAR_HALF_LANES:  ScalarFamily = 16
```

## fun declare_scalar_rows

```mach
pub fun declare_scalar_rows(reg: *IsaRegistry, m: *MachineModel, explicit: *ScalarForm, explicit_len: u32, families: ScalarFamily) err[fail.Fail];
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
pub fun release_scalar_rows(reg: *IsaRegistry, m: *MachineModel);
```

free the table declare_scalar_rows gave the model

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

## fun from_is_float

```mach
pub fun from_is_float(op: VecOp, is_float: bool) bool;
```

the lane kind of a cell's operand: a conversion between the kinds reads
the other kind, and every other cell its result's

## fun packed_width

```mach
pub fun packed_width(m: *MachineModel, op: VecOp, is_float: bool, lane_bits: u32, from_bits: u32) u32;
```

a conversion's register holds its wider side, so that side sets the extent,
and each side's lanes stay within its own kind's width

## fun packing_extension

```mach
pub fun packing_extension(m: *MachineModel, op: VecOp, is_float: bool, lane_bits: u32, from_bits: u32) u64;
```

the extensions a target would add to pack this cell: what the first declared
row of the cell needs beyond the model's selection. 0 when a selected row
packs it already or no row declares it at any extension

## fun packed_row_selected_by

```mach
pub fun packed_row_selected_by(m: *MachineModel, op: VecOp, is_float: bool, lane_bits: u32, from_bits: u32, ext: u64) bool;
```

whether the model selects the row of this cell gated on exactly `ext`: the
lowering's choice between the baseline form and an extension's instruction

## fun extends_directly

```mach
pub fun extends_directly(m: *MachineModel) bool;
```

a lane-wise integer extension of any ratio is one instruction under the
selected extensions, rather than a chain of doublings

## fun vector_form

```mach
pub fun vector_form(m: *MachineModel, op: VecOp, is_float: bool, lane_bits: u32, from_bits: u32) VectorForm;
```

the one declared outcome for a cell: packed, the scalar expansion, or nothing

## fun packed_lane_cap

```mach
pub fun packed_lane_cap(m: *MachineModel, is_float: bool, lane_bits: u32) u32;
```

## fun vector_op_bytes

```mach
pub fun vector_op_bytes(m: *MachineModel, is_float: bool, lane_bits: u32, bytes: u32) u32;
```

the bytes of the register that realizes a vector of `bytes` over lanes of
this kind whole: the narrowest register width the kind computes at that
holds it, `vector_op_bytes` at the least. a vector wider than every such
register is not realized in one and keeps the narrowest (#4128)

## fun moves_unaligned_gp

```mach
pub fun moves_unaligned_gp(m: *MachineModel, bytes: u32) bool;
```

## fun offset_fits

```mach
pub fun offset_fits(bits: u32, off: i64) bool;
```

a constant offset fits a signed field of `bits` bits; 0 bits is no bound

## fun sym_offset_folds

```mach
pub fun sym_offset_folds(m: *MachineModel, off: i64) bool;
```

## fun mem_disp_folds

```mach
pub fun mem_disp_folds(m: *MachineModel, off: i64) bool;
```

## fun moves_vector_memory

```mach
pub fun moves_vector_memory(m: *MachineModel, bytes: u32) bool;
```

a vector of `bytes` moves between memory and its register in one access:
a width the target declares, or a register width its extensions give past
the compute width (#4128)

## fun reads_slot_operand

```mach
pub fun reads_slot_operand(m: *MachineModel, bytes: u32) bool;
```

## fun models_lifetime_holes

```mach
pub fun models_lifetime_holes(m: *MachineModel) bool;
```

## fun shift_count_mod_width

```mach
pub fun shift_count_mod_width(m: *MachineModel, bytes: u32) bool;
```

## fun moves_cross_bank

```mach
pub fun moves_cross_bank(m: *MachineModel, bytes: u32) bool;
```

## fun declare_alu_widths

```mach
pub fun declare_alu_widths(m: *MachineModel, widths: u32);
```

declares the alu widths `widths` (encoded like int_widths), each one legal
under every extension set, in place of any declared before

## fun gate_alu_width

```mach
pub fun gate_alu_width(m: *MachineModel, bytes: u32, ext: u64);
```

declares the alu width `bytes`, legal only while the model holds `ext`

## fun alu_width_set

```mach
pub fun alu_width_set(m: *MachineModel) u32;
```

the alu widths legal under the model's extensions, encoded like int_widths

## fun alu_min_width

```mach
pub fun alu_min_width(m: *MachineModel) u32;
```

the narrowest legal alu width, 0 for a model that declares none

## fun max_alu_width

```mach
pub fun max_alu_width(m: *MachineModel) u32;
```

the widest legal alu width, above which legalize splits an operation into
lanes; 0 for a model that declares none

## fun alu_width_floor_set

```mach
pub fun alu_width_floor_set(m: *MachineModel) u32;
```

the alu widths legal under every extension set, encoded like int_widths

## fun clamp_alu_width

```mach
pub fun clamp_alu_width(m: *MachineModel, w: u8) u8;
```

the width an integer operation of `w` bytes computes at: w itself when it is
legal, otherwise the narrowest width above it that is legal whatever the
extensions, so a lift never needs a capability the program did not spell; w
again when none is (legalize splits it)

## fun realizes_int_width

```mach
pub fun realizes_int_width(m: *MachineModel, bytes: u32) bool;
```

the only reader of int_widths: whether the target realizes a scalar integer
of this many bytes

## fun fits_vector_register

```mach
pub fun fits_vector_register(m: *MachineModel, is_float: bool, lane_bits: u32, lanes: u32) bool;
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
pub def AsmCtScanFn: fun(str, *ct.AsmSecret, u32, ct.CtMulMask, bool, *A.Allocator) err[ct.AsmRefusal]
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

## def IntImmFitsFn

```mach
pub def IntImmFitsFn: fun(u64, u32) bool
```

whether one instruction materializes the integer `value`, read at `bits`
(at most 64): the rule the middle end hoists a loop's constants by (#3807)

## def ReadsConstFn

```mach
pub def ReadsConstFn: fun(*mir.MirInstr, u32) bool
```

whether the selected instruction reads operand `index` as a constant in
place, as a constant-pool memory operand, at no instruction of its own: the
rule the allocator folds a rebuilt constant into its reader by (#4184)

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
pub def EmitModuleFn: fun(*A.Allocator, *BackendTarget, *unit_input.Unit, *debug_input.ModuleDebug, *of.ObjectImage) err[fail.Fail]
```

a whole-module emitter fills the object image codegen initialized: its sections,
symbols and relocations, in the same neutral kinds a native image carries, with
section bytes allocated from the image's allocator. names are the emitter's own

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

## def OpOperandKind

```mach
pub def OpOperandKind: u8
```

how one operand of an `#[op]` instruction is written: an ordinary value id, an
id that must be an integer constant by emission, a literal word inline, a
pointer whose storage class the call site's access chain decides, which the
instruction only reads through, only stores through, or reads and writes, a
handle value whose memory the instruction reads or writes (a handle passed as
an ordinary value names its descriptor and touches none of its memory), or a
truth value the target writes as its boolean type, from an integer argument
whose nonzero is true. a memory scope or a memory semantics operand is a
constant id too, one the target reads by value and holds to what that value
needs under its memory model

## val OP_OPERAND_VALUE

```mach
pub val OP_OPERAND_VALUE:          OpOperandKind = 0
```

## val OP_OPERAND_CONSTANT

```mach
pub val OP_OPERAND_CONSTANT:       OpOperandKind = 1
```

## val OP_OPERAND_LITERAL

```mach
pub val OP_OPERAND_LITERAL:        OpOperandKind = 2
```

## val OP_OPERAND_POINTER_READ

```mach
pub val OP_OPERAND_POINTER_READ:   OpOperandKind = 3
```

## val OP_OPERAND_POINTER_WRITE

```mach
pub val OP_OPERAND_POINTER_WRITE:  OpOperandKind = 4
```

## val OP_OPERAND_POINTER_UPDATE

```mach
pub val OP_OPERAND_POINTER_UPDATE: OpOperandKind = 5
```

## val OP_OPERAND_HANDLE_READ

```mach
pub val OP_OPERAND_HANDLE_READ:    OpOperandKind = 6
```

## val OP_OPERAND_HANDLE_WRITE

```mach
pub val OP_OPERAND_HANDLE_WRITE:   OpOperandKind = 7
```

## val OP_OPERAND_BOOL

```mach
pub val OP_OPERAND_BOOL:           OpOperandKind = 8
```

## val OP_OPERAND_SCOPE

```mach
pub val OP_OPERAND_SCOPE:          OpOperandKind = 9
```

## val OP_OPERAND_SEMANTICS

```mach
pub val OP_OPERAND_SEMANTICS:      OpOperandKind = 10
```

## val OP_OPERAND_KIND_COUNT

```mach
pub val OP_OPERAND_KIND_COUNT:     OpOperandKind = 11
```

## def OpResult

```mach
pub def OpResult: u8
```

whether an `#[op]` instruction has a result id, and whether that result is a
pointer into the space the row declares, or a truth value of the target's boolean
type, which the declaration's integer return type receives as 1 or 0

## val OP_RESULT_VALUE

```mach
pub val OP_RESULT_VALUE:   OpResult = 0
```

## val OP_RESULT_NONE

```mach
pub val OP_RESULT_NONE:    OpResult = 1
```

## val OP_RESULT_POINTER

```mach
pub val OP_RESULT_POINTER: OpResult = 2
```

## val OP_RESULT_BOOL

```mach
pub val OP_RESULT_BOOL:    OpResult = 3
```

## val OP_MAX_OPERANDS

```mach
pub val OP_MAX_OPERANDS: u32 = 16
```

## val NO_OP_SPACE

```mach
pub val NO_OP_SPACE: u32 = 0xFFFFFFFF
```

## val NO_OP_CAPABILITY

```mach
pub val NO_OP_CAPABILITY: u32 = 0xFFFFFFFF
```

## rec OpEnumerant

```mach
pub rec OpEnumerant;
```

one value a literal operand may take and what that value brings: the target's
own capability word it needs (NO_OP_CAPABILITY for none), the extensions of the
isa's vocabulary that capability needs selected, and the operands it adds to the end
of the instruction, one letter each as `op_def_shaped` spells them (`""` for none).
what a value brings is never a pointer or a handle: a row's memory effects are known
before a call's literal is

## rec OpEnum

```mach
pub rec OpEnum;
```

the closed set of values a literal operand takes. a value enumeration admits
exactly one of its values. a mask admits any union of its single-bit values,
each set bit bringing its own requirement and operands, which follow in
ascending bit order, and 0 is the empty mask

## def OpDataClass

```mach
pub def OpDataClass: u8
```

the class of the data type a typed row operates on: an integer, a float, or anything
else (a vector, an aggregate, a handle), which no requirement admits

## val OP_DATA_INT

```mach
pub val OP_DATA_INT:   OpDataClass = 0
```

## val OP_DATA_FLOAT

```mach
pub val OP_DATA_FLOAT: OpDataClass = 1
```

## val OP_DATA_OTHER

```mach
pub val OP_DATA_OTHER: OpDataClass = 2
```

## rec OpTypeRequirement

```mach
pub rec OpTypeRequirement;
```

one data type a typed row admits in one of the target's address spaces (NO_OP_SPACE
for every space), and what a use of it there needs: the target's capability word
(NO_OP_CAPABILITY for none) and the extensions of the isa's vocabulary. `refused`,
when not nil, is why the target cannot emit the type yet, and the declaration is
refused with it

## rec OpTyping

```mach
pub rec OpTyping;
```

the data types a row operates on, read from its operand `operand` (a pointer's
pointee), and what each needs in each address space. a type no requirement admits in
any space is refused at the declaration, and one admitted only in other spaces at the
call. a typing with no requirements admits every type and needs nothing: it only names
the operand the row's relations are stated against. static data the target owns for
the life of the program, like an OpEnum

## def OpRelation

```mach
pub def OpRelation: u8
```

how an operand's or the result's type relates to the row's data type, the type of its
typing's operand (a pointer operand's pointee): unrelated, that same type, a texel, a
scalar or vector whose component is the sampled type of the handle the typing's operand
is, with as many components as the row's texel count states, or, for a pointer result
only, a pointer to that sampled type, into the memory of the handle the typing's pointer
operand addresses

## val OP_RELATION_NONE

```mach
pub val OP_RELATION_NONE:    OpRelation = 0
```

## val OP_RELATION_DATA

```mach
pub val OP_RELATION_DATA:    OpRelation = 1
```

## val OP_RELATION_TEXEL

```mach
pub val OP_RELATION_TEXEL:   OpRelation = 2
```

## val OP_RELATION_POINTEE

```mach
pub val OP_RELATION_POINTEE: OpRelation = 3
```

## val OP_RELATION_COUNT

```mach
pub val OP_RELATION_COUNT:   OpRelation = 4
```

## def OpCountKind

```mach
pub def OpCountKind: u8
```

how a row's texel count is stated: exactly a number of components, or at least as many
as the handle's format stores, any number when its format is unknown

## val OP_COUNT_EXACT

```mach
pub val OP_COUNT_EXACT:      OpCountKind = 0
```

## val OP_COUNT_FORMAT

```mach
pub val OP_COUNT_FORMAT:     OpCountKind = 1
```

## val OP_COUNT_KIND_COUNT

```mach
pub val OP_COUNT_KIND_COUNT: OpCountKind = 2
```

## rec OpTexelCount

```mach
pub rec OpTexelCount;
```

the number of components a row's texel has, a scalar being one: `components` exactly,
or as `kind` says. `rule` names the rule a refusal cites. static data the target owns
for the life of the program, like an OpTyping

## rec OpScalar

```mach
pub rec OpScalar;
```

a scalar data type by class, width and signedness, such as a handle's sampled type

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

## def TypeBindFn

```mach
pub def TypeBindFn: fun(*u32, u32) u32
```

the descriptor role a handle binds through, from its operands

## def TypeComposeFn

```mach
pub def TypeComposeFn: fun(u32, *u32, u32) str
```

why a composing constructor refuses the handle named as its operand `index`, from
that handle's own operands, nil when it composes over it

## def TypeSampledFn

```mach
pub def TypeSampledFn: fun(*u32, u32) OpScalar
```

the scalar a handle's texels are read and written as, from its operands, class
OP_DATA_OTHER when it has none

## def TypeComponentsFn

```mach
pub def TypeComponentsFn: fun(*u32, u32) u32
```

the number of components a handle's format stores each texel in, from its operands, 0
when its format is unknown

## val NO_TEXEL_OPERAND

```mach
pub val NO_TEXEL_OPERAND: u32 = 0xFFFFFFFF
```

a constructor whose handles state their own texels, or have none

## def TypeAddressFn

```mach
pub def TypeAddressFn: fun(*u32, u32) str
```

why no instruction may derive a pointer into a handle's memory, from its operands, nil
when one may

## val HANDLE_BIND_SAMPLER

```mach
pub val HANDLE_BIND_SAMPLER: u32 = 0
```

a handle bound as a sampled descriptor, `#[sampler(set, binding)]`

## val HANDLE_BIND_STORAGE

```mach
pub val HANDLE_BIND_STORAGE: u32 = 1
```

a handle bound as a storage descriptor, `#[storage(set, binding, ...)]`

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

## fun op_def

```mach
pub fun op_def(set: str, name: str, set_tag: u32, opcode: u32, arity: u32) OpDef;
```

a row whose every operand is a value and whose result is a value

## fun op_def_shaped

```mach
pub fun op_def_shaped(set: str, name: str, set_tag: u32, opcode: u32,
result: OpResult, result_space: u32, signature: str) OpDef;
```

a row spelled by its operand signature, one letter per operand in order: `v` a
value, `c` a constant id, `l` a literal word, `r` a pointer only read through, `w`
a pointer only stored through, `u` a pointer read and written (read-modify-write),
`i` a handle whose memory is read, `o` a handle whose memory is written, `b` a
truth value, `s` a memory scope and `m` a memory semantics, each a constant id.
a row spells only its own operands: the ones an enumerated literal's value brings
follow them, typed by the enumerant (`op_enumerated`). a `|` makes the literal after
it optional (`"iv|l"`), a call passing it or leaving it out with every operand its
value would bring, and nothing else follows it.
a letter outside the set is an invalid kind, which registration refuses

## fun op_requiring

```mach
pub fun op_requiring(d: OpDef, capability: u32, requires: u64) OpDef;
```

`d` needing the target's capability word `capability` and the extensions `requires` on every use

## fun op_enumerated

```mach
pub fun op_enumerated(d: OpDef, operand: u32, e: *OpEnum) OpDef;
```

`d` with its literal operand `operand` held to the values of `e`, and room after the
operands it spells for the most every enumerated literal's value brings

## fun op_typed

```mach
pub fun op_typed(d: OpDef, t: *OpTyping) OpDef;
```

`d` operating on the data types `t` admits, each with its own requirement

## fun op_related

```mach
pub fun op_related(d: OpDef, result: OpRelation, operands: str) OpDef;
```

`d` with its result's type related to its data type by `result`, and each operand's by
one letter of `operands` in order: `-` unrelated, `=` the data type itself, `t` a texel
of the handle the typing's operand is. a letter outside the set is an invalid relation,
which registration refuses

## fun op_texel_counted

```mach
pub fun op_texel_counted(d: OpDef, count: *OpTexelCount) OpDef;
```

`d` with the texel it relates holding `count` components

## fun op_texel_count_admits

```mach
pub fun op_texel_count_admits(count: *OpTexelCount, n: u32, stored: u32) bool;
```

whether `n` components meet `count`, against a handle whose format stores `stored`, 0
when its format is unknown

## fun op_operand_relation

```mach
pub fun op_operand_relation(d: *OpDef, i: u32) OpRelation;
```

## fun op_typing_constrains

```mach
pub fun op_typing_constrains(t: *OpTyping) bool;
```

whether `t` holds the row's data type to requirements, rather than only naming its operand

## fun op_type_requirement

```mach
pub fun op_type_requirement(t: *OpTyping, class: OpDataClass, bits: u32, space: u32) *OpTypeRequirement;
```

the requirement of `t` admitting a `class` of `bits` in `space`, nil when none does.
`space` NO_OP_SPACE asks whether any space admits the type

## fun op_operand_enum

```mach
pub fun op_operand_enum(d: *OpDef, i: u32) *OpEnum;
```

## fun op_tail_literal

```mach
pub fun op_tail_literal(d: *OpDef) *OpEnum;
```

the enumeration of the optional literal that leads `d`'s tail, nil when the tail has none

## fun op_enumerant_brings

```mach
pub fun op_enumerant_brings(v: *OpEnumerant) u32;
```

how many operands the enumerant `v` brings

## fun op_enumerant_kind

```mach
pub fun op_enumerant_kind(v: *OpEnumerant, k: u32) OpOperandKind;
```

the kind of the `k`th operand the enumerant `v` brings

## fun op_enumerant_of

```mach
pub fun op_enumerant_of(e: *OpEnum, value: u32) *OpEnumerant;
```

the enumerant of `e` whose value is exactly `value`, nil when none is

## fun op_enum_admits

```mach
pub fun op_enum_admits(e: *OpEnum, value: u32, out_trailing: *u32) bool;
```

whether `value` is a value of `e`: one of its values, or for a mask a union of its
bits. `out_trailing` receives the tail operands the value brings

## val NO_OPERAND

```mach
pub val NO_OPERAND: u32 = 0xFFFFFFFF
```

## fun op_literal_span

```mach
pub fun op_literal_span(d: *OpDef, argc: u32) u32;
```

how many of a call's leading operands are enumerated-literal positions it passes: the
required operands, and the literal leading the tail when the call passes more than them

## fun op_call_arity

```mach
pub fun op_call_arity(d: *OpDef, argc: u32, words: *u32, out_bad: *u32) u32;
```

the operand count a call of `d` takes with the literal words `words` it passes:
the operands before its tail, the literal leading the tail when the call passes one,
and every operand an enumerated value brings. `out_bad` receives the first
enumerated operand whose word is no value of its enumeration, else NO_OPERAND

## fun op_call_kinds

```mach
pub fun op_call_kinds(d: *OpDef, argc: u32, words: *u32, out: *OpOperandKind);
```

the kind of each operand of a call of `d` passing `argc` operands with the literal
words `words`, into `out` (OP_MAX_OPERANDS long): the operands the row spells by the
row, then each operand a literal's value brings by the enumerant that brings it, in
the order of the literals and of each mask's bits. a call `op_call_arity` refuses
has the kinds of the operands it does take, and the rest are values

## fun op_operand_kind

```mach
pub fun op_operand_kind(d: *OpDef, i: u32) OpOperandKind;
```

the kind of operand `i` as the row spells it. an operand a literal's value brings is
typed by its enumerant (`op_call_kinds`), and is never a pointer or a handle, so it
reads here as a value

## fun op_kind_is_pointer

```mach
pub fun op_kind_is_pointer(k: OpOperandKind) bool;
```

## fun op_kind_is_constant

```mach
pub fun op_kind_is_constant(k: OpOperandKind) bool;
```

an id that must be an integer constant by emission, whatever the target reads its value as

## fun op_kind_is_handle

```mach
pub fun op_kind_is_handle(k: OpOperandKind) bool;
```

## fun op_def_shape_refusal

```mach
pub fun op_def_shape_refusal(d: *OpDef) str;
```

the reason a row's shape is malformed, nil when it is well formed

## fun op_def_by_code

```mach
pub fun op_def_by_code(defs: *TargetDefs, set_tag: u32, opcode: u32) *OpDef;
```

## fun type_def

```mach
pub fun type_def(name: str, tag: u32, operands: *u32, arity: u32, refuse: TypeRefuseFn) TypeDef;
```

## fun type_def_binding

```mach
pub fun type_def_binding(td: *TypeDef, ops: *u32, n: u32) u32;
```

the role a handle of constructor `td` with operands `ops` binds through

## fun type_def_sampled

```mach
pub fun type_def_sampled(td: *TypeDef, ops: *u32, n: u32) OpScalar;
```

the scalar texels of a handle of constructor `td` with operands `ops` are, class
OP_DATA_OTHER when the constructor declares none

## fun type_def_components

```mach
pub fun type_def_components(td: *TypeDef, ops: *u32, n: u32) u32;
```

the number of components a handle of constructor `td` with operands `ops` stores each
texel in, 0 when its format is unknown or the constructor declares none

## fun type_def_address

```mach
pub fun type_def_address(td: *TypeDef, ops: *u32, n: u32) str;
```

why no instruction may derive a pointer into a handle of constructor `td` with operands
`ops`, nil when one may

## fun target_defs

```mach
pub fun target_defs(ops: *OpDef, op_count: u32, types: *TypeDef, type_count: u32) TargetDefs;
```

## fun type_def_of_region

```mach
pub fun type_def_of_region(defs: *TargetDefs, src: str, off: usize, len: usize) *TypeDef;
```

## fun type_def_by_tag

```mach
pub fun type_def_by_tag(defs: *TargetDefs, tag: u32) *TypeDef;
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

## fun with_int_imm_rule

```mach
pub fun with_int_imm_rule(m: *RegMachine, f: IntImmFitsFn);
```

## fun with_const_operand_rule

```mach
pub fun with_const_operand_rule(m: *RegMachine, f: ReadsConstFn);
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

## fun with_branch_thunks

```mach
pub fun with_branch_thunks(s: *RelocSeam, reach: of.BranchReachFn, thunk: of.BranchThunkFn);
```

## fun declares_branch_thunks

```mach
pub fun declares_branch_thunks(tgt_isa: *IsaVTable) bool;
```

## fun branch_reach

```mach
pub fun branch_reach(tgt_isa: *IsaVTable, kind: of.RelocKind) opt[of.BranchReach];
```

the reach of a direct branch a thunk can extend, none for any other kind or
an instruction set that places no thunks

## fun with_machine_flags

```mach
pub fun with_machine_flags(s: *RelocSeam, f: of.MachineFlagsFn);
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
pub fun module_emitter(emit_module: EmitModuleFn, has_assembly: bool) ModuleEmitter;
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

