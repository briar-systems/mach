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

The `op` field on each `Inst` record holds a portable `Operator` value during lowering. After instruction selection, the ISA backend replaces these with target-specific opcode values.

## Quick Reference

### Common IR Operations

| Category | Operations |
|----------|------------|
| **Data Movement** | `mov`, `load`, `store`, `addr` |
| **Integer Arithmetic** | `add`, `sub`, `mul`, `div`, `mod`, `divu`, `modu` |
| **Bitwise** | `and`, `or`, `xor`, `shl`, `shr`, `sar` |
| **Comparison** | `icmp` (with cc: `eq`, `ne`, `lt`, `le`, `gt`, `ge`, `b`, `be`, `a`, `ae`) |
| **Conversions** | `zext`, `sext`, `trunc`, `itof`, `ftoi`, `fext`, `ftrunc` |
| **Floating Point** | `fadd`, `fsub`, `fmul`, `fdiv`, `fcmp` |
| **Control Flow** | `jmp`, `brcond`, `call`, `ret` |
| **System** | `syscall` |
| **Pseudo-ops** | `label` |
| **Internal** | `nop`, `memset` |

### Type System

- **Integers**: `i8`, `u8`, `i16`, `u16`, `i32`, `u32`, `i64`, `u64`
- **Floats**: `f32`, `f64`
- **Pointers**: `ptr`
- **Compound**: Arrays, records, functions

## Implementation Files

All paths are relative to `src/compiler/`:

| Module | Path | Description |
|--------|------|-------------|
| Context | `masm.mach` | MASM context, section/symbol management |
| IR | `masm/ir.mach` | IR opcodes, Inst/Operand/Block/Function records |
| Sections | `masm/section.mach` | Section kinds, byte emission, relocations |
| Symbols | `masm/symbol.mach` | Symbol binding, kinds, symbol table |
| Types | `masm/type.mach` | Machine-level type representation |
| Target | `target.mach` | Target triple parsing and interface composition |
| Object Format | `of.mach` | ObjectFormat/ExecFormat/OutputBuffer interfaces |
| Lowering | `lower.mach` | AST → MASM IR |
| ISA | `isa/x86_64/` | x86_64 isel, encoding |
| ABI | `abi/sysv64.mach` | System V AMD64 calling convention |
| ELF | `of/elf/` | ELF object and executable emission |