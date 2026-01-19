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
3. **Three-Operand Form** - Avoids destructive two-operand semantics
4. **Explicit Control Flow** - All branches use explicit labels
5. **Type-Aware** - Instructions carry type information for correct code generation

## Architecture

### Compilation Pipeline

```
┌─────────────────────┐
│     Mach Source     │
└──────────┬──────────┘
           │ parse
           ▼
┌─────────────────────┐
│        AST          │
└──────────┬──────────┘
           │ lower.c
           ▼
┌─────────────────────┐
│      MASM IR        │  Portable three-operand instructions
└──────────┬──────────┘
           │ isel (instruction selection)
           ▼
┌─────────────────────┐
│  Target Instructions│  x86_64, ARM64, etc.
└──────────┬──────────┘
           │ encode
           ▼
┌─────────────────────┐
│   Machine Code      │  Raw bytes
└──────────┬──────────┘
           │ emit (ELF, PE, etc.)
           ▼
┌─────────────────────┐
│    Object File      │
└─────────────────────┘
```

### Two-Layer Opcode Architecture

MASM uses a discriminated two-layer opcode system to cleanly separate portable and target-specific instructions:

**Layer 1: Portable IR (`MASM_OPCODE_IR`)**

Platform-independent instructions using three-operand form:

```c
typedef enum MasmIrOpcode {
    MASM_IR_MOV,
    MASM_IR_ADD,
    MASM_IR_SUB,
    // ... more IR opcodes
} MasmIrOpcode;
```

- Emitted by the lowering phase (`lower.c`)
- No implicit flags or register constraints
- Comparisons use set-if semantics (produce 0 or 1)

**Layer 2: Target-Specific (`MASM_OPCODE_X86`, etc.)**

Machine-level instructions with full target semantics:

```c
typedef enum MasmX86Opcode {
    MASM_OP_X86_MOV_RR,
    MASM_OP_X86_ADD_RI,
    MASM_OP_X86_SYSCALL,
    // ... more x86 opcodes
} MasmX86Opcode;
```

- Emitted by instruction selection (isel)
- Includes register/memory addressing variants
- Consumed by the encoder for binary emission

The `MasmOpcodeKind` discriminator on each instruction indicates which layer it belongs to:

```c
typedef enum MasmOpcodeKind {
    MASM_OPCODE_IR = 0,   // portable IR
    MASM_OPCODE_X86,      // x86_64 target
    // future: MASM_OPCODE_ARM64, etc.
} MasmOpcodeKind;
```

### Core Components

| Component | Purpose |
|-----------|---------|
| `Masm` | Top-level container holding sections, symbols, and target info |
| `MasmSection` | Code (`.text`) or data (`.data`, `.rodata`, `.bss`) segment |
| `MasmSymbol` | Named location in a section (function, label, data) |
| `MasmInstruction` | Single operation with opcode and operands |
| `MasmOperand` | Instruction input: register, immediate, memory, symbol, or label |
| `MasmTarget` | Platform configuration (ISA + ABI + OS + OF) |

### Target Configuration

MASM supports modular target configuration through four independent components:

```c
typedef struct MasmTarget {
    MasmTargetISA isa;  // x86_64, arm64, riscv, ...
    MasmTargetABI abi;  // sysv64, win64, aapcs, ...
    MasmTargetOS  os;   // linux, windows, macos, ...
    MasmTargetOF  of;   // elf, pe, mach-o, ...
} MasmTarget;
```

Each component has its own specification interface (`MasmISASpec`, `MasmABISpec`, `MasmOFSpec`) that provides the necessary hooks for that layer.

## Example

### Mach Source

```mach
pub fun add(a: i64, b: i64) i64 {
    ret a + b;
}
```

### MASM IR (after lowering)

```
section .text

add:
    stack_frame 0
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

1. **ISA Implementation** - Create `masm/isa/arm64/arm64.h` and `arm64.c`:
   - Define `MasmArm64Opcode` enum
   - Implement `MasmISASpec` interface (register roles, isel, encode)

2. **ABI Implementation** - Create `masm/abi/aapcs.c`:
   - Implement `MasmABISpec` interface (argument registers, calling convention)

3. **Register Targets** - Add new enums to `masm/target.h`:
   - `MASM_ISA_ARM64` to `MasmTargetISA`
   - `MASM_OPCODE_ARM64` to `MasmOpcodeKind`

No changes are needed to the lowering phase, IR definitions, or emit infrastructure - they are platform-independent.

## See Also

- [IR Opcodes](ir.md) - Complete portable instruction reference
- [Target Configuration](target.md) - Platform selection details
- [Code Generation](codegen.md) - Instruction selection and encoding
- [ABI](abi.md) - Calling conventions