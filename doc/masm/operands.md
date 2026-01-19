# Operands

This document describes the operand types used in MASM instructions.

## Overview

Operands are the inputs and outputs of MASM instructions. Each operand has a kind that determines how it is interpreted during code generation.


## Operand Kinds

| Kind | Description | Example |
|------|-------------|---------|
| `MASM_OPERAND_NONE` | No operand (placeholder) | - |
| `MASM_OPERAND_REGISTER` | Physical or virtual register | `%rax`, `%xmm0` |
| `MASM_OPERAND_IMM` | Immediate constant | `42`, `-1` |
| `MASM_OPERAND_MEMORY` | Memory reference | `[rbp - 8]` |
| `MASM_OPERAND_SYMBOL` | Global symbol reference | `@global_var` |
| `MASM_OPERAND_LABEL` | Code label reference | `.loop_start` |
| `MASM_OPERAND_TYPE` | Type descriptor | `i64`, `f32` |


## Register Operands

Registers represent storage locations for values during execution.

### Register Structure

```c
typedef struct MasmRegister {
    uint32_t          id;     // register identifier
    uint8_t           size;   // size in bytes (1, 2, 4, 8, 16)
    MasmRegisterClass class;  // int or float
} MasmRegister;
```

### Register Classes

| Class | Description | x86_64 Examples |
|-------|-------------|-----------------|
| `MASM_REG_CLASS_INT` | General-purpose integer | RAX, RBX, RCX, ... |
| `MASM_REG_CLASS_FLOAT` | Floating-point / SIMD | XMM0, XMM1, ... |

### Creating Register Operands

```c
// integer register
MasmOperand reg = masm_operand_register(MASM_X86_RAX, 8);

// floating-point register
MasmOperand xmm = masm_operand_register_fp(0, 16);  // XMM0
```

### x86_64 Register IDs

| ID | 64-bit | 32-bit | 16-bit | 8-bit |
|----|--------|--------|--------|-------|
| 0 | RAX | EAX | AX | AL |
| 1 | RCX | ECX | CX | CL |
| 2 | RDX | EDX | DX | DL |
| 3 | RBX | EBX | BX | BL |
| 4 | RSP | ESP | SP | SPL |
| 5 | RBP | EBP | BP | BPL |
| 6 | RSI | ESI | SI | SIL |
| 7 | RDI | EDI | DI | DIL |
| 8-15 | R8-R15 | R8D-R15D | R8W-R15W | R8B-R15B |

The `size` field determines which sub-register is accessed.


## Immediate Operands

Immediate operands represent constant values embedded directly in instructions.

### Creating Immediate Operands

```c
MasmOperand imm = masm_operand_imm(42);
MasmOperand neg = masm_operand_imm(-1);
```

### Immediate Range

Immediates are stored as `int64_t` and may be constrained by the target instruction encoding:
- x86_64 most instructions: 32-bit sign-extended
- x86_64 `mov` to 64-bit register: full 64-bit immediate


## Memory Operands

Memory operands reference values in memory using base, index, scale, and displacement.

### Memory Structure

```c
typedef struct MasmMemory {
    MasmRegister base;   // base register
    MasmRegister index;  // index register (optional)
    uint8_t      scale;  // index scale: 1, 2, 4, or 8
    int64_t      disp;   // displacement offset
    uint8_t      size;   // access size in bytes
} MasmMemory;
```

### Addressing Modes

**Simple**: `[base + disp]`
```c
MasmOperand mem = masm_operand_memory_simple(MASM_X86_RBP, -8, 8);
// equivalent to: [rbp - 8]
```

**Full**: `[base + index * scale + disp]`
```c
MasmRegister base = { MASM_X86_RBP, 8, MASM_REG_CLASS_INT };
MasmRegister index = { MASM_X86_RCX, 8, MASM_REG_CLASS_INT };
MasmOperand mem = masm_operand_memory(base, index, 4, 0, 8);
// equivalent to: [rbp + rcx * 4]
```

### Common Patterns

| Pattern | Description | Example |
|---------|-------------|---------|
| `[rbp - N]` | Local variable | Stack-allocated locals |
| `[rsp + N]` | Stack argument | Parameters beyond 6th |
| `[rip + sym]` | Global access | RIP-relative addressing |
| `[base + idx*scale]` | Array access | Indexed array element |


## Symbol Operands

Symbol operands reference named locations in the program (functions, global variables, string literals).

### Creating Symbol Operands

```c
MasmOperand sym = masm_operand_symbol("my_function");
MasmOperand global = masm_operand_symbol("global_counter");
```

### Usage

Symbols are resolved during linking. In code generation, they typically become:
- RIP-relative addresses (x86_64)
- Relocations in the object file


## Label Operands

Label operands reference code locations within the current function for control flow.

### Creating Label Operands

```c
MasmOperand label = masm_operand_label(".loop_start");
MasmOperand exit = masm_operand_label(".func_end");
```

### Label Naming

Labels are function-local. By convention:
- Start with `.` to indicate local scope
- Use descriptive names: `.loop_body`, `.if_else`, `.return`


## Type Operands

Type operands carry type information for instructions that need it (conversions, sized operations).

### Creating Type Operands

```c
MasmOperand t = masm_operand_type(MASM_TYPE_I64);
```

### Type Kinds

| Kind | Size | Description |
|------|------|-------------|
| `MASM_TYPE_VOID` | 0 | No type / void |
| `MASM_TYPE_I8` | 1 | Signed 8-bit integer |
| `MASM_TYPE_U8` | 1 | Unsigned 8-bit integer |
| `MASM_TYPE_I16` | 2 | Signed 16-bit integer |
| `MASM_TYPE_U16` | 2 | Unsigned 16-bit integer |
| `MASM_TYPE_I32` | 4 | Signed 32-bit integer |
| `MASM_TYPE_U32` | 4 | Unsigned 32-bit integer |
| `MASM_TYPE_I64` | 8 | Signed 64-bit integer |
| `MASM_TYPE_U64` | 8 | Unsigned 64-bit integer |
| `MASM_TYPE_F32` | 4 | 32-bit float |
| `MASM_TYPE_F64` | 8 | 64-bit float |
| `MASM_TYPE_PTR` | 8 | Pointer (64-bit) |


## Operand Structure

The complete operand representation:

```c
typedef struct MasmOperand {
    MasmOperandKind kind;
    union {
        MasmRegister reg;       // MASM_OPERAND_REGISTER
        int64_t      imm;       // MASM_OPERAND_IMM
        MasmMemory   mem;       // MASM_OPERAND_MEMORY
        const char  *symbol;    // MASM_OPERAND_SYMBOL
        const char  *label;     // MASM_OPERAND_LABEL
        MasmTypeKind type;      // MASM_OPERAND_TYPE
    };
} MasmOperand;
```


## Builder Functions

### Summary

| Function | Description |
|----------|-------------|
| `masm_operand_none()` | Create empty operand |
| `masm_operand_register(id, size)` | Integer register |
| `masm_operand_register_fp(id, size)` | Float register |
| `masm_operand_imm(value)` | Immediate constant |
| `masm_operand_memory(base, index, scale, disp, size)` | Full memory reference |
| `masm_operand_memory_simple(base_reg, disp, size)` | Simple memory reference |
| `masm_operand_symbol(name)` | Global symbol |
| `masm_operand_label(name)` | Code label |
| `masm_operand_type(kind)` | Type descriptor |


## Examples

### Load from Local Variable

```c
// load value from [rbp - 16]
MasmOperand dst = masm_operand_register(MASM_X86_RAX, 8);
MasmOperand src = masm_operand_memory_simple(MASM_X86_RBP, -16, 8);
MasmInstruction inst = masm_inst_2(MASM_IR_LOAD, dst, src);
```

### Store to Global

```c
// store %rax to global "counter"
MasmOperand dst = masm_operand_symbol("counter");
MasmOperand src = masm_operand_register(MASM_X86_RAX, 8);
MasmInstruction inst = masm_inst_2(MASM_IR_STORE, dst, src);
```

### Conditional Branch

```c
// branch to .loop if %rcx != 0
MasmOperand a = masm_operand_register(MASM_X86_RCX, 8);
MasmOperand b = masm_operand_imm(0);
MasmOperand target = masm_operand_label(".loop");
MasmInstruction inst = masm_inst_3(MASM_IR_BNE, a, b, target);
```


## See Also

- [IR Opcodes](ir.md) - Instructions that use operands
- [Sections](sections.md) - Where instructions are stored
- [Code Generation](codegen.md) - How operands are encoded