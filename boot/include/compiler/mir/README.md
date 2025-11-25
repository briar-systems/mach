# MIR (Mach Intermediate Representation) Architecture

## Core MIR Components (Platform-Agnostic SSA)

### `opcode.h` - Operation Codes
- Defines all MIR operations (add, load, br, call, etc.)
- Architecture-independent instruction set
- Utilities for opcode classification

### `value.h` - SSA Values
- Represents SSA values (virtual registers)
- Each value has unique ID, type, and defining instruction
- Optional human-readable names

### `operand.h` - Instruction Operands
- Value references, immediates, globals, blocks
- Type-safe operand construction

### `inst.h` - Instructions
- SSA instructions with operation, type, result, and operands
- Builder functions for common instruction patterns

### `block.h` - Basic Blocks
- CFG basic blocks containing instructions
- Each block has unique ID and optional label
- Linked list of instructions

### `function.h` - Functions
- SSA functions with parameters and blocks
- Entry block is first block
- Allocates values and blocks with unique IDs

### `global.h` - Global Data
- Global variables and constants (val/var)
- Initializer data support

### `module.h` - Compilation Unit
- Contains functions and globals
- Module-level lookup functions

## Target Abstraction Layers

### ISA: `isa/x86_64.h`
- Physical register definitions (RAX, RDI, XMM0, etc.)
- Register classification (GP vs FP)
- Register utilities for allocator

### ABI: `abi/sysv64.h`
- System V AMD64 calling convention
- Argument/return register mappings (RDI, RSI, RDX, RCX, R8, R9)
- Caller/callee-saved register classification
- Stack alignment (16 bytes)

### OS: `os/linux.h`
- System call number mappings
- Entry point name ("_start")
- Linking requirements (PLT/GOT)

### Object File: `of/elf.h`
- ELF64 file format generation
- Section management (.text, .data, .bss, .rodata)
- Symbol table and relocations

## Pipeline Components

### `lower.h` - AST → MIR Lowering
- Converts Mach AST to SSA MIR
- Handles inline `mir {}` blocks
- Maps Mach variables to SSA values

### `emit.h` - MIR → Machine Code
- Compilation pipeline orchestration
- Target configuration (ISA + ABI + OS + OF)
- Produces executables or object files

## Compilation Flow

```
Mach Source
     ↓ (parser)
Mach AST
     ↓ (mir_lower_module)
SSA MIR (virtual registers, architecture-agnostic)
     ↓ (instruction selection - ISA-specific)
SSA MIR (ISA-specific operations, still virtual registers)
     ↓ (register allocation - ABI-aware)
MIR (physical registers)
     ↓ (code emission - ISA-specific encoding)
Machine Code Bytes
     ↓ (object file generation - OF-specific)
ELF64 Object File or Executable
```

## MIR Syntax Reference

```mir
# globals
pub val message: [13]u8 = "Hello, world!";
var counter: i64 = 0;

# function
pub fun factorial(n: i64) i64 {
lab entry
    is_zero: i8 = eq:i64 n, 0;
    br is_zero, base_case, recurse;
    
lab base_case
    ret:i64 1;
    
lab recurse
    n_minus_1: i64 = sub:i64 n, 1;
    rec_result: i64 = call:i64 factorial(n_minus_1);
    result: i64 = mul:i64 n, rec_result;
    ret:i64 result;
}
```

## Design Principles

1. **KISS** - Keep implementations simple and straightforward
2. **Modularity** - Each component is self-contained with clean API
3. **No "god files"** - Separate headers for each concern
4. **SSA Form** - Single assignment, explicit control flow
5. **Type-aware** - All operations explicitly typed
6. **Platform-agnostic** - ISA/ABI/OS abstracted until emission
