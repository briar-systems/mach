# Code Generation

This document describes how MASM IR is lowered to machine code.

## Overview

Code generation transforms portable MASM IR into target-specific machine code through several phases:

1. **Instruction Selection (isel)** - IR opcodes to target-specific opcodes
2. **Register Allocation** - Virtual registers to physical registers
3. **Encoding** - Opcodes to machine code bytes
4. **Object Emission** - Sections to ELF/PE/Mach-O file


## Pipeline

```
MASM IR Instructions (Inst records)
        |
        v
+---------------------+
|  Instruction         |  Lower IR opcodes to target-specific opcodes
|  Selection (isel)    |  e.g., OP_ADD -> x86 ADD_RR
+----------+-----------+
           |
           v
+---------------------+
|  Register            |  Map virtual registers to physical registers
|  Allocation          |  Handle spills to stack
+----------+-----------+
           |
           v
+---------------------+
|  Encoding            |  Generate machine code bytes
|                      |  e.g., ADD -> 0x48 0x01 ...
+----------+-----------+
           |
           v
+---------------------+
|  Object Emission     |  Write ELF file with sections,
|                      |  symbols, and relocations
+---------------------+
```


## Instruction Selection

Instruction selection (isel) transforms platform-independent IR into target-specific opcodes. The ISA spec interface provides the isel hook.

### IR to x86_64 Mapping

| IR Opcode | x86_64 Output |
|-----------|---------------|
| `OP_MOV` | MOV_RR, MOV_RI |
| `OP_ADD` | ADD_RR, ADD_RI |
| `OP_SUB` | SUB_RR, SUB_RI |
| `OP_MUL` | IMUL_RR |
| `OP_DIV` | CQO + IDIV |
| `OP_LOAD` | MOV_RM, MOVQ (float) |
| `OP_STORE` | MOV_MR |
| `OP_ICMP` | CMP_RR + SETcc |
| `OP_BRCOND` | CMP + Jcc |
| `OP_CALL` | CALL_REL |
| `OP_RET` | RET |
| `OP_SYSCALL` | SYSCALL |

### Operand Variants

x86_64 instructions have multiple forms based on operand types:

| Suffix | Operands | Example |
|--------|----------|---------|
| `_RR` | Register, Register | `add rax, rbx` |
| `_RI` | Register, Immediate | `add rax, 42` |
| `_RM` | Register, Memory | `mov rax, [rbp-8]` |
| `_MR` | Memory, Register | `mov [rbp-8], rax` |
| `_MI` | Memory, Immediate | `mov [rbp-8], 42` |

### Example: Addition

**IR**:
```
add %dst, %a, %b
```

**Isel output** (three-operand to two-operand conversion):
```
mov_rr %dst, %a    ; dst = a
add_rr %dst, %b    ; dst = dst + b
```


## Register Allocation

### Strategy

The current allocator uses a simple strategy:

1. **Parameters** - Assigned to ABI-designated registers
2. **Return value** - RAX (integer) or XMM0 (float)
3. **Temporaries** - Allocated from scratch registers
4. **Spills** - Overflow to stack slots

### x86_64 Register Classes

**Scratch Registers** (caller-saved, available for allocation):
RAX, RCX, RDX, R8, R9, R10, R11

**Reserved Registers** (not available for general allocation):
RSP, RBP (stack/frame pointers), RBX, R12, R13, R14, R15 (callee-saved)

### Register Role Helpers

The ISA spec provides role-based register access:

| Role | Description |
|------|-------------|
| `reg_result` | Return value register |
| `reg_tmp0` | First temporary |
| `reg_tmp1` | Second temporary |
| `reg_arg(index)` | Argument register by index |
| `reg_sp` | Stack pointer |
| `reg_fp` | Frame pointer |


## Encoding

Encoding transforms target-specific opcodes into machine code bytes.

### x86_64 Instruction Format

```
+---------+---------+---------+---------+---------+---------+
| Prefix  |   REX   | Opcode  | ModR/M  |   SIB   | Disp/Imm|
| (opt)   | (opt)   | 1-3 B   | (opt)   | (opt)   | (opt)   |
+---------+---------+---------+---------+---------+---------+
```

### REX Prefix

For 64-bit operands and extended registers (R8-R15):

| Bit | Name | Purpose |
|-----|------|---------|
| 0x08 | REX.W | 64-bit operand size |
| 0x04 | REX.R | Extend ModR/M reg field |
| 0x02 | REX.X | Extend SIB index field |
| 0x01 | REX.B | Extend ModR/M r/m or SIB base |

### ModR/M Byte

| Mod | Description |
|-----|-------------|
| 00 | Memory, no displacement |
| 01 | Memory, 8-bit displacement |
| 10 | Memory, 32-bit displacement |
| 11 | Register direct |


## Floating-Point Operations

Floating-point uses SSE instructions with XMM registers.

### Common SSE Instructions

| IR | x86_64 | Encoding |
|----|--------|----------|
| `OP_FADD` | `addsd` | `F2 0F 58 /r` |
| `OP_FSUB` | `subsd` | `F2 0F 5C /r` |
| `OP_FMUL` | `mulsd` | `F2 0F 59 /r` |
| `OP_FDIV` | `divsd` | `F2 0F 5E /r` |
| `OP_LOAD` (f64) | `movsd` | `F2 0F 10 /r` |
| `OP_STORE` (f64) | `movsd` | `F2 0F 11 /r` |


## Relocations

References to symbols generate relocations in the object file.

### Relocation Types

| Type | Usage |
|------|-------|
| `R_X86_64_PC32` | 32-bit PC-relative (data references) |
| `R_X86_64_PLT32` | 32-bit PLT-relative (function calls) |
| `R_X86_64_32` | 32-bit absolute |
| `R_X86_64_64` | 64-bit absolute |


## Object Emission

Final output is an ELF object file.

### ELF Structure

```
+--------------------+
|    ELF Header      |
+--------------------+
|  Section: .text    |  <-- Encoded machine code
+--------------------+
| Section: .rodata   |  <-- Constants, strings
+--------------------+
|  Section: .data    |  <-- Initialized globals
+--------------------+
|  Section: .bss     |  <-- Uninitialized globals
+--------------------+
|    .rela.text      |  <-- Relocations for .text
+--------------------+
|     .symtab        |  <-- Symbol table
+--------------------+
|     .strtab        |  <-- String table
+--------------------+
|  Section Headers   |
+--------------------+
```


## Inline Assembly

Inline `asm` blocks can contain portable IR or target-specific instructions.

### Portable Syntax

```mach
fun add_values(a: u64, b: u64) u64 {
    var result: u64 = 0;
    asm {
        add result, a, b
    }
    ret result;
}
```

Parsed and lowered through normal isel.

### ISA-Specific Syntax

```mach
fun syscall_exit(code: u64) {
    asm {
        x86_64 {
            mov rax, 60
            mov rdi, code
            syscall
        }
    }
}
```

Parsed by ISA-specific parser, emits target opcodes directly.


## Implementation

The code generation components are organized under `src/compiler/`:

| Component | Description |
|-----------|-------------|
| `masm/ir.mach` | IR opcode definitions, Inst/Operand/Block/Function records |
| `lower.mach` | AST to MASM IR lowering |
| `isa.mach` | ISA spec interface |
| `abi.mach` | ABI spec interface |
| `of.mach` | Object format spec interface |
| `of/elf/` | ELF emission |


## See Also

- [Overview](overview.md) - MASM architecture
- [IR Opcodes](ir.md) - Portable instructions
- [ABI](abi.md) - Calling conventions
- [Target Configuration](target.md) - Platform selection
