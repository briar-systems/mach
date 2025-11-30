# MIR Overview

This document provides an overview of the Mid-level Intermediate Representation (MIR) used by the Mach compiler.

## What is MIR?

MIR (Mid-level Intermediate Representation) is an SSA-based intermediate language that sits between Mach's high-level AST and low-level machine code. It provides a stable abstraction layer that enables:

- **Platform-independent optimization** - MIR can be analyzed and transformed without knowledge of the target architecture
- **Type-aware code generation** - All operations carry type information for correct register allocation and instruction selection
- **Simplified lowering** - AST → MIR handles high-level constructs; MIR → assembly handles low-level details
- **Maintainability** - Clear separation between language semantics and code generation

## Design Goals

1. **Simplicity** - MIR uses a small, orthogonal set of operations
2. **SSA Form** - Single Static Assignment enables straightforward dataflow analysis
3. **Explicit Control Flow** - All branches and jumps are explicit basic block transitions
4. **Type Safety** - Every instruction and value carries type information
5. **Platform Abstraction** - Target-specific details isolated to codegen phase

## Architecture

The MIR compilation pipeline consists of several phases:

```
┌─────────┐
│   AST   │  High-level language constructs
└────┬────┘
     │
     ▼
┌─────────┐
│ Lowering│  AST → MIR transformation (platform-independent)
└────┬────┘
     │
     ▼
┌─────────┐
│   MIR   │  SSA-based intermediate representation (platform-agnostic)
└────┬────┘
     │
     ▼
┌─────────┐
│ Codegen │  MIR → target-specific assembly (pluggable backend)
└────┬────┘
     │
     ▼
┌─────────┐
│ Machine │  Target platform (ISA/ABI/OS/OF determined by MIRTarget)
│  Code   │
└─────────┘
```

### Target Platform Modularity

MIR is designed to be platform-agnostic. Target-specific details are isolated into pluggable components:

| Component | Current Implementation | Purpose |
|-----------|------------------------|---------|
| **ISA** (Instruction Set) | x86_64 | Register set, instruction selection |
| **ABI** (Calling Convention) | System V AMD64 | Parameter passing, register usage |
| **OS** (Operating System) | Linux | Syscalls, executable format details |
| **OF** (Object Format) | ELF | Object file structure and linking |

These components are defined in `MIRTarget` and can be swapped independently:

```c
typedef struct MIRTarget {
     MIRTargetISA isa;  // x86_64, arm64, risc-v, ...
     MIRTargetABI abi;  // sysv64, win64, aapcs, ...
     MIRTargetOS  os;   // linux, windows, macos, ...
     MIRTargetOF  of;   // elf, pe, mach-o, ...
} MIRTarget;
```

To add support for a new platform (e.g., ARM64/Linux/ELF):
1. Create `boot/src/compiler/mir/isa/arm64.c` with ARM64 instruction emission
2. Create `boot/src/compiler/mir/abi/aapcs.c` with ARM calling convention
3. Register new enums in `boot/include/compiler/mir/target.h`
4. No changes needed to MIR core, lowering, or other components

### Components

#### Module
The top-level container for a compilation unit. Contains:
- List of global variables and constants
- List of function definitions
- Target platform information

#### Function
Represents a callable function. Contains:
- Parameter list
- Return type
- List of basic blocks
- Local value allocations

#### Basic Block
A sequence of instructions with a single entry point and single exit point. Contains:
- List of MIR instructions
- Terminator instruction (branch, return, etc.)

#### Instruction
A single operation in SSA form. Contains:
- Opcode (operation type)
- Type information
- Result value (for non-void operations)
- Operands (inputs to the operation)

#### Value
An SSA value representing the result of an instruction. Each value:
- Is assigned exactly once
- Has an associated type
- Is mapped to a physical register during code generation

#### Operand
An input to an instruction. Can be:
- A reference to another SSA value
- An immediate constant
- A global symbol reference
- A basic block label

## SSA Form

MIR uses Static Single Assignment (SSA) form where each value is assigned exactly once:

```
# Valid MIR
%1 = const 42
%2 = add %1, %1
%3 = mul %2, %1

# Invalid - %1 assigned twice
%1 = const 42
%1 = add %1, %1  # error: redefinition
```

This property simplifies:
- **Dataflow analysis** - Use-def chains are trivial
- **Register allocation** - Each value has one lifetime
- **Optimization** - No need to track mutations

## Type System

Every MIR instruction and value has an associated type:

- **Primitives**: `u8`, `u16`, `u32`, `u64`, `i8`, `i16`, `i32`, `i64`, `f32`, `f64`, `ptr`
- **Pointers**: Typed pointers (`*T`) and untyped pointers (`ptr`)
- **Aggregates**: Records (structs), unions, arrays
- **Functions**: Function pointers with signature information

Type information is used for:
- Selecting correct machine instructions
- Determining register classes (integer vs. floating-point)
- Validating instruction operands
- Computing memory layout and alignment

## Example

Here's how a simple Mach function is lowered to MIR:

```mach
pub fun add(a: i64, b: i64) i64 {
    ret a + b;
}
```

```mir
function add(i64, i64) -> i64 {
  block_0:
    %1 = param 0  # type: i64
    %2 = param 1  # type: i64
    %3 = add %1, %2  # type: i64
    ret %3
}
```

## See Also

- [Opcodes](opcodes.md) - Complete instruction reference
- [Functions & Blocks](functions.md) - Function and control flow details
- [Codegen](codegen.md) - Machine code generation
