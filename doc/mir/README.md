# MIR (Mid-level Intermediate Representation)

MIR is the compiler's internal representation used between the high-level AST and low-level machine code generation. It provides a stable, SSA-based intermediate form that is easier to analyze and optimize than either the source language or assembly.

## Documentation Index

- **[Overview](overview.md)** - Introduction to MIR, design goals, and architecture
- **[Opcodes](opcodes.md)** - Complete reference of all MIR operations
- **[Types](types.md)** - Type system and type representation in MIR
- **[Values & Operands](values.md)** - SSA values, operands, and register allocation
- **[Functions & Blocks](functions.md)** - Function structure, basic blocks, and control flow
- **[Globals](globals.md)** - Global variables, constants, and data sections
- **[ABI](abi.md)** - Application Binary Interface and calling conventions
- **[Codegen](codegen.md)** - Machine code generation

## Platform Support

MIR is designed as a platform-agnostic intermediate representation. Target-specific details are isolated into modular components:

**Architecture** (`MIRTarget`):
- **ISA**: Instruction set (currently: x86_64)
- **ABI**: Calling convention (currently: System V AMD64)
- **OS**: Operating system (currently: Linux)
- **OF**: Object format (currently: ELF)

These components are pluggable and can be extended independently. See [Overview](overview.md) for details on adding new platform support.

## Quick Reference

### Common Operations

| Category | Operations |
|----------|------------|
| **Arithmetic** | `add`, `sub`, `mul`, `div`, `and`, `or`, `xor` |
| **Memory** | `load`, `store`, `mov` |
| **Control Flow** | `br`, `brcond`, `ret`, `call` |
| **Comparison** | `eq`, `ne`, `lt`, `le`, `gt`, `ge` |
| **Conversion** | `zext`, `sext`, `trunc`, `fptosi`, `sitofp` |

### Type System

- **Primitives**: `u8`, `u16`, `u32`, `u64`, `i8`, `i16`, `i32`, `i64`, `f32`, `f64`, `ptr`
- **Compound**: Pointers, arrays, records (structs), unions
- **Functions**: Function types with parameter and return types

### SSA Properties

- Every value is assigned exactly once (Single Static Assignment)
- Values are immutable after assignment
- Control flow represented via basic blocks and explicit branches
- Phi nodes not currently used (value mapping handled during lowering)

## Implementation Files

**Platform-Independent Core:**

| Component | Location |
|-----------|----------|
| Module | `boot/src/compiler/mir/module.c` |
| Functions | `boot/src/compiler/mir/function.c` |
| Blocks | `boot/src/compiler/mir/block.c` |
| Instructions | `boot/src/compiler/mir/inst.c` |
| Opcodes | `boot/src/compiler/mir/opcode.c` |
| Values | `boot/src/compiler/mir/value.c` |
| Operands | `boot/src/compiler/mir/operand.c` |
| Globals | `boot/src/compiler/mir/global.c` |
| Lowering | `boot/src/compiler/mir/lower.c` |
| Target Config | `boot/src/compiler/mir/target.c` |
| Object Emission | `boot/src/compiler/mir/emit.c` |

**Platform-Specific Backends** (current implementation):

| Component | Location |
|-----------|----------|
| x86_64 ISA | `boot/src/compiler/mir/isa/x86_64.c` |
| x86_64 Codegen | `boot/src/compiler/mir/codegen/x86_64.c` |
| System V ABI | `boot/src/compiler/mir/abi/sysv64.c` |
| Linux OS | `boot/src/compiler/mir/os/linux.c` |
| ELF Format | `boot/src/compiler/mir/of/elf.c` |
