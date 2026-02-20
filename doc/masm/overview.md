# MASM Overview

This document provides an introduction to MASM (Mach Assembly), the intermediate representation and code generation system used by the Mach compiler.

## What is MASM?

MASM is a portable, low-level intermediate representation designed to sit between the Mach compiler frontend (AST) and platform-specific backends. It provides:

1. **Portability** - Abstract enough to map cleanly to x86_64, ARM64, RISC-V, and future targets
2. **Readability** - Human-friendly syntax familiar to assembly programmers
3. **Modularity** - Clean separation between platform-independent IR and target-specific code generation
4. **Simplicity** - A reduced instruction set that avoids ISA-specific quirks

## Design Goals

1. **Two-Layer Architecture** - Separate portable IR from target-specific instructions
2. **Flag-Free Comparisons** - Comparisons produce values, not implicit condition flags
3. **Fixed Three-Operand Form** - Every instruction has exactly `dst`, `src1`, `src2` (unused slots zeroed)
4. **Explicit Control Flow** - All branches use explicit labels
5. **Type-Aware** - Instructions carry size information for correct code generation

## Architecture

### Compilation Pipeline

```
+-----------------------+
|     Mach Source        |
+----------+------------+
           | parse
           v
+-----------------------+
|        AST            |
+----------+------------+
           | lower.mach
           v
+-----------------------+
|      MASM IR          |  Portable three-operand instructions
+----------+------------+
           | isel (instruction selection)
           v
+-----------------------+
|  Target Instructions  |  x86_64, ARM64, etc.
+----------+------------+
           | encode
           v
+-----------------------+
|   Machine Code        |  Raw bytes
+----------+------------+
           | emit (ELF, PE, etc.)
           v
+-----------------------+
|    Object File        |
+-----------------------+
```

### Two-Layer Opcode Architecture

MASM uses a two-layer opcode system to cleanly separate portable and target-specific instructions.

**Layer 1: Portable IR**

Platform-independent instructions using fixed three-operand form:

```mach
# defined in src/compiler/masm/ir.mach
pub def Operator: u16;

pub val OP_MOV:   Operator = 0;
pub val OP_ADD:   Operator = 4;
pub val OP_SUB:   Operator = 5;
pub val OP_ICMP:  Operator = 17;
pub val OP_BRCOND: Operator = 20;
# ... more IR opcodes
```

- Emitted by the lowering phase (`lower.mach`)
- No implicit flags or register constraints
- Comparisons use `OP_ICMP`/`OP_FCMP` with a condition code field

**Layer 2: Target-Specific**

Machine-level instructions with full target semantics. These are ISA-specific opcodes defined by each backend (e.g., x86_64 backend defines MOV_RR, ADD_RI, SYSCALL, etc.).

- Emitted by instruction selection (isel)
- Includes register/memory addressing variants
- Consumed by the encoder for binary emission

### Core Components

| Component | Purpose |
|-----------|---------|
| `Inst` | Single IR instruction with op, dst, src1, src2, cc, size |
| `Operand` | Instruction operand: register, immediate, memory, symbol, or label |
| `Block` | Basic block: label + linear sequence of instructions |
| `Function` | Ordered list of basic blocks |

### Target Configuration

MASM supports modular target configuration through four independent components, specified via `mach.toml`:

```toml
[targets.linux]
os  = "linux"
isa = "x86_64"
abi = "sysv64"
```

Each component (ISA, ABI, OS, Object Format) has its own specification interface that provides the necessary hooks for that layer.

## Example

### Mach Source

```mach
pub fun add(a: i64, b: i64) i64 {
    ret a + b;
}
```

### MASM IR (after lowering)

```
add:
    ; a is in first arg register, b is in second
    add %result, %arg0, %arg1
    ret %result
```

### x86_64 Output (after isel and encode)

```asm
add:
    push rbp
    mov rbp, rsp
    mov rax, rdi
    add rax, rsi
    pop rbp
    ret
```

## Adding Platform Support

To add support for a new target (e.g., ARM64/Linux/ELF):

1. **ISA Implementation** - Create a new ISA module under `src/compiler/`:
   - Define target-specific opcode values
   - Implement ISA spec interface (register roles, isel, encode)

2. **ABI Implementation** - Implement the ABI spec interface:
   - Define argument registers and calling convention

3. **Register Targets** - Add new enum values for ISA and opcode kind

No changes are needed to the lowering phase, IR definitions, or emit infrastructure -- they are platform-independent.

## See Also

- [IR Opcodes](ir.md) - Complete portable instruction reference
- [Target Configuration](target.md) - Platform selection details
- [Code Generation](codegen.md) - Instruction selection and encoding
- [ABI](abi.md) - Calling conventions
