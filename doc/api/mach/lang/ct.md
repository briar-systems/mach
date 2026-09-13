# mach.lang.ct

## def CtOp

```mach
pub def CtOp: u8
```

## val CT_OP_NONE

```mach
pub val CT_OP_NONE:       CtOp = 0
```

## val CT_OP_INT_DIVMOD

```mach
pub val CT_OP_INT_DIVMOD: CtOp = 1
```

## val CT_OP_FLOAT

```mach
pub val CT_OP_FLOAT:      CtOp = 2
```

## val CT_OP_INT_MUL

```mach
pub val CT_OP_INT_MUL:    CtOp = 3
```

## val CT_OP_VAR_SHIFT

```mach
pub val CT_OP_VAR_SHIFT:  CtOp = 4
```

## def CtCap

```mach
pub def CtCap: u8
```

## val CT_CAP_OK

```mach
pub val CT_CAP_OK:        CtCap = 0
```

## val CT_CAP_ALWAYS

```mach
pub val CT_CAP_ALWAYS:    CtCap = 1
```

## val CT_CAP_MUL

```mach
pub val CT_CAP_MUL:       CtCap = 2
```

## val CT_CAP_VAR_SHIFT

```mach
pub val CT_CAP_VAR_SHIFT: CtCap = 3
```

## val CT_OP_COUNT

```mach
pub val CT_OP_COUNT: u32 = 5
```

## fun ct_op_valid

```mach
pub fun ct_op_valid(op: CtOp) bool;
```

## fun ct_cap

```mach
pub fun ct_cap(op: CtOp) opt[CtCap];
```

the capability a target must provide for an operation to be constant-time,
absent for an operation outside the catalog: an unknown operation is never
answered with "no capability needed"

## fun ct_cap_or_always

```mach
pub fun ct_cap_or_always(op: CtOp) CtCap;
```

tests and the sema gate: the declared capability, or the one no target
provides for an operation outside the catalog

## fun cap_at_sema

```mach
pub fun cap_at_sema(cap: CtCap) bool;
```

## fun cap_at_mir

```mach
pub fun cap_at_mir(cap: CtCap) bool;
```

## fun target_provides

```mach
pub fun target_provides(trust_mul: bool, trust_var_shift: bool, cap: CtCap) bool;
```

whether a target's declared trust covers a capability; false for every
other capability, which no target provides (a partition, enumerated by its
test)

## fun ct_op_gates_count_only

```mach
pub fun ct_op_gates_count_only(op: CtOp) bool;
```

## rec AsmClass

```mach
pub rec AsmClass;
```

## fun asm_unknown

```mach
pub fun asm_unknown() AsmClass;
```

## fun flags_untouched

```mach
pub fun flags_untouched(a: AsmClass) AsmClass;
```

## fun flags_defined

```mach
pub fun flags_defined(a: AsmClass) AsmClass;
```

## fun flags_written

```mach
pub fun flags_written(a: AsmClass) AsmClass;
```

## fun flags_opaque

```mach
pub fun flags_opaque(a: AsmClass) AsmClass;
```

## fun flags_read

```mach
pub fun flags_read(a: AsmClass) AsmClass;
```

## fun asm_op

```mach
pub fun asm_op(op: CtOp) AsmClass;
```

## fun asm_branch_flags

```mach
pub fun asm_branch_flags() AsmClass;
```

## fun asm_branch_reg

```mach
pub fun asm_branch_reg() AsmClass;
```

## fun untouched_branch_reg

```mach
pub fun untouched_branch_reg() AsmClass;
```

## def OperandRole

```mach
pub def OperandRole: u8
```

the role an emitted instruction gives one of its operand positions
(isa.Inst dst, src1, src2, src3), the part of the effect description that
ct_class does not carry: which registers a notification defines and which
it only reads, and whether a memory operand is loaded, stored, or only its
address computed

## val ROLE_NONE

```mach
pub val ROLE_NONE:  OperandRole = 0
```

## val ROLE_READ

```mach
pub val ROLE_READ:  OperandRole = 1
```

## val ROLE_WRITE

```mach
pub val ROLE_WRITE: OperandRole = 2
```

## val ROLE_RW

```mach
pub val ROLE_RW:    OperandRole = 3
```

## val ROLE_ADDR

```mach
pub val ROLE_ADDR: OperandRole = 4
```

the address registers are read as values and no memory is accessed (lea)

## val INST_OPERANDS

```mach
pub val INST_OPERANDS: u32 = 4
```

## def Transfer

```mach
pub def Transfer: u8
```

## val XFER_NONE

```mach
pub val XFER_NONE: Transfer = 0
```

## val XFER_BRANCH

```mach
pub val XFER_BRANCH: Transfer = 1
```

conditional: may fall through

## val XFER_JUMP

```mach
pub val XFER_JUMP: Transfer = 2
```

unconditional: never falls through

## val XFER_CALL

```mach
pub val XFER_CALL: Transfer = 3
```

falls through after the callee returns

## val XFER_RET

```mach
pub val XFER_RET:  Transfer = 4
```

## val XFER_TRAP

```mach
pub val XFER_TRAP: Transfer = 5
```

## def TargetKind

```mach
pub def TargetKind: u8
```

## val TARGET_NONE

```mach
pub val TARGET_NONE: TargetKind = 0
```

## val TARGET_BLOCK

```mach
pub val TARGET_BLOCK: TargetKind = 1
```

a basic block of the same function

## val TARGET_LOCAL

```mach
pub val TARGET_LOCAL:    TargetKind = 2
```

an offset inside the same expansion or inline-asm body, backward or of
unknown direction

## val TARGET_EXTERNAL

```mach
pub val TARGET_EXTERNAL: TargetKind = 3
```

## val TARGET_REGISTER

```mach
pub val TARGET_REGISTER: TargetKind = 4
```

through a register operand

## val TARGET_LOCAL_FWD

```mach
pub val TARGET_LOCAL_FWD: TargetKind = 5
```

an offset inside the same expansion, known to lie ahead

## rec InstEffects

```mach
pub rec InstEffects;
```

the complete effect description of one emitted instruction: the closed
class table's row plus the operand roles and the implicit register and
memory effects. masks are over general-purpose register indexes

## fun inst_effects_unknown

```mach
pub fun inst_effects_unknown() InstEffects;
```

## fun reg_bit

```mach
pub fun reg_bit(index: i32) u64;
```

