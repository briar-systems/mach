# mach.lang.target.model

the machine model of an instruction set: its widths, registers, vector
forms, constant-time multiply rows and extension selection, as plain data
the front end, the middle end and every backend read

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

## rec Register

```mach
pub rec Register;
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
is the unfused path, one extension per lane into a rebuilt vector

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
each: a uniform count (`_U`) is one scalar applied to every lane, the
form every baseline set has an instruction for, and a per-lane count (`_V`)
is a vector of counts, one per lane. a count at or above the lane width
saturates in every form, as the scalar shift does

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
each lane extracted and the result built or written through memory

## val VEC_OP_CONCAT

```mach
pub val VEC_OP_CONCAT: VecOp = 33
```

a lane join: the lanes of each operand in turn, all of one lane type, in one
vector of their summed count. it is how a vector wider than the register is
put back together from its register-width pieces, and how a narrowing
conversion over those pieces gathers its halves. a cell names the
lane kind and width, which the operation keeps; its scalar row is the lane
path through memory

## val VEC_OP_MUL_HIGH_S

```mach
pub val VEC_OP_MUL_HIGH_S: VecOp = 34
```

the high half of the full product of two integer lanes, in lanes of their
own width, under the operation's signedness: the upper word of the widening
multiply, whose lower word is the plain multiply. a cell names the lane width
twice; its scalar row is each lane's own high multiply

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
operand lane width; its scalar row is each lane joined on its own

## val VEC_OP_WIDEN_SUM_U

```mach
pub val VEC_OP_WIDEN_SUM_U: VecOp = 37
```

a widening group sum: each result lane the zero-extended sum of the operand
integer lanes it covers, so the result holds the operand's bits in fewer,
wider lanes. a cell names the result lane width and the operand lane width;
its scalar row is the per-lane sum, and only a packed cell is ever formed,
as the fold of a count accumulated in narrow lanes

## val VEC_OP_SIGN_MASK

```mach
pub val VEC_OP_SIGN_MASK: VecOp = 38
```

a lane sign mask: each integer lane all ones when it is negative, else zero,
in lanes of its own width. it is the high half a signed lane-halving
extension interleaves its lanes with, so both halves of one extension share
it; its scalar row is each lane shifted right by its width less one, and
only a packed cell is ever formed

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
the register it fits reads the widest selected row. `upper_clear`
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
splits it only past it. the kinds widen apart on a target whose
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
and none the 128-bit one

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
multiply beside the low one, or a fixed register pair; a
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

## rec Machine

```mach
pub rec Machine;
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

## fun packed_form_valid

```mach
pub fun packed_form_valid(op: VecOp, is_float: bool, lane_bits: u32, from_bits: u32) bool;
```

## fun half_forms_valid

```mach
pub fun half_forms_valid(m: *Machine) bool;
```

## fun half_native

```mach
pub fun half_native(m: *Machine, op: VecOp, is_float: bool, lane_bits: u32, from_bits: u32) bool;
```

the target realizes this scalar f16 cell with its own instruction under the
selected extensions; false is the inlined expansion

## fun vector_register_bytes

```mach
pub fun vector_register_bytes(m: *Machine) u32;
```

the widest vector one register carries under the selected extensions: the
compute width, or a selected row's wider register

## fun vector_register_bits

```mach
pub fun vector_register_bits(m: *Machine) u32;
```

the widest vector register the selected extensions give, in bits: the
layout aligns a vector as wide as one to it

## fun vector_upper_clear

```mach
pub fun vector_upper_clear(m: *Machine, bytes: u32) bool;
```

a value `bytes` wide moved out of its register leaves upper state the
target clears afterwards

## fun vector_register_rows_valid

```mach
pub fun vector_register_rows_valid(m: *Machine) bool;
```

every row names an extension the isa has and a power-of-two width above the
compute width, and no width twice

## fun vector_width

```mach
pub fun vector_width(m: *Machine, is_float: bool, lane_bits: u32) u32;
```

the width in bits the packed rows over lanes of this kind and width run at
under the selected extensions: the widest selected row's, or `vector_bits`

## fun vector_lanes_valid

```mach
pub fun vector_lanes_valid(m: *Machine) bool;
```

a vector of the declared lane count at the widest lane, 64 bits, spans a
width a u32 holds, which a target whose vectors are values packs at

## fun vector_width_rows_valid

```mach
pub fun vector_width_rows_valid(m: *Machine) bool;
```

every row names an extension the isa has, lanes of the kind's widths, and
a power-of-two width above `vector_bits` that a register the row's
extensions select holds

## fun packed_forms_valid

```mach
pub fun packed_forms_valid(m: *Machine) bool;
```

## fun scalar_forms_valid

```mach
pub fun scalar_forms_valid(m: *Machine) bool;
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

## def ScalarFamily

```mach
pub def ScalarFamily: u32
```

the families of retained cells a model leaves to the scalar path wherever
its packed table does not claim them: the conversions; the widening
multiplies with their high halves, the lane interleave, the lane-halving
extensions with their sign masks and the widening group sums; the shifts, lane by lane through the
scalar shift, which saturates the same way; the lane ranges and joins; and
the f16 lane arithmetic and comparisons, each lane's scalar f16 operation

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

## fun scalar_family_has

```mach
pub fun scalar_family_has(families: ScalarFamily, cell: *ScalarForm) bool;
```

whether `cell` is in one of `families`

## fun is_widening_cell

```mach
pub fun is_widening_cell(cell: *ScalarForm) bool;
```

## fun ct_mul_rows_admit

```mach
pub fun ct_mul_rows_admit(m: *Machine, dit_mode: bool) ct.CtMulMask;
```

the cells a model's rows admit once each condition is decided: ALWAYS
holds, EXTENSION holds when the model selects every named extension, and
DIT_MODE holds when the caller says the mode is guaranteed on

## fun ct_mul_rows_under

```mach
pub fun ct_mul_rows_under(m: *Machine, cond: ct.CtMulCond) ct.CtMulMask;
```

the cells whose rows carry one condition, decided or not: what a DIT_MODE
multiply would need before it is admitted. a realized cell needs the
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
pub fun extension_view(m: *Machine) ExtensionView;
```

## fun extension_domain

```mach
pub fun extension_domain(m: *Machine) u64;
```

every extension bit the model's vocabulary names

## fun extension_bit

```mach
pub fun extension_bit(m: *Machine, name: str, len: usize) u64;
```

the bit the model's vocabulary gives `name[0..len]`, 0 when it names no such extension

## fun extension_bundle

```mach
pub fun extension_bundle(m: *Machine, name: str, len: usize) u64;
```

what a manifest's `extensions` entry `name[0..len]` selects: a level's
members when the model publishes one by that spelling, else the row's bit,
0 when the vocabulary has neither

## fun spelling_finish

```mach
pub fun spelling_finish(b: *textbuild.TextBuilder, r: err[textbuild.Error]) res[str, fail.Fail];
```

the text a spelling built into `b`, owned by `b`'s allocator (released with
str_free), or the failure that stopped it; `b` is released either way

## fun level_names

```mach
pub fun level_names(a: *A.Allocator, m: *Machine) res[str, fail.Fail];
```

the model's levels, `, `-separated, empty when it publishes none; owned by `a`

## fun extension_close

```mach
pub fun extension_close(m: *Machine, bits: u64) u64;
```

`bits` closed over the model's vocabulary: every extension a selected one implies

## fun extension_names

```mach
pub fun extension_names(a: *A.Allocator, m: *Machine) res[str, fail.Fail];
```

the model's vocabulary, `, `-separated, or `none`; owned by `a`

## fun table_names

```mach
pub fun table_names(a: *A.Allocator, table: *extension.Extension, count: u32) res[str, fail.Fail];
```

the names of an extension table, `, `-separated; owned by `a`

## rec DecoratorExtension

```mach
pub rec DecoratorExtension;
```

what `#[extensions(name)]` may mean: the selected isa's row when it has one,
else any row of that name in the vocabulary, which admits nothing here. a row
only a target selects is never a decorator's, and `target_only` says why

## fun decorator_extension

```mach
pub fun decorator_extension(vocabulary: *extension.Vocabulary, selected: ExtensionView, name: str, len: usize) DecoratorExtension;
```

## fun vocabulary_names

```mach
pub fun vocabulary_names(a: *A.Allocator, vocabulary: *extension.Vocabulary) res[str, fail.Fail];
```

every name of `vocabulary`, `, `-separated; owned by `a`

## fun ct_mul_forms_refusal

```mach
pub fun ct_mul_forms_refusal(m: *Machine) opt[str];
```

why a model's constant-time multiply rows are malformed; the extension
domain is every extension bit the target model knows

## fun vector_domain_complete

```mach
pub fun vector_domain_complete(m: *Machine) bool;
```

## fun from_is_float

```mach
pub fun from_is_float(op: VecOp, is_float: bool) bool;
```

the lane kind of a cell's operand: a conversion between the kinds reads
the other kind, and every other cell its result's

## fun packed_width

```mach
pub fun packed_width(m: *Machine, op: VecOp, is_float: bool, lane_bits: u32, from_bits: u32) u32;
```

a conversion's register holds its wider side, so that side sets the extent,
and each side's lanes stay within its own kind's width

## fun packing_extension

```mach
pub fun packing_extension(m: *Machine, op: VecOp, is_float: bool, lane_bits: u32, from_bits: u32) u64;
```

the extensions a target would add to pack this cell: what the first declared
row of the cell needs beyond the model's selection. 0 when a selected row
packs it already or no row declares it at any extension

## fun packed_row_selected_by

```mach
pub fun packed_row_selected_by(m: *Machine, op: VecOp, is_float: bool, lane_bits: u32, from_bits: u32, ext: u64) bool;
```

whether the model selects the row of this cell gated on exactly `ext`: the
lowering's choice between the baseline form and an extension's instruction

## fun extends_directly

```mach
pub fun extends_directly(m: *Machine) bool;
```

a lane-wise integer extension of any ratio is one instruction under the
selected extensions, rather than a chain of doublings

## fun vector_form

```mach
pub fun vector_form(m: *Machine, op: VecOp, is_float: bool, lane_bits: u32, from_bits: u32) VectorForm;
```

the one declared outcome for a cell: packed, the scalar expansion, or nothing

## fun packed_lane_cap

```mach
pub fun packed_lane_cap(m: *Machine, is_float: bool, lane_bits: u32) u32;
```

## fun vector_op_bytes

```mach
pub fun vector_op_bytes(m: *Machine, is_float: bool, lane_bits: u32, bytes: u32) u32;
```

the bytes of the register that realizes a vector of `bytes` over lanes of
this kind whole: the narrowest register width the kind computes at that
holds it, `vector_op_bytes` at the least. a vector wider than every such
register is not realized in one and keeps the narrowest

## fun moves_unaligned_gp

```mach
pub fun moves_unaligned_gp(m: *Machine, bytes: u32) bool;
```

## fun offset_fits

```mach
pub fun offset_fits(bits: u32, off: i64) bool;
```

a constant offset fits a signed field of `bits` bits; 0 bits is no bound

## fun sym_offset_folds

```mach
pub fun sym_offset_folds(m: *Machine, off: i64) bool;
```

## fun mem_disp_folds

```mach
pub fun mem_disp_folds(m: *Machine, off: i64) bool;
```

## fun moves_vector_memory

```mach
pub fun moves_vector_memory(m: *Machine, bytes: u32) bool;
```

a vector of `bytes` moves between memory and its register in one access:
a width the target declares, or a register width its extensions give past
the compute width

## fun reads_slot_operand

```mach
pub fun reads_slot_operand(m: *Machine, bytes: u32) bool;
```

## fun shift_count_mod_width

```mach
pub fun shift_count_mod_width(m: *Machine, bytes: u32) bool;
```

## fun moves_cross_bank

```mach
pub fun moves_cross_bank(m: *Machine, bytes: u32) bool;
```

## fun declare_alu_widths

```mach
pub fun declare_alu_widths(m: *Machine, widths: u32);
```

declares the alu widths `widths` (encoded like int_widths, a run of adjacent
widths), each one legal under every extension set, in place of any declared
before

## fun alu_min_width

```mach
pub fun alu_min_width(m: *Machine) u32;
```

the narrowest legal alu width, 0 for a model that declares none

## fun max_alu_width

```mach
pub fun max_alu_width(m: *Machine) u32;
```

the widest legal alu width, above which legalize splits an operation into
lanes; 0 for a model that declares none

## fun clamp_alu_width

```mach
pub fun clamp_alu_width(m: *Machine, w: u8) u8;
```

the width an integer operation of `w` bytes computes at: w itself when it is
legal, otherwise the narrowest width above it that is legal whatever the
extensions, so a lift never needs a capability the program did not spell; w
again when none is (legalize splits it)

## fun realizes_int_width

```mach
pub fun realizes_int_width(m: *Machine, bytes: u32) bool;
```

the only reader of int_widths: whether the target realizes a scalar integer
of this many bytes

## fun fits_vector_register

```mach
pub fun fits_vector_register(m: *Machine, is_float: bool, lane_bits: u32, lanes: u32) bool;
```

