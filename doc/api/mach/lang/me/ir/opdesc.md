# mach.lang.me.ir.opdesc

## def IrEffects

```mach
pub def IrEffects: u32
```

## val EFF_NONE

```mach
pub val EFF_NONE: IrEffects = 0x00000000
```

## val EFF_TERMINATOR

```mach
pub val EFF_TERMINATOR:     IrEffects = 0x00000001
```

## val EFF_HAS_RESULT

```mach
pub val EFF_HAS_RESULT:     IrEffects = 0x00000002
```

## val EFF_READS_MEMORY

```mach
pub val EFF_READS_MEMORY:   IrEffects = 0x00000004
```

## val EFF_WRITES_MEMORY

```mach
pub val EFF_WRITES_MEMORY:  IrEffects = 0x00000008
```

## val EFF_MAY_CALL

```mach
pub val EFF_MAY_CALL:       IrEffects = 0x00000010
```

## val EFF_MAY_TRAP

```mach
pub val EFF_MAY_TRAP:       IrEffects = 0x00000020
```

## val EFF_VOLATILE_OK

```mach
pub val EFF_VOLATILE_OK:    IrEffects = 0x00000040
```

## val EFF_ORDERED

```mach
pub val EFF_ORDERED:        IrEffects = 0x00000080
```

## val EFF_DISCARDABLE

```mach
pub val EFF_DISCARDABLE:    IrEffects = 0x00000100
```

## val EFF_SPECULATABLE

```mach
pub val EFF_SPECULATABLE:   IrEffects = 0x00000200
```

## val EFF_MOVABLE

```mach
pub val EFF_MOVABLE:        IrEffects = 0x00000400
```

## val EFF_CSE_SAFE

```mach
pub val EFF_CSE_SAFE:       IrEffects = 0x00000800
```

## val EFF_SECRET_MOVE

```mach
pub val EFF_SECRET_MOVE:    IrEffects = 0x00001000
```

## val EFF_OPERANDS_AGREE

```mach
pub val EFF_OPERANDS_AGREE: IrEffects = 0x00002000
```

## val EFF_COMPARE

```mach
pub val EFF_COMPARE:        IrEffects = 0x00004000
```

## val EFF_CMP_ORDERED

```mach
pub val EFF_CMP_ORDERED:    IrEffects = 0x00008000
```

## val EFF_FOLD_INT

```mach
pub val EFF_FOLD_INT:       IrEffects = 0x00010000
```

## val EFF_FOLD_FLOAT

```mach
pub val EFF_FOLD_FLOAT:     IrEffects = 0x00020000
```

## val EFF_REDUCE_INT

```mach
pub val EFF_REDUCE_INT:     IrEffects = 0x00040000
```

## val EFF_REDUCE_FLOAT

```mach
pub val EFF_REDUCE_FLOAT:   IrEffects = 0x00080000
```

## val EFF_VEC_SAMESHAPE

```mach
pub val EFF_VEC_SAMESHAPE:  IrEffects = 0x00100000
```

## val EFF_VEC_COMPUTE

```mach
pub val EFF_VEC_COMPUTE:    IrEffects = 0x00200000
```

## val EFF_INT_ALU

```mach
pub val EFF_INT_ALU:        IrEffects = 0x00400000
```

## val EFF_INT_CONVERSION

```mach
pub val EFF_INT_CONVERSION: IrEffects = 0x00800000
```

## val EFF_FP_CONVERT

```mach
pub val EFF_FP_CONVERT:     IrEffects = 0x01000000
```

## val EFF_CMP_UNSIGNED

```mach
pub val EFF_CMP_UNSIGNED:   IrEffects = 0x02000000
```

## val EFF_VEC_INCR_INT

```mach
pub val EFF_VEC_INCR_INT:   IrEffects = 0x04000000
```

## val EFF_VEC_INCR_FLOAT

```mach
pub val EFF_VEC_INCR_FLOAT: IrEffects = 0x08000000
```

## val EFF_USES_AUX_TYPE

```mach
pub val EFF_USES_AUX_TYPE:  IrEffects = 0x10000000
```

## val EFF_MAY_BE_ADDRESS

```mach
pub val EFF_MAY_BE_ADDRESS: IrEffects = 0x20000000
```

## def OperandRole

```mach
pub def OperandRole: u8
```

## val ROLE_NONE

```mach
pub val ROLE_NONE:      OperandRole = 0
```

## val ROLE_VALUE

```mach
pub val ROLE_VALUE:     OperandRole = 1
```

## val ROLE_ADDRESS

```mach
pub val ROLE_ADDRESS:   OperandRole = 2
```

## val ROLE_STORED

```mach
pub val ROLE_STORED:    OperandRole = 3
```

## val ROLE_SIZE

```mach
pub val ROLE_SIZE:      OperandRole = 4
```

## val ROLE_INDEX

```mach
pub val ROLE_INDEX:     OperandRole = 5
```

## val ROLE_LANE

```mach
pub val ROLE_LANE:      OperandRole = 6
```

## val ROLE_CONDITION

```mach
pub val ROLE_CONDITION: OperandRole = 7
```

## val ROLE_CFG_EDGE

```mach
pub val ROLE_CFG_EDGE:  OperandRole = 8
```

## val ROLE_PHI_EDGE

```mach
pub val ROLE_PHI_EDGE:  OperandRole = 9
```

## val ROLE_PHI_VALUE

```mach
pub val ROLE_PHI_VALUE: OperandRole = 10
```

## val ROLE_CALLEE

```mach
pub val ROLE_CALLEE:    OperandRole = 11
```

## val ROLE_CALL_ARG

```mach
pub val ROLE_CALL_ARG:  OperandRole = 12
```

## val ROLE_ASM_BIND

```mach
pub val ROLE_ASM_BIND:  OperandRole = 13
```

## val ROLE_DEBUG

```mach
pub val ROLE_DEBUG:     OperandRole = 14
```

## def ArityKind

```mach
pub def ArityKind: u8
```

## val ARITY_EXACT

```mach
pub val ARITY_EXACT:    ArityKind = 0
```

## val ARITY_AT_LEAST

```mach
pub val ARITY_AT_LEAST: ArityKind = 1
```

## val ARITY_AT_MOST

```mach
pub val ARITY_AT_MOST:  ArityKind = 2
```

## val ARITY_PAIRS

```mach
pub val ARITY_PAIRS:    ArityKind = 3
```

## val ARITY_ANY

```mach
pub val ARITY_ANY:      ArityKind = 4
```

## def ResultTyping

```mach
pub def ResultTyping: u8
```

## val RESULT_NONE

```mach
pub val RESULT_NONE:     ResultTyping = 0
```

## val RESULT_DECLARED

```mach
pub val RESULT_DECLARED: ResultTyping = 1
```

## val RESULT_MASK

```mach
pub val RESULT_MASK:     ResultTyping = 2
```

## val RESULT_ADDRESS

```mach
pub val RESULT_ADDRESS:  ResultTyping = 3
```

## def IrVecOp

```mach
pub def IrVecOp: u8
```

## val IVEC_NONE

```mach
pub val IVEC_NONE: IrVecOp = 0
```

## val IVEC_ADD

```mach
pub val IVEC_ADD:  IrVecOp = 1
```

## val IVEC_SUB

```mach
pub val IVEC_SUB:  IrVecOp = 2
```

## val IVEC_MUL

```mach
pub val IVEC_MUL:  IrVecOp = 3
```

## val IVEC_DIV

```mach
pub val IVEC_DIV:  IrVecOp = 4
```

## val IVEC_AND

```mach
pub val IVEC_AND:  IrVecOp = 5
```

## val IVEC_OR

```mach
pub val IVEC_OR:   IrVecOp = 6
```

## val IVEC_XOR

```mach
pub val IVEC_XOR:  IrVecOp = 7
```

## val IVEC_NOT

```mach
pub val IVEC_NOT:  IrVecOp = 8
```

## val IVEC_NEG

```mach
pub val IVEC_NEG:  IrVecOp = 9
```

## val IVEC_SHL

```mach
pub val IVEC_SHL:  IrVecOp = 10
```

## val IVEC_SHR

```mach
pub val IVEC_SHR:  IrVecOp = 11
```

## val IVEC_CMP

```mach
pub val IVEC_CMP:  IrVecOp = 12
```

## def VecClass

```mach
pub def VecClass: u8
```

## val VCLASS_NONE

```mach
pub val VCLASS_NONE:       VecClass = 0
```

## val VCLASS_BINARY

```mach
pub val VCLASS_BINARY:     VecClass = 1
```

## val VCLASS_UNARY

```mach
pub val VCLASS_UNARY:      VecClass = 2
```

## val VCLASS_COMPARE

```mach
pub val VCLASS_COMPARE:    VecClass = 3
```

## val VCLASS_DISALLOWED

```mach
pub val VCLASS_DISALLOWED: VecClass = 4
```

## def CtClass

```mach
pub def CtClass: u8
```

## val CTC_NONE

```mach
pub val CTC_NONE:      CtClass = 0
```

## val CTC_INT_MUL

```mach
pub val CTC_INT_MUL:   CtClass = 1
```

## val CTC_VAR_SHIFT

```mach
pub val CTC_VAR_SHIFT: CtClass = 2
```

## val CTC_FLOAT

```mach
pub val CTC_FLOAT:     CtClass = 3
```

## def LowerRoute

```mach
pub def LowerRoute: u8
```

## val LOWER_GENERIC

```mach
pub val LOWER_GENERIC:     LowerRoute = 0
```

## val LOWER_CALL

```mach
pub val LOWER_CALL:        LowerRoute = 1
```

## val LOWER_RET

```mach
pub val LOWER_RET:         LowerRoute = 2
```

## val LOWER_PHI

```mach
pub val LOWER_PHI:         LowerRoute = 3
```

## val LOWER_DBG

```mach
pub val LOWER_DBG:         LowerRoute = 4
```

## val LOWER_ALLOCA

```mach
pub val LOWER_ALLOCA:      LowerRoute = 5
```

## val LOWER_LOAD

```mach
pub val LOWER_LOAD:        LowerRoute = 6
```

## val LOWER_STORE

```mach
pub val LOWER_STORE:       LowerRoute = 7
```

## val LOWER_MEMZERO

```mach
pub val LOWER_MEMZERO:     LowerRoute = 8
```

## val LOWER_GEP

```mach
pub val LOWER_GEP:         LowerRoute = 9
```

## val LOWER_VEC_EXTRACT

```mach
pub val LOWER_VEC_EXTRACT: LowerRoute = 10
```

## val LOWER_VEC_INSERT

```mach
pub val LOWER_VEC_INSERT:  LowerRoute = 11
```

## val LOWER_VEC_BUILD

```mach
pub val LOWER_VEC_BUILD:   LowerRoute = 12
```

## val LOWER_DECLASSIFY

```mach
pub val LOWER_DECLASSIFY:  LowerRoute = 13
```

## val LOWER_NEG

```mach
pub val LOWER_NEG:         LowerRoute = 14
```

## val LOWER_BITCAST

```mach
pub val LOWER_BITCAST:     LowerRoute = 15
```

## val LOWER_DIV

```mach
pub val LOWER_DIV:         LowerRoute = 16
```

## val LOWER_SHIFT

```mach
pub val LOWER_SHIFT:       LowerRoute = 17
```

## val LOWER_ASM

```mach
pub val LOWER_ASM:         LowerRoute = 18
```

## val LOWER_CBR

```mach
pub val LOWER_CBR:         LowerRoute = 19
```

## val OP_MAX_FIXED_OPERANDS

```mach
pub val OP_MAX_FIXED_OPERANDS: usize = 3
```

## rec IrOpDescriptor

```mach
pub rec IrOpDescriptor;
```

## val IR_OP_DESCRIPTOR_COUNT

```mach
pub val IR_OP_DESCRIPTOR_COUNT: usize = 50
```

## fun is_known

```mach
pub fun is_known(k: instruction.InstrKind) bool;
```

## fun describe

```mach
pub fun describe(k: instruction.InstrKind) res[*IrOpDescriptor, fail.Fail];
```

## fun unknown_kind

```mach
pub fun unknown_kind(a: *A.Allocator, k: instruction.InstrKind, where: str) fail.Fail;
```

the middle end's catalog adapter: an opcode a pass dispatch reaches that no
arm names is an internal failure naming the catalog, the tag and the pass.
the text lives in the module allocator, which outlives the pass

## fun desc

```mach
pub fun desc(k: instruction.InstrKind) *IrOpDescriptor;
```

## fun effects

```mach
pub fun effects(k: instruction.InstrKind) IrEffects;
```

## fun has

```mach
pub fun has(k: instruction.InstrKind, mask: IrEffects) bool;
```

## fun name

```mach
pub fun name(k: instruction.InstrKind) str;
```

## fun class_name

```mach
pub fun class_name(k: instruction.InstrKind) str;
```

## fun vec_op

```mach
pub fun vec_op(k: instruction.InstrKind) IrVecOp;
```

## fun vec_class

```mach
pub fun vec_class(k: instruction.InstrKind) VecClass;
```

## fun ct_class

```mach
pub fun ct_class(k: instruction.InstrKind) CtClass;
```

## fun lower_route

```mach
pub fun lower_route(k: instruction.InstrKind) LowerRoute;
```

## fun result_typing

```mach
pub fun result_typing(k: instruction.InstrKind) ResultTyping;
```

## fun operand_role

```mach
pub fun operand_role(k: instruction.InstrKind, index: u32) OperandRole;
```

## fun is_terminator

```mach
pub fun is_terminator(k: instruction.InstrKind) bool;
```

## fun may_trap

```mach
pub fun may_trap(k: instruction.InstrKind) bool;
```

## fun transfers_secrecy

```mach
pub fun transfers_secrecy(k: instruction.InstrKind) bool;
```

## fun produces_value

```mach
pub fun produces_value(k: instruction.InstrKind) bool;
```

## fun accepts_volatile

```mach
pub fun accepts_volatile(k: instruction.InstrKind) bool;
```

## fun arity_ok

```mach
pub fun arity_ok(k: instruction.InstrKind, operand_count: u32) bool;
```

