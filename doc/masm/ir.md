# Portable IR Opcodes

This document provides a complete reference for MASM's portable intermediate representation (IR) opcodes.

## Overview

MASM IR is a platform-independent instruction set using fixed three-operand form. Key characteristics:

- **Flag-Free** - Comparisons produce values (0 or 1), not implicit condition flags
- **Fixed Three-Operand** - Every instruction has exactly `dst`, `src1`, `src2` fields (unused fields are zeroed)
- **Condition Code Field** - Comparisons and conditional branches use a `cc` field, not separate opcodes per condition
- **RISC-like** - Maps cleanly to RISC architectures; x86 lowering handles the differences
- **Typed** - Instructions carry a `size` field for correct code generation

IR opcodes are defined in `src/compiler/masm/ir.mach`.


## Opcode Categories

- [Data Movement](#data-movement)
- [Integer Arithmetic](#integer-arithmetic)
- [Bitwise Operations](#bitwise-operations)
- [Comparisons](#comparisons)
- [Conversions](#conversions)
- [Floating Point](#floating-point)
- [Control Flow](#control-flow)
- [System](#system)
- [Pseudo-Ops](#pseudo-ops)
- [Internal](#internal)


## Data Movement

### `mov` (OP_MOV = 0)

Copy a value from source to destination.

**Operands**: `dst`, `src1`

**Example**:
```
mov %1, %0       ; copy register
mov %2, 42       ; load immediate
```

### `load` (OP_LOAD = 1)

Load a value from memory.

**Operands**: `dst`, `src1` (memory operand)

**Notes**: The instruction's `size` field determines load width.

**Example**:
```
load %val, [FP - 8]    ; load from stack slot
```

### `store` (OP_STORE = 2)

Store a value to memory.

**Operands**: `dst` (memory operand), `src1` (value)

**Example**:
```
store [FP - 8], %val   ; store to stack slot
```

### `addr` (OP_ADDR = 3)

Compute effective address (no memory access).

**Operands**: `dst`, `src1` (address source)

**Example**:
```
addr %ptr, @string_literal
```


## Integer Arithmetic

All arithmetic operations use three-operand form: `dst = src1 op src2`.

### `add` (OP_ADD = 4)

**Result**: `dst = src1 + src2`

### `sub` (OP_SUB = 5)

**Result**: `dst = src1 - src2`

### `mul` (OP_MUL = 6)

**Result**: `dst = src1 * src2` (lower bits)

### `div` (OP_DIV = 7)

Signed division.

**Result**: `dst = src1 / src2` (signed)

### `mod` (OP_MOD = 8)

Signed modulo.

**Result**: `dst = src1 % src2` (signed)

### `divu` (OP_DIVU = 9)

Unsigned division.

**Result**: `dst = src1 / src2` (unsigned)

### `modu` (OP_MODU = 10)

Unsigned modulo.

**Result**: `dst = src1 % src2` (unsigned)


## Bitwise Operations

### `and` (OP_AND = 11)

**Result**: `dst = src1 & src2`

### `or` (OP_OR = 12)

**Result**: `dst = src1 | src2`

### `xor` (OP_XOR = 13)

**Result**: `dst = src1 ^ src2`

### `shl` (OP_SHL = 14)

Logical left shift.

**Result**: `dst = src1 << src2`

### `shr` (OP_SHR = 15)

Logical right shift (zero fill).

**Result**: `dst = src1 >> src2` (unsigned)

### `sar` (OP_SAR = 16)

Arithmetic right shift (sign fill).

**Result**: `dst = src1 >> src2` (signed)


## Comparisons

MASM IR uses two parameterized comparison opcodes (`icmp` and `fcmp`) with a condition code field, rather than separate opcodes per comparison kind. All comparisons produce 1 (true) or 0 (false) in the destination.

### `icmp` (OP_ICMP = 17)

Integer comparison. The `cc` field selects the comparison operation.

**Operands**: `dst`, `src1`, `src2`, `cc`

**Result**: `dst = (src1 CC src2) ? 1 : 0`

### `fcmp` (OP_FCMP = 18)

Floating-point comparison. The `cc` field selects the comparison operation.

**Operands**: `dst`, `src1`, `src2`, `cc`

**Result**: `dst = (src1 CC src2) ? 1 : 0`

### Condition Codes

| Code | Value | Description |
|------|-------|-------------|
| `CC_EQ` | 1 | Equal |
| `CC_NE` | 2 | Not equal |
| `CC_LT` | 3 | Signed less than |
| `CC_LE` | 4 | Signed less or equal |
| `CC_GT` | 5 | Signed greater than |
| `CC_GE` | 6 | Signed greater or equal |
| `CC_B` | 7 | Unsigned below |
| `CC_BE` | 8 | Unsigned below or equal |
| `CC_A` | 9 | Unsigned above |
| `CC_AE` | 10 | Unsigned above or equal |


## Conversions

### `zext` (OP_ZEXT = 23)

Zero-extend a smaller integer to a larger integer.

**Operands**: `dst`, `src1`

**Example**:
```
zext %wide, %byte    ; u8 -> u64
```

### `sext` (OP_SEXT = 24)

Sign-extend a smaller signed integer to a larger signed integer.

**Operands**: `dst`, `src1`

**Example**:
```
sext %wide, %short   ; i16 -> i64
```

### `trunc` (OP_TRUNC = 25)

Truncate a larger integer to a smaller integer.

**Operands**: `dst`, `src1`

### `itof` (OP_ITOF = 26)

Convert integer to float.

**Operands**: `dst`, `src1`

### `ftoi` (OP_FTOI = 27)

Convert float to integer (truncating).

**Operands**: `dst`, `src1`

### `fext` (OP_FEXT = 28)

Widen f32 to f64.

**Operands**: `dst`, `src1`

### `ftrunc` (OP_FTRUNC = 29)

Narrow f64 to f32.

**Operands**: `dst`, `src1`


## Floating Point

### Arithmetic

| Opcode | Value | Result |
|--------|-------|--------|
| `fadd` | 30 | `dst = src1 + src2` |
| `fsub` | 31 | `dst = src1 - src2` |
| `fmul` | 32 | `dst = src1 * src2` |
| `fdiv` | 33 | `dst = src1 / src2` |

### Comparison

Float comparisons use `fcmp` (OP_FCMP = 18) with the same condition codes as `icmp`. See [Comparisons](#comparisons).


## Control Flow

### `jmp` (OP_JMP = 19)

Unconditional jump.

**Operands**: `dst` (label operand)

**Example**:
```
jmp .loop_start
```

### `brcond` (OP_BRCOND = 20)

Conditional branch. Branches to the label in `dst` if `src1` is nonzero.

**Operands**: `dst` (label), `src1` (condition value)

**Notes**: Typically preceded by an `icmp` or `fcmp` that produces the condition value. The `cc` field is not used by `brcond` itself -- the condition is already materialized as a 0/1 value in `src1`.

**Example**:
```
icmp %cond, %i, %n, CC_LT    ; %cond = (i < n) ? 1 : 0
brcond .loop_body, %cond      ; branch if %cond != 0
```

### `call` (OP_CALL = 21)

Call a function.

**Operands**: `dst` (symbol operand for target function)

**Notes**: Arguments are placed in ABI-designated registers before the call.

### `ret` (OP_RET = 22)

Return from function.

**Operands**: `src1` (return value, if any)


## System

### `syscall` (OP_SYSCALL = 35)

Execute a system call.

**Operands**: `dst` (result), `src1` (syscall number)

**Notes**: Up to 6 arguments are placed in ABI-designated registers. Backend handles register placement.

**Example**:
```
syscall %result, 1, ...   ; SYS_write
syscall %_, 60, ...       ; SYS_exit
```


## Pseudo-Ops

### `label` (OP_LABEL = 34)

Define a code label (basic block entry point).

**Operands**: `dst` (label operand with name)

**Example**:
```
label .loop_start
```


## Internal

These opcodes are used internally by the peephole optimizer and instruction selector. They are never emitted by the lowering phase.

### `nop` (OP_NOP = 36)

Tombstone for peephole elimination. Marks an instruction as deleted without removing it from the instruction array.

### `memset` (OP_MEMSET = 37)

Peephole optimization that replaces loop-based memory initialization with a memset operation.


## Instruction Structure

Each IR instruction is a fixed-size `Inst` record:

```mach
pub rec Inst {
    op:       Operator;    # the operation (OP_ADD, OP_ICMP, etc.)
    dst:      Operand;     # destination operand
    src1:     Operand;     # first source operand
    src2:     Operand;     # second source operand
    cc:       Cond;        # condition code (for OP_ICMP, OP_FCMP, OP_BRCOND)
    size:     u8;          # operation size in bytes
    src_line: i32;         # source line number for debug info
    src_file: u16;         # source file index for debug info
}
```

All instructions have exactly three operand slots (`dst`, `src1`, `src2`). Unused slots are zeroed. This fixed layout eliminates variable-length instruction handling.

### Operand Constructors

```mach
ir.make_none()                                  # empty operand
ir.make_reg(reg: i32, size: u8)                 # register operand
ir.make_imm(imm: i64, size: u8)                # immediate constant
ir.make_mem(base, index, scale, disp, size)     # memory reference
ir.make_label(label: str)                       # label reference
ir.make_sym(sym_id: u64)                        # symbol reference
```

### Block Append

Instructions are appended to basic blocks:

```mach
ir.block_append(alloc, block, op, dst, src1, src2, cc)
```


## Lowering Examples

### Comparison and Branch (`if a < b`)

**MASM IR**:
```
icmp %cond, %a, %b, CC_LT    ; %cond = (a < b) ? 1 : 0
brcond .then_block, %cond     ; branch if true
jmp .else_block               ; fallthrough to else
```

**x86_64 Lowering**:
```asm
cmp %a, %b
jl .then_block
jmp .else_block
```

### Arithmetic (`dst = a + b`)

**MASM IR**:
```
add %dst, %a, %b
```

**x86_64 Lowering**:
```asm
mov %dst, %a
add %dst, %b
```

### Syscall (`write(1, msg, len)`)

**MASM IR**:
```
syscall %res, 1, ...    ; args in ABI registers
```

**Linux x86_64 Lowering**:
```asm
mov rax, 1      ; SYS_write
mov rdi, 1      ; fd = stdout
mov rsi, %msg   ; buffer
mov rdx, %len   ; count
syscall
mov %res, rax
```


## See Also

- [Overview](overview.md) - MASM architecture
- [Operands](operands.md) - Operand types
- [Code Generation](codegen.md) - How IR maps to machine code
