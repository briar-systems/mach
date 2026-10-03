# mach.lang.target.isa.effect

the effect description of one emitted instruction, the contract every
instruction set encoder fills in: the constant-time class from the ct
catalog, the operand roles, the control transfer and its target, and the
implicit register and memory effects

## def OperandRole

```mach
pub def OperandRole: u8
```

the role an emitted instruction gives one of its operand positions
(isa_inst.Inst dst, src1, src2, src3), the part of the effect description that
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

## rec Hooks

```mach
pub rec Hooks;
```

the per-ISA half of the effect description, declared beside the encoder

## fun hooks_none

```mach
pub fun hooks_none() Hooks;
```

