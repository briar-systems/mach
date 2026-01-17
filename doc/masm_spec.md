# MASM Intermediate Representation Specification

## Overview

MASM (Mach Assembly) is a portable, low-level intermediate representation designed to sit between the Mach compiler frontend and platform-specific backends. It aims to be:

1.  **Portable**: Abstract enough to map cleanly to x86-64, ARM64, RISC-V, and WebAssembly.
2.  **Readable & Writable**: Human-friendly syntax familiar to assembly programmers.
3.  **Minimal**: A reduced instruction set that avoids ISA-specific quirks (like implicit condition flags).

## Virtual Machine Model

### Registers
MASM operates on an infinite set of **virtual registers** (vregs).
- Registers are typed by size (u8, u16, u32, u64).
- Syntax: `%name` (e.g., `%tmp0`, `%result`, `%loop_idx`).
- Physical register allocation happens during the backend lowering phase.

### Memory
Memory is a linear array of bytes. Load and store operations allow access to memory with optional sign or zero extension.

## Instruction Set

MASM uses a **Three-Operand Form (TOF)** where applicable: `OP dest, src1, src2`.
This avoids the destructive nature of two-operand ISAs (like x86) and maps 1:1 with RISC architectures.

### Data Movement

| Opcode | Syntax | Description |
|--------|--------|-------------|
| `mov` | `mov dst, src` | Copy `src` (register or immediate) to `dst`. |
| `load` | `load dst, [base + off], type` | Load from memory. `type` specifies size and extension (e.g., `i8` for sign-extend, `u8` for zero-extend). |
| `store` | `store [base + off], src, size` | Store `size` bytes from `src` to memory address. |
| `lea` | `lea dst, label` | Load Effective Address of a label or stack slot into `dst`. |

**Types for Load:** `i8`, `u8`, `i16`, `u16`, `i32`, `u32`, `u64`.
- Signed types (`i8`...) imply **sign-extension** to the destination register size.
- Unsigned types (`u8`...) imply **zero-extension**.

### Integer Arithmetic

| Opcode | Syntax | Description |
|--------|--------|-------------|
| `add` | `add dst, a, b` | `dst = a + b` |
| `sub` | `sub dst, a, b` | `dst = a - b` |
| `mul` | `mul dst, a, b` | `dst = a * b` (lower bits) |
| `div` | `div dst, a, b` | `dst = a / b` (signed) |
| `divu` | `divu dst, a, b` | `dst = a / b` (unsigned) |
| `rem` | `rem dst, a, b` | `dst = a % b` (signed remainder) |
| `remu` | `remu dst, a, b` | `dst = a % b` (unsigned remainder) |
| `neg` | `neg dst, src` | `dst = -src` (2's complement negation) |

### Bitwise Operations

| Opcode | Syntax | Description |
|--------|--------|-------------|
| `and` | `and dst, a, b` | `dst = a & b` |
| `or` | `or dst, a, b` | `dst = a | b` |
| `xor` | `xor dst, a, b` | `dst = a ^ b` |
| `not` | `not dst, src` | `dst = ~src` (bitwise complement) |
| `shl` | `shl dst, src, cnt` | `dst = src << cnt` (logical left shift) |
| `shr` | `shr dst, src, cnt` | `dst = src >> cnt` (logical right shift, zero fill) |
| `sar` | `sar dst, src, cnt` | `dst = src >> cnt` (arithmetic right shift, sign fill) |

### Comparisons (Set-if)

Comparisons produce `1` (true) or `0` (false) in the destination register.
MASM does **not** use implicit condition flags.

| Opcode | Syntax | Description |
|--------|--------|-------------|
| `seq` | `seq dst, a, b` | `dst = (a == b)` |
| `sne` | `sne dst, a, b` | `dst = (a != b)` |
| `slt` | `slt dst, a, b` | `dst = (a < b)` (signed) |
| `sltu` | `sltu dst, a, b` | `dst = (a < b)` (unsigned) |
| `sle` | `sle dst, a, b` | `dst = (a <= b)` (signed) |
| `sleu` | `sleu dst, a, b` | `dst = (a <= b)` (unsigned) |
| `sgt` | `sgt dst, a, b` | `dst = (a > b)` (signed) |
| `sgtu` | `sgtu dst, a, b` | `dst = (a > b)` (unsigned) |
| `sge` | `sge dst, a, b` | `dst = (a >= b)` (signed) |
| `sgeu` | `sgeu dst, a, b` | `dst = (a >= b)` (unsigned) |

### Floating Point

| Opcode | Syntax | Description |
|--------|--------|-------------|
| `fadd` | `fadd dst, a, b` | Float addition |
| `fsub` | `fsub dst, a, b` | Float subtraction |
| `fmul` | `fmul dst, a, b` | Float multiplication |
| `fdiv` | `fdiv dst, a, b` | Float division |
| `fcmp` | `fcmp dst, a, b, cond` | Float compare. `cond` is one of `eq, ne, lt, le, gt, ge`. |
| `fconv`| `fconv dst, src, mode` | Conversion. `mode`: `i2f`, `f2i`, `f32_f64`, `f64_f32`. |

### Control Flow

MASM distinguishes between unconditional jumps and conditional branches.
Conditional branches compare two operands directly (RISC-style), avoiding flags logic.

| Opcode | Syntax | Description |
|--------|--------|-------------|
| `jmp` | `jmp label` | Unconditional jump. |
| `beq` | `beq a, b, label` | Branch to `label` if `a == b`. |
| `bne` | `bne a, b, label` | Branch to `label` if `a != b`. |
| `blt` | `blt a, b, label` | Branch to `label` if `a < b` (signed). |
| `bltu` | `bltu a, b, label` | Branch to `label` if `a < b` (unsigned). |
| `bge` | `bge a, b, label` | Branch to `label` if `a >= b` (signed). |
| `bgeu` | `bgeu a, b, label` | Branch to `label` if `a >= b` (unsigned). |
| `call` | `call target, [args...]` | Call function `target` with arguments. |
| `ret` | `ret [val]` | Return from function, optionally with value. |

### System

| Opcode | Syntax | Description |
|--------|--------|-------------|
| `syscall` | `syscall dst, num, [args...]` | Execute system call `num`. Platform-specific ABI lowering. |

**Syscall Note:**
Syscalls are treated as generic operations. The backend is responsible for putting arguments into the correct physical registers defined by the target OS ABI (e.g., `RAX/RDI/RSI` on Linux x86-64, `X8/X0/X1` on Linux ARM64).

### Pseudo-Ops

| Opcode | Syntax | Description |
|--------|--------|-------------|
| `label` | `label:` | Define a code label. |
| `data` | `data val, size` | Emit static data. |

## Lowering Examples

### 1. Simple Comparison (`if a < b`)

**MASM:**
```asm
blt %a, %b, .then_block
```

**x86-64 Lowering:**
```asm
cmp %a, %b
jl .then_block
```

**ARM64 Lowering:**
```asm
cmp x0, x1  ; assuming %a=x0, %b=x1
b.lt .then_block
```

### 2. Arithmetic (`dst = a + b`)

**MASM:**
```asm
add %dst, %a, %b
```

**x86-64 Lowering:**
```asm
mov %dst, %a
add %dst, %b
```

**RISC-V Lowering:**
```asm
add %dst, %a, %b
```

### 3. Syscall (`write(1, msg, len)`)

**MASM:**
```asm
syscall %res, 1, %msg, %len
```

**Linux x86-64 Lowering:**
```asm
mov rax, 1
mov rdi, 1
mov rsi, %msg
mov rdx, %len
syscall
mov %res, rax
```
