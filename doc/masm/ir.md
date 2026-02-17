# Portable IR Opcodes

This document provides a complete reference for MASM's portable intermediate representation (IR) opcodes.

## Overview

MASM IR is a platform-independent instruction set using three-operand form (TOF). Key characteristics:

- **Flag-Free** - Comparisons produce values (0 or 1), not implicit condition flags
- **Three-Operand** - Avoids destructive two-operand semantics (`dst = src1 op src2`)
- **RISC-like** - Maps cleanly to RISC architectures; x86 lowering handles the differences
- **Typed** - Instructions carry type information for correct code generation

IR opcodes are defined in `src/compiler/masm/ir.mach`.


## Opcode Categories

- [Data Movement](#data-movement)
- [Integer Arithmetic](#integer-arithmetic)
- [Bitwise Operations](#bitwise-operations)
- [Comparisons](#comparisons)
- [Floating Point](#floating-point)
- [Control Flow](#control-flow)
- [System](#system)
- [Pseudo-Ops](#pseudo-ops)


## Data Movement

### `mov`

Copy a value from source to destination.

**Syntax**: `mov dst, src`

**Example**:
```
mov %1, %0       ; copy register
mov %2, 42       ; load immediate
mov %3, @global  ; load global address
```

### `load`

Load a value from memory.

**Syntax**: `load dst, [addr]`

**Notes**: Type on instruction determines load size and extension behavior.

**Example**:
```
load %val, [%ptr]    ; load from pointer
```

### `store`

Store a value to memory.

**Syntax**: `store [addr], src`

**Example**:
```
store [%ptr], %val   ; store to pointer
```

### `lea`

Load effective address (compute address without dereferencing).

**Syntax**: `lea dst, addr`

**Example**:
```
lea %addr, @string_literal
```

### `zext`

Zero-extend a smaller integer to a larger integer.

**Syntax**: `zext dst, src`

**Example**:
```
zext %wide, %byte    ; u8 -> u64
```

### `sext`

Sign-extend a smaller signed integer to a larger signed integer.

**Syntax**: `sext dst, src`

**Example**:
```
sext %wide, %short   ; i16 -> i64
```


## Integer Arithmetic

### `add`

Add two values.

**Syntax**: `add dst, a, b`

**Result**: `dst = a + b`

### `sub`

Subtract second value from first.

**Syntax**: `sub dst, a, b`

**Result**: `dst = a - b`

### `mul`

Multiply two values.

**Syntax**: `mul dst, a, b`

**Result**: `dst = a * b` (lower bits)

### `div`

Signed division.

**Syntax**: `div dst, a, b`

**Result**: `dst = a / b` (signed)

### `divu`

Unsigned division.

**Syntax**: `divu dst, a, b`

**Result**: `dst = a / b` (unsigned)

### `rem`

Signed remainder.

**Syntax**: `rem dst, a, b`

**Result**: `dst = a % b` (signed)

### `remu`

Unsigned remainder.

**Syntax**: `remu dst, a, b`

**Result**: `dst = a % b` (unsigned)

### `neg`

Two's complement negation.

**Syntax**: `neg dst, src`

**Result**: `dst = -src`


## Bitwise Operations

### `and`

Bitwise AND.

**Syntax**: `and dst, a, b`

**Result**: `dst = a & b`

### `or`

Bitwise OR.

**Syntax**: `or dst, a, b`

**Result**: `dst = a | b`

### `xor`

Bitwise XOR.

**Syntax**: `xor dst, a, b`

**Result**: `dst = a ^ b`

### `not`

Bitwise complement.

**Syntax**: `not dst, src`

**Result**: `dst = ~src`

### `shl`

Logical left shift.

**Syntax**: `shl dst, src, count`

**Result**: `dst = src << count`

### `shr`

Logical right shift (zero fill).

**Syntax**: `shr dst, src, count`

**Result**: `dst = src >> count` (unsigned)

### `sar`

Arithmetic right shift (sign fill).

**Syntax**: `sar dst, src, count`

**Result**: `dst = src >> count` (signed)


## Comparisons

All comparison operations produce 1 (true) or 0 (false) in the destination register. MASM IR does **not** use implicit condition flags.

### Signed Comparisons

| Opcode | Syntax | Result |
|--------|--------|--------|
| `seq` | `seq dst, a, b` | `dst = (a == b)` |
| `sne` | `sne dst, a, b` | `dst = (a != b)` |
| `slt` | `slt dst, a, b` | `dst = (a < b)` signed |
| `sle` | `sle dst, a, b` | `dst = (a <= b)` signed |
| `sgt` | `sgt dst, a, b` | `dst = (a > b)` signed |
| `sge` | `sge dst, a, b` | `dst = (a >= b)` signed |

### Unsigned Comparisons

| Opcode | Syntax | Result |
|--------|--------|--------|
| `sltu` | `sltu dst, a, b` | `dst = (a < b)` unsigned |
| `sleu` | `sleu dst, a, b` | `dst = (a <= b)` unsigned |
| `sgtu` | `sgtu dst, a, b` | `dst = (a > b)` unsigned |
| `sgeu` | `sgeu dst, a, b` | `dst = (a >= b)` unsigned |


## Floating Point

### Arithmetic

| Opcode | Syntax | Result |
|--------|--------|--------|
| `fadd` | `fadd dst, a, b` | `dst = a + b` |
| `fsub` | `fsub dst, a, b` | `dst = a - b` |
| `fmul` | `fmul dst, a, b` | `dst = a * b` |
| `fdiv` | `fdiv dst, a, b` | `dst = a / b` |

### Comparison

**Syntax**: `fcmp dst, a, b, cond`

**Condition codes** (`MasmIrFcmpCond`):
- `eq` - Equal
- `ne` - Not equal
- `lt` - Less than
- `le` - Less or equal
- `gt` - Greater than
- `ge` - Greater or equal

**Result**: 1 if condition true, 0 otherwise

### Conversion

**Syntax**: `fconv dst, src, mode`

**Conversion modes**:
- `i2f` - Integer to float
- `f2i` - Float to integer (truncating)
- `f32_f64` - Widen f32 to f64
- `f64_f32` - Narrow f64 to f32


## Control Flow

### `jmp`

Unconditional jump.

**Syntax**: `jmp label`

**Example**:
```
jmp .loop_start
```

### Conditional Branches

MASM uses RISC-style compare-and-branch instructions that compare two operands directly:

| Opcode | Syntax | Condition |
|--------|--------|-----------|
| `beq` | `beq a, b, label` | Branch if `a == b` |
| `bne` | `bne a, b, label` | Branch if `a != b` |
| `blt` | `blt a, b, label` | Branch if `a < b` (signed) |
| `bltu` | `bltu a, b, label` | Branch if `a < b` (unsigned) |
| `bge` | `bge a, b, label` | Branch if `a >= b` (signed) |
| `bgeu` | `bgeu a, b, label` | Branch if `a >= b` (unsigned) |

**Example**:
```
blt %i, %n, .loop_body    ; if i < n, goto loop_body
jmp .loop_end
```

### `call`

Call a function.

**Syntax**: `call target, [args...]`

**Example**:
```
call @add, %a, %b
```

### `ret`

Return from function.

**Syntax**: `ret [value]`

**Example**:
```
ret %result    ; return with value
ret            ; return void
```


## System

### `syscall`

Execute a system call.

**Syntax**: `syscall dst, num, [args...]`

**Notes**: Up to 6 arguments. Backend handles ABI-specific register placement.

**Example**:
```
syscall %result, 1, %fd, %buf, %len   ; write()
syscall %_, 60, %code                  ; exit()
```


## Pseudo-Ops

### `label`

Define a code label.

**Syntax**: `label name:`

**Example**:
```
label .loop_start:
```

### `data`

Emit static data.

**Syntax**: `data value, size`

**Example**:
```
data "Hello, World!", 13
```

### `stack_frame`

Emit function prologue/epilogue with given local variable space.

**Syntax**: `stack_frame size`

**Notes**: This is a high-level pseudo-op that expands to target-specific prologue code.

**Example**:
```
stack_frame 32    ; allocate 32 bytes for locals
```


## Instruction Structure

Each IR instruction is represented by:

```c
typedef struct MasmInstruction {
    MasmOpcodeKind kind;          // MASM_OPCODE_IR
    uint32_t       opcode;        // MasmIrOpcode value
    MasmOperand   *operands;
    uint8_t        operand_count;
} MasmInstruction;
```

### Instruction Builders

Convenience functions for creating IR instructions:

```c
MasmInstruction masm_inst_0(uint32_t opcode);
MasmInstruction masm_inst_1(uint32_t opcode, MasmOperand op1);
MasmInstruction masm_inst_2(uint32_t opcode, MasmOperand op1, MasmOperand op2);
MasmInstruction masm_inst_3(uint32_t opcode, MasmOperand op1, MasmOperand op2, MasmOperand op3);
MasmInstruction masm_inst_4(uint32_t opcode, MasmOperand op1, MasmOperand op2, MasmOperand op3, MasmOperand op4);
```


## Lowering Examples

### Simple Comparison (`if a < b`)

**MASM IR**:
```
blt %a, %b, .then_block
```

**x86_64 Lowering**:
```asm
cmp %a, %b
jl .then_block
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
syscall %res, 1, 1, %msg, %len
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