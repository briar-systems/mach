# MASM (Mach Assembly)

MASM is the compiler's intermediate representation and code generation infrastructure. It sits between the high-level AST and the final object file output, providing a portable instruction layer that can be lowered to target-specific machine code.

## Documentation Index

- **[Overview](overview.md)** - Introduction to MASM, design goals, and architecture
- **[Target Configuration](target.md)** - ISA, ABI, OS, and object format selection
- **[Portable IR](ir.md)** - Platform-independent instruction set reference
- **[Operands](operands.md)** - Operand types, registers, and memory references
- **[Sections](sections.md)** - Code and data section management
- **[Symbols](symbols.md)** - Symbol definitions and binding
- **[Types](types.md)** - MASM type system
- **[ABI](abi.md)** - Application Binary Interface and calling conventions
- **[Code Generation](codegen.md)** - Instruction selection and binary encoding

## Platform Support

MASM is designed as a platform-agnostic intermediate representation with modular, pluggable backends:

| Component | Current Support | Purpose |
|-----------|-----------------|---------|
| **ISA** | x86_64 | Instruction set, registers, encoding |
| **ABI** | System V AMD64 | Calling convention, parameter passing |
| **OS** | Linux | Syscall conventions |
| **OF** | ELF | Object file format |

These components are independent and can be extended separately. See [Target Configuration](target.md) for details on adding new platform support.

## Architecture

The MASM compilation pipeline:

```
┌─────────┐
│   AST   │  High-level language constructs
└────┬────┘
     │
     ▼
┌─────────┐
│ Lowering│  AST → MASM IR (platform-independent)
└────┬────┘
     │
     ▼
┌─────────┐
│ MASM IR │  Portable three-operand instructions
└────┬────┘
     │
     ▼
┌─────────┐
│  ISel   │  IR → target-specific instructions
└────┬────┘
     │
     ▼
┌─────────┐
│ Encode  │  Instructions → machine code bytes
└────┬────┘
     │
     ▼
┌─────────┐
│ Object  │  ELF/PE/Mach-O output
└─────────┘
```

## Two-Layer Opcode Architecture

MASM uses a two-layer opcode system:

**Layer 1: Portable IR**
- Platform-independent three-operand instructions
- Flag-free comparisons (set-if semantics)
- Emitted by AST lowering
- Consumed by instruction selection (isel)

**Layer 2: Target-Specific Opcodes**
- Machine-level instructions (e.g., `MasmX86Opcode`)
- Full target semantics (flags, register constraints)
- Emitted by isel and inline asm parsers
- Consumed by encoder for binary emission

The `MasmOpcodeKind` discriminator on each instruction indicates which layer it belongs to, enabling type-safe dispatch through the emit pipeline.

## Quick Reference

### Common IR Operations

| Category | Operations |
|----------|------------|
| **Data Movement** | `mov`, `load`, `store`, `lea`, `zext`, `sext` |
| **Arithmetic** | `add`, `sub`, `mul`, `div`, `divu`, `rem`, `remu`, `neg` |
| **Bitwise** | `and`, `or`, `xor`, `not`, `shl`, `shr`, `sar` |
| **Comparison** | `seq`, `sne`, `slt`, `sltu`, `sle`, `sleu`, `sgt`, `sgtu`, `sge`, `sgeu` |
| **Floating Point** | `fadd`, `fsub`, `fmul`, `fdiv`, `fcmp`, `fconv` |
| **Control Flow** | `jmp`, `beq`, `bne`, `blt`, `bltu`, `bge`, `bgeu`, `call`, `ret` |
| **System** | `syscall` |
| **Pseudo-ops** | `label`, `data`, `stack_frame` |

### Type System

- **Integers**: `i8`, `u8`, `i16`, `u16`, `i32`, `u32`, `i64`, `u64`
- **Floats**: `f32`, `f64`
- **Pointers**: `ptr`
- **Compound**: Arrays, records, functions

## Implementation Files

**Core Infrastructure:**

| Component | Header | Implementation |
|-----------|--------|----------------|
| Container | `masm/masm.h` | `masm/masm.c` |
| IR Opcodes | `masm/ir.h` | `masm/ir.c` |
| Target | `masm/target.h` | `masm/target.c` |
| Operands | `masm/operand.h` | `masm/operand.c` |
| Sections | `masm/section.h` | `masm/section.c` |
| Symbols | `masm/symbol.h` | `masm/symbol.c` |
| Types | `masm/type.h` | `masm/type.c` |
| Lowering | `masm/lower.h` | `masm/lower.c` |
| Emission | `masm/emit.h` | `masm/emit.c` |

**Platform-Specific Backends:**

| Component | Location |
|-----------|----------|
| x86_64 ISA | `masm/isa/x86_64/` |
| System V ABI | `masm/abi/sysv64.c` |
| ELF Format | `masm/of/elf.c` |

All paths are relative to `src/compiler/`.