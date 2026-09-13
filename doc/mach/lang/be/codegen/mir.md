# mach.lang.be.codegen.mir

## def MirOperandKind

```mach
pub def MirOperandKind: u8
```

## val MIR_OP_VREG

```mach
pub val MIR_OP_VREG:  MirOperandKind = 0
```

## val MIR_OP_PREG

```mach
pub val MIR_OP_PREG:  MirOperandKind = 1
```

## val MIR_OP_IMM

```mach
pub val MIR_OP_IMM:   MirOperandKind = 2
```

## val MIR_OP_MEM

```mach
pub val MIR_OP_MEM:   MirOperandKind = 3
```

## val MIR_OP_SYM

```mach
pub val MIR_OP_SYM:   MirOperandKind = 4
```

## val MIR_OP_BLOCK

```mach
pub val MIR_OP_BLOCK: MirOperandKind = 5
```

## def MirOpcode

```mach
pub def MirOpcode: u32
```

## val MIR_ADD

```mach
pub val MIR_ADD:         MirOpcode = 0
```

## val MIR_SUB

```mach
pub val MIR_SUB:         MirOpcode = 1
```

## val MIR_MUL

```mach
pub val MIR_MUL:         MirOpcode = 2
```

## val MIR_DIV_S

```mach
pub val MIR_DIV_S:       MirOpcode = 3
```

## val MIR_DIV_U

```mach
pub val MIR_DIV_U:       MirOpcode = 4
```

## val MIR_REM_S

```mach
pub val MIR_REM_S:       MirOpcode = 5
```

## val MIR_REM_U

```mach
pub val MIR_REM_U:       MirOpcode = 6
```

## val MIR_NEG

```mach
pub val MIR_NEG:         MirOpcode = 7
```

## val MIR_AND

```mach
pub val MIR_AND:         MirOpcode = 8
```

## val MIR_OR

```mach
pub val MIR_OR:          MirOpcode = 9
```

## val MIR_XOR

```mach
pub val MIR_XOR:         MirOpcode = 10
```

## val MIR_SHL

```mach
pub val MIR_SHL:         MirOpcode = 11
```

## val MIR_SHR_S

```mach
pub val MIR_SHR_S:       MirOpcode = 12
```

## val MIR_SHR_U

```mach
pub val MIR_SHR_U:       MirOpcode = 13
```

## val MIR_NOT

```mach
pub val MIR_NOT:         MirOpcode = 14
```

## val MIR_CMP_EQ

```mach
pub val MIR_CMP_EQ:      MirOpcode = 15
```

## val MIR_CMP_NE

```mach
pub val MIR_CMP_NE:      MirOpcode = 16
```

## val MIR_CMP_LT_S

```mach
pub val MIR_CMP_LT_S:    MirOpcode = 17
```

## val MIR_CMP_LT_U

```mach
pub val MIR_CMP_LT_U:    MirOpcode = 18
```

## val MIR_CMP_LE_S

```mach
pub val MIR_CMP_LE_S:    MirOpcode = 19
```

## val MIR_CMP_LE_U

```mach
pub val MIR_CMP_LE_U:    MirOpcode = 20
```

## val MIR_TRUNC

```mach
pub val MIR_TRUNC:       MirOpcode = 21
```

## val MIR_SEXT

```mach
pub val MIR_SEXT:        MirOpcode = 22
```

## val MIR_ZEXT

```mach
pub val MIR_ZEXT:        MirOpcode = 23
```

## val MIR_FP_TRUNC

```mach
pub val MIR_FP_TRUNC:    MirOpcode = 24
```

## val MIR_FP_EXT

```mach
pub val MIR_FP_EXT:      MirOpcode = 25
```

## val MIR_FP_TO_SI

```mach
pub val MIR_FP_TO_SI:    MirOpcode = 26
```

## val MIR_FP_TO_UI

```mach
pub val MIR_FP_TO_UI:    MirOpcode = 27
```

## val MIR_SI_TO_FP

```mach
pub val MIR_SI_TO_FP:    MirOpcode = 28
```

## val MIR_UI_TO_FP

```mach
pub val MIR_UI_TO_FP:    MirOpcode = 29
```

## val MIR_BITCAST

```mach
pub val MIR_BITCAST:     MirOpcode = 30
```

## val MIR_ALLOCA

```mach
pub val MIR_ALLOCA:      MirOpcode = 31
```

## val MIR_LOAD

```mach
pub val MIR_LOAD:        MirOpcode = 32
```

## val MIR_STORE

```mach
pub val MIR_STORE:       MirOpcode = 33
```

## val MIR_GEP

```mach
pub val MIR_GEP:         MirOpcode = 34
```

## val MIR_EXTRACT

```mach
pub val MIR_EXTRACT:     MirOpcode = 35
```

## val MIR_INSERT

```mach
pub val MIR_INSERT:      MirOpcode = 36
```

## val MIR_PHI

```mach
pub val MIR_PHI:         MirOpcode = 37
```

## val MIR_CALL_IR

```mach
pub val MIR_CALL_IR:     MirOpcode = 38
```

## val MIR_BR

```mach
pub val MIR_BR:          MirOpcode = 39
```

## val MIR_CBR

```mach
pub val MIR_CBR:         MirOpcode = 40
```

## val MIR_RET

```mach
pub val MIR_RET:         MirOpcode = 41
```

## val MIR_UNREACHABLE

```mach
pub val MIR_UNREACHABLE: MirOpcode = 42
```

## val MIR_ASM

```mach
pub val MIR_ASM:         MirOpcode = 43
```

## val MIR_MOV

```mach
pub val MIR_MOV:        MirOpcode = 0x1000
```

## val MIR_ARG

```mach
pub val MIR_ARG:        MirOpcode = 0x1001
```

## val MIR_CALL

```mach
pub val MIR_CALL:       MirOpcode = 0x1002
```

## val MIR_CALL_RES

```mach
pub val MIR_CALL_RES:   MirOpcode = 0x1003
```

## val MIR_DECLASSIFY

```mach
pub val MIR_DECLASSIFY: MirOpcode = 0x1004
```

## val MIR_VEC_EXTRACT

```mach
pub val MIR_VEC_EXTRACT: MirOpcode = 0x1005
```

## val MIR_VEC_INSERT

```mach
pub val MIR_VEC_INSERT:  MirOpcode = 0x1006
```

## val MIR_VEC_BUILD

```mach
pub val MIR_VEC_BUILD:   MirOpcode = 0x1007
```

## val MIR_MUL_HI_U

```mach
pub val MIR_MUL_HI_U: MirOpcode = 0x1008
```

## val MIR_SEL_ADD

```mach
pub val MIR_SEL_ADD:      MirOpcode = 0x1100
```

## val MIR_SEL_SUB

```mach
pub val MIR_SEL_SUB:      MirOpcode = 0x1101
```

## val MIR_SEL_MUL

```mach
pub val MIR_SEL_MUL:      MirOpcode = 0x1102
```

## val MIR_SEL_AND

```mach
pub val MIR_SEL_AND:      MirOpcode = 0x1103
```

## val MIR_SEL_OR

```mach
pub val MIR_SEL_OR:       MirOpcode = 0x1104
```

## val MIR_SEL_XOR

```mach
pub val MIR_SEL_XOR:      MirOpcode = 0x1105
```

## val MIR_SEL_SHL

```mach
pub val MIR_SEL_SHL:      MirOpcode = 0x1106
```

## val MIR_SEL_SHR_U

```mach
pub val MIR_SEL_SHR_U:    MirOpcode = 0x1107
```

## val MIR_SEL_SHR_S

```mach
pub val MIR_SEL_SHR_S:    MirOpcode = 0x1108
```

## val MIR_SEL_DIV_S

```mach
pub val MIR_SEL_DIV_S:    MirOpcode = 0x1109
```

## val MIR_SEL_DIV_U

```mach
pub val MIR_SEL_DIV_U:    MirOpcode = 0x110A
```

## val MIR_SEL_REM_S

```mach
pub val MIR_SEL_REM_S:    MirOpcode = 0x110B
```

## val MIR_SEL_REM_U

```mach
pub val MIR_SEL_REM_U:    MirOpcode = 0x110C
```

## val MIR_SEL_CMP_EQ

```mach
pub val MIR_SEL_CMP_EQ:   MirOpcode = 0x110D
```

## val MIR_SEL_CMP_NE

```mach
pub val MIR_SEL_CMP_NE:   MirOpcode = 0x110E
```

## val MIR_SEL_CMP_LT_S

```mach
pub val MIR_SEL_CMP_LT_S: MirOpcode = 0x110F
```

## val MIR_SEL_CMP_LT_U

```mach
pub val MIR_SEL_CMP_LT_U: MirOpcode = 0x1110
```

## val MIR_SEL_CMP_LE_S

```mach
pub val MIR_SEL_CMP_LE_S: MirOpcode = 0x1111
```

## val MIR_SEL_CMP_LE_U

```mach
pub val MIR_SEL_CMP_LE_U: MirOpcode = 0x1112
```

## val MIR_SEL_CBR

```mach
pub val MIR_SEL_CBR:      MirOpcode = 0x1113
```

## val MIR_SEL_CBR_EQ

```mach
pub val MIR_SEL_CBR_EQ:   MirOpcode = 0x1120
```

## val MIR_SEL_CBR_NE

```mach
pub val MIR_SEL_CBR_NE:   MirOpcode = 0x1121
```

## val MIR_SEL_CBR_LT_S

```mach
pub val MIR_SEL_CBR_LT_S: MirOpcode = 0x1122
```

## val MIR_SEL_CBR_LT_U

```mach
pub val MIR_SEL_CBR_LT_U: MirOpcode = 0x1123
```

## val MIR_SEL_CBR_LE_S

```mach
pub val MIR_SEL_CBR_LE_S: MirOpcode = 0x1124
```

## val MIR_SEL_CBR_LE_U

```mach
pub val MIR_SEL_CBR_LE_U: MirOpcode = 0x1125
```

## fun is_cmp_opcode

```mach
pub fun is_cmp_opcode(op: MirOpcode) bool;
```

## fun operand_requires_reg

```mach
pub fun operand_requires_reg(op: MirOpcode, idx: u32) bool;
```

## fun is_fused_cbr

```mach
pub fun is_fused_cbr(op: MirOpcode) bool;
```

## fun fused_cbr_opcode

```mach
pub fun fused_cbr_opcode(cmp: MirOpcode) MirOpcode;
```

## fun instr_declassifies

```mach
pub fun instr_declassifies(mi: *MirInstr) bool;
```

the only downgrade: the declassify opcode before selection, the flag after

## fun instr_defines_shape

```mach
pub fun instr_defines_shape(mi: *MirInstr) bool;
```

## fun is_signed_cmp

```mach
pub fun is_signed_cmp(op: MirOpcode) bool;
```

## val MIR_FADD

```mach
pub val MIR_FADD: MirOpcode = 0x1114
```

## val MIR_FSUB

```mach
pub val MIR_FSUB: MirOpcode = 0x1115
```

## val MIR_FMUL

```mach
pub val MIR_FMUL: MirOpcode = 0x1116
```

## val MIR_FDIV

```mach
pub val MIR_FDIV: MirOpcode = 0x1117
```

## val MIR_FCMP

```mach
pub val MIR_FCMP: MirOpcode = 0x1118
```

## val MIR_FNEG

```mach
pub val MIR_FNEG: MirOpcode = 0x111B
```

## val MIR_MEMZERO

```mach
pub val MIR_MEMZERO: MirOpcode = 0x111C
```

## val MIR_MEMCPY

```mach
pub val MIR_MEMCPY: MirOpcode = 0x111D
```

copies the exact byte extent as a snapshot, including overlapping ranges

## val MIR_VALUE_CALL

```mach
pub val MIR_VALUE_CALL: MirOpcode = 0x111E
```

callee, explicit result destination, then ordered logical arguments

## val MIR_VALUE_PARAM

```mach
pub val MIR_VALUE_PARAM: MirOpcode = 0x111F
```

destination and logical parameter index, without physical carriers

## val MIR_OP_NONE

```mach
pub val MIR_OP_NONE: MirOpcode = 0xFFFF
```

## def MirFlags

```mach
pub def MirFlags: u32
```

## val MIRF_NONE

```mach
pub val MIRF_NONE: MirFlags = 0x00000000
```

## val MIRF_TERMINATOR

```mach
pub val MIRF_TERMINATOR:       MirFlags = 0x00000001
```

## val MIRF_COMPARE

```mach
pub val MIRF_COMPARE:          MirFlags = 0x00000002
```

## val MIRF_FUSED_CBR

```mach
pub val MIRF_FUSED_CBR:        MirFlags = 0x00000004
```

## val MIRF_SIGNED_CMP

```mach
pub val MIRF_SIGNED_CMP:       MirFlags = 0x00000008
```

## val MIRF_TWO_ADDRESS

```mach
pub val MIRF_TWO_ADDRESS:      MirFlags = 0x00000010
```

## val MIRF_CLONABLE

```mach
pub val MIRF_CLONABLE:         MirFlags = 0x00000020
```

## val MIRF_SHIFT

```mach
pub val MIRF_SHIFT:            MirFlags = 0x00000040
```

## val MIRF_DIVIDE

```mach
pub val MIRF_DIVIDE:           MirFlags = 0x00000080
```

## val MIRF_REMAINDER

```mach
pub val MIRF_REMAINDER:        MirFlags = 0x00000100
```

## val MIRF_DIV_SIGNED

```mach
pub val MIRF_DIV_SIGNED:       MirFlags = 0x00000200
```

## val MIRF_CMP_EQUALITY

```mach
pub val MIRF_CMP_EQUALITY:     MirFlags = 0x00000400
```

## val MIRF_CMP_SIGNED

```mach
pub val MIRF_CMP_SIGNED:       MirFlags = 0x00000800
```

## val MIRF_CMP_LE

```mach
pub val MIRF_CMP_LE:           MirFlags = 0x00001000
```

## val MIRF_INT_FP_CONV

```mach
pub val MIRF_INT_FP_CONV:      MirFlags = 0x00002000
```

## val MIRF_CONV_TO_FLOAT

```mach
pub val MIRF_CONV_TO_FLOAT:    MirFlags = 0x00004000
```

## val MIRF_CONV_FROM_FLOAT

```mach
pub val MIRF_CONV_FROM_FLOAT:  MirFlags = 0x00008000
```

## val MIRF_NO_SHAPE

```mach
pub val MIRF_NO_SHAPE:         MirFlags = 0x00010000
```

## val MIRF_MEMORY

```mach
pub val MIRF_MEMORY:           MirFlags = 0x00020000
```

## val MIRF_SEL_DIVIDE

```mach
pub val MIRF_SEL_DIVIDE:       MirFlags = 0x00040000
```

## val MIRF_SEL_SHIFT

```mach
pub val MIRF_SEL_SHIFT:        MirFlags = 0x00080000
```

## val MIRF_FLOAT_UNIT

```mach
pub val MIRF_FLOAT_UNIT:       MirFlags = 0x00100000
```

## val MIRF_ADDRESSED

```mach
pub val MIRF_ADDRESSED:        MirFlags = 0x00200000
```

## val MIRF_ADDRESS_PRODUCER

```mach
pub val MIRF_ADDRESS_PRODUCER: MirFlags = 0x00400000
```

## def MirCtClass

```mach
pub def MirCtClass: u8
```

## val MCT_NONE

```mach
pub val MCT_NONE:       MirCtClass = 0
```

## val MCT_INT_MUL

```mach
pub val MCT_INT_MUL:    MirCtClass = 1
```

## val MCT_INT_DIVMOD

```mach
pub val MCT_INT_DIVMOD: MirCtClass = 2
```

## val MCT_VAR_SHIFT

```mach
pub val MCT_VAR_SHIFT:  MirCtClass = 3
```

## val MCT_FLOAT

```mach
pub val MCT_FLOAT:      MirCtClass = 4
```

## def OperandBank

```mach
pub def OperandBank: u8
```

## val BANK_UNKNOWN

```mach
pub val BANK_UNKNOWN: OperandBank = 0
```

## val BANK_GP

```mach
pub val BANK_GP:      OperandBank = 1
```

## val BANK_FP

```mach
pub val BANK_FP:      OperandBank = 2
```

## val BANK_EITHER

```mach
pub val BANK_EITHER:  OperandBank = 3
```

## val BANK_NONE

```mach
pub val BANK_NONE:    OperandBank = 4
```

## fun validate_bank_pattern

```mach
pub fun validate_bank_pattern(pattern: str) err[fail.Fail];
```

g/f require a bank, v is gp scalar or fp vector, a is typed transport, n forbids registers
a final star repeats the preceding operand contract

## fun check_operand_banks

```mach
pub fun check_operand_banks() err[fail.Fail];
```

## fun operand_bank

```mach
pub fun operand_bank(op: MirOpcode, index: u32, vector: bool) res[OperandBank, fail.Fail];
```

## fun describe

```mach
pub fun describe(op: MirOpcode) res[*MirOpDescriptor, fail.Fail];
```

## fun desc

```mach
pub fun desc(op: MirOpcode) *MirOpDescriptor;
```

## fun has

```mach
pub fun has(op: MirOpcode, mask: MirFlags) bool;
```

## fun ct_class

```mach
pub fun ct_class(op: MirOpcode) MirCtClass;
```

## fun ct_op

```mach
pub fun ct_op(op: MirOpcode) res[ct.CtOp, fail.Fail];
```

## fun is_terminator

```mach
pub fun is_terminator(op: MirOpcode) bool;
```

## val SELECTION_REACHABLE_COUNT

```mach
pub val SELECTION_REACHABLE_COUNT:       u32 = 46
```

## val SELECTION_REACHABLE_FLOAT_COUNT

```mach
pub val SELECTION_REACHABLE_FLOAT_COUNT: u32 = 6
```

## val SELECTION_ELIMINATED_COUNT

```mach
pub val SELECTION_ELIMINATED_COUNT:      u32 = 5
```

## val SELECTION_FUSED_COUNT

```mach
pub val SELECTION_FUSED_COUNT:           u32 = 6
```

## val SELECTION_LANE_COUNT

```mach
pub val SELECTION_LANE_COUNT:            u32 = 2
```

## val SELECTION_WIDENING_MUL_COUNT

```mach
pub val SELECTION_WIDENING_MUL_COUNT:    u32 = 1
```

## fun selection_reachable

```mach
pub fun selection_reachable(out: *u32);
```

## fun selection_reachable_float

```mach
pub fun selection_reachable_float(out: *u32);
```

## fun selection_eliminated

```mach
pub fun selection_eliminated(out: *u32);
```

## fun selection_lane

```mach
pub fun selection_lane(out: *u32);
```

## fun selection_widening_mul

```mach
pub fun selection_widening_mul(out: *u32);
```

## fun selection_fused

```mach
pub fun selection_fused(out: *u32);
```

## val FCC_EQ

```mach
pub val FCC_EQ: u8 = 0
```

## val FCC_NE

```mach
pub val FCC_NE: u8 = 1
```

## val FCC_LT

```mach
pub val FCC_LT: u8 = 2
```

## val FCC_LE

```mach
pub val FCC_LE: u8 = 3
```

## val FCC_GT

```mach
pub val FCC_GT: u8 = 4
```

## val FCC_GE

```mach
pub val FCC_GE: u8 = 5
```

## rec VRegId

```mach
pub rec VRegId;
```

register identities are records, not `def` aliases: a vreg number cannot be
handed where a physical register is meant, and neither indexes a table
without naming the unwrap. a PRegId carries the class-tagged isa regid.

## rec PRegId

```mach
pub rec PRegId;
```

## val MIR_VREG_NIL

```mach
pub val MIR_VREG_NIL: VRegId = VRegId;
```

## val MIR_PREG_NIL

```mach
pub val MIR_PREG_NIL: PRegId = PRegId;
```

## fun vreg_id

```mach
pub fun vreg_id(n: u32) VRegId;
```

## fun vreg_is_nil

```mach
pub fun vreg_is_nil(v: VRegId) bool;
```

## fun vreg_same

```mach
pub fun vreg_same(a: VRegId, b: VRegId) bool;
```

## fun preg_id

```mach
pub fun preg_id(regid: i32) PRegId;
```

the one way an isa regid becomes a MIR physical register

## fun preg_regid

```mach
pub fun preg_regid(p: PRegId) i32;
```

## fun preg_is_nil

```mach
pub fun preg_is_nil(p: PRegId) bool;
```

## fun preg_same

```mach
pub fun preg_same(a: PRegId, b: PRegId) bool;
```

## tag MirIndex

```mach
pub tag MirIndex: u8 {
    none;
    vreg: VRegId;
    preg: PRegId;
}
```

the index register of a memory operand: absent, a vreg before allocation
or a preg after it. a case, not a flag beside a number

## fun index_none

```mach
pub fun index_none() MirIndex;
```

## fun index_vreg

```mach
pub fun index_vreg(v: VRegId) MirIndex;
```

## fun index_preg

```mach
pub fun index_preg(p: PRegId) MirIndex;
```

## val REG_CLASS_GP

```mach
pub val REG_CLASS_GP: u32 = 0
```

## val REG_CLASS_VALUE

```mach
pub val REG_CLASS_VALUE: u32 = 0xFFFFFFFF
```

every vreg of a values convention: never a bank index

## val ALU_MIN_WIDTH

```mach
pub val ALU_MIN_WIDTH: u8 = 4
```

## rec MirOperand

```mach
pub rec MirOperand;
```

## rec MirAsmBind

```mach
pub rec MirAsmBind;
```

## fun asm_bind

```mach
pub fun asm_bind(name: intern.StrId, slot_vreg: u32, secret: bool) MirAsmBind;
```

## rec MirAsm

```mach
pub rec MirAsm;
```

## val MEMORY_VOLATILE

```mach
pub val MEMORY_VOLATILE: u8 = 1
```

memory flags constrain the memory effects of an instruction

## rec MirInstr

```mach
pub rec MirInstr;
```

## fun instr

```mach
pub fun instr(opcode: MirOpcode, operands: *MirOperand, operand_count: u32,
width: u8, src_width: u8) MirInstr;
```

## fun declassify_marker

```mach
pub fun declassify_marker(src: *MirInstr, v: VRegId) MirInstr;
```

a dropped declassify self-copy becomes a zero-byte use that keeps the
barrier and names the vreg it downgraded; its carrier is read through
MirVReg.assigned and spill_slot

## val ISA_USE_OPCODE

```mach
pub val ISA_USE_OPCODE: MirOpcode = 0xFFFB
```

isa.ISA_USE, spelled here because mir sits below isa in the import graph

## fun instr_like

```mach
pub fun instr_like(src: *MirInstr, operands: *MirOperand, operand_count: u32) MirInstr;
```

## val VEC_LANE_NONE

```mach
pub val VEC_LANE_NONE:  u32 = 0
```

## val VEC_LANE_FLOAT

```mach
pub val VEC_LANE_FLOAT: u8  = 1
```

## val VEC_LANE_INT

```mach
pub val VEC_LANE_INT:   u8  = 2
```

## val VEC_LANE_MAX_LANES

```mach
pub val VEC_LANE_MAX_LANES: u32 = 0xFFFF
```

## fun vec_lane_make

```mach
pub fun vec_lane_make(kind: u8, elem_bytes: u8, lanes: u32) u32;
```

## fun vec_lane_kind

```mach
pub fun vec_lane_kind(v: u32) u8;
```

## fun vec_lane_elem_bytes

```mach
pub fun vec_lane_elem_bytes(v: u32) u8;
```

## fun vec_lane_is_vector

```mach
pub fun vec_lane_is_vector(v: u32) bool;
```

## fun vec_lane_count

```mach
pub fun vec_lane_count(v: u32) u32;
```

## rec MirDbgBinding

```mach
pub rec MirDbgBinding;
```

## rec MirBlock

```mach
pub rec MirBlock;
```

## rec MirVReg

```mach
pub rec MirVReg;
```

## val VREG_TY_NIL

```mach
pub val VREG_TY_NIL: u32 = 0xFFFFFFFF
```

## fun vreg

```mach
pub fun vreg(id: u32, class: u32, vector: bool, secret: bool) MirVReg;
```

## def MirSlotKind

```mach
pub def MirSlotKind: u8
```

## val SLOT_ALLOCA

```mach
pub val SLOT_ALLOCA: MirSlotKind = 0
```

## val SLOT_SPILL

```mach
pub val SLOT_SPILL:  MirSlotKind = 1
```

## rec MirSlot

```mach
pub rec MirSlot;
```

## val SLOT_TY_NIL

```mach
pub val SLOT_TY_NIL: u32 = 0xFFFFFFFF
```

## rec MirFrame

```mach
pub rec MirFrame;
```

## val ABI_INPUT_REGISTER

```mach
pub val ABI_INPUT_REGISTER:  u8  = 0
```

## val ABI_INPUT_ARG_STACK

```mach
pub val ABI_INPUT_ARG_STACK: u8  = 1
```

## val ABI_INPUT_SRET

```mach
pub val ABI_INPUT_SRET:      u32 = 0xFFFFFFFF
```

## val ABI_CONTENT_NONE

```mach
pub val ABI_CONTENT_NONE:    u8 = 0
```

## val ABI_CONTENT_UNKNOWN

```mach
pub val ABI_CONTENT_UNKNOWN: u8 = 1
```

## val ABI_CONTENT_PUBLIC

```mach
pub val ABI_CONTENT_PUBLIC:  u8 = 2
```

## val ABI_CONTENT_SECRET

```mach
pub val ABI_CONTENT_SECRET:  u8 = 3
```

## val ABI_CONTENT_OUTPUT

```mach
pub val ABI_CONTENT_OUTPUT:  u8 = 4
```

## rec MirAbiInput

```mach
pub rec MirAbiInput;
```

## rec MirFunction

```mach
pub rec MirFunction;
```

## fun opcode_name

```mach
pub fun opcode_name(op: MirOpcode) str;
```

## fun instr_refusal

```mach
pub fun instr_refusal(alloc: *A.Allocator, srcmap: *source.SourceMap,
fn_name: str, mi: *MirInstr, what: str) str;
```

## rec MirModule

```mach
pub rec MirModule;
```

## fun catalog_failure

```mach
pub fun catalog_failure(m: *MirModule, catalog: str, tag: u32) err[fail.Fail];
```

the module interner owns failure text independently of temporary backend storage

## fun validate_catalog

```mach
pub fun validate_catalog(m: *MirModule) err[fail.Fail];
```

## fun validate_operands

```mach
pub fun validate_operands(m: *MirModule) err[fail.Fail];
```

## val INITIAL_FN_CAP

```mach
pub val INITIAL_FN_CAP:    u32 = 8
```

## val INITIAL_VREG_CAP

```mach
pub val INITIAL_VREG_CAP:  u32 = 32
```

## val INITIAL_INSTR_CAP

```mach
pub val INITIAL_INSTR_CAP: u32 = 16
```

## val INITIAL_SLOT_CAP

```mach
pub val INITIAL_SLOT_CAP:  u32 = 8
```

## fun emits_instr

```mach
pub fun emits_instr(f: *MirFunction, mi: *MirInstr) bool;
```

## fun op_vreg

```mach
pub fun op_vreg(vreg: VRegId) MirOperand;
```

## fun op_preg

```mach
pub fun op_preg(preg: PRegId) MirOperand;
```

## fun op_preg_of

```mach
pub fun op_preg_of(preg: PRegId, origin: VRegId) MirOperand;
```

a physical operand that remembers the vreg it was rewritten from; the seed
secrecy of that vreg is what the post-allocation walk reads at this use

## fun operand_origin

```mach
pub fun operand_origin(op: *MirOperand) VRegId;
```

the vreg an operand carries the secrecy of after allocation: the origin of a
rewritten register operand, the slot key of a frame-slot memory operand,
MIR_VREG_NIL for a precolored register, a preg-based address, an immediate
or a symbol

## fun operand_seed_secret

```mach
pub fun operand_seed_secret(f: *MirFunction, op: *MirOperand) bool;
```

## fun op_imm

```mach
pub fun op_imm(imm: i64) MirOperand;
```

## fun op_mem_slot

```mach
pub fun op_mem_slot(slotkey: VRegId, disp: i64) MirOperand;
```

## fun op_mem_preg

```mach
pub fun op_mem_preg(preg: PRegId, disp: i64) MirOperand;
```

## fun op_mem_value

```mach
pub fun op_mem_value(base: VRegId, disp: i64) MirOperand;
```

## fun op_sym

```mach
pub fun op_sym(sym: intern.StrId, sym_off: i32) MirOperand;
```

## fun op_block

```mach
pub fun op_block(block: u32) MirOperand;
```

## fun vreg_is_fp

```mach
pub fun vreg_is_fp(fn: *MirFunction, vreg: VRegId) bool;
```

## fun push_function

```mach
pub fun push_function(mm: *MirModule, mf: MirFunction) err[fail.Fail];
```

## fun instr_attach_dbg

```mach
pub fun instr_attach_dbg(a: *A.Allocator, mi: *MirInstr, iid: u32, vreg: u32) err[fail.Fail];
```

## fun dnit_module

```mach
pub fun dnit_module(mm: *MirModule);
```

## fun dnit_function

```mach
pub fun dnit_function(a: *A.Allocator, mf: *MirFunction);
```

## fun dnit_block

```mach
pub fun dnit_block(a: *A.Allocator, mb: *MirBlock);
```

