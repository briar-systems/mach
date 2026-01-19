# Code Generation

This document describes how MASM IR is lowered to machine code.

## Overview

Code generation transforms portable MASM IR into target-specific machine code through several phases:

1. **Instruction Selection (isel)** - IR → target-specific opcodes
2. **Register Allocation** - Virtual registers → physical registers
3. **Encoding** - Opcodes → machine code bytes
4. **Object Emission** - Sections → ELF/PE/Mach-O file


## Pipeline

```
MASM IR Instructions
        │
        ▼
┌───────────────────┐
│  Instruction      │  Lower IR opcodes to target-specific opcodes
│  Selection (isel) │  e.g., MASM_IR_ADD → MASM_OP_X86_ADD_RR
└─────────┬─────────┘
          │
          ▼
┌───────────────────┐
│  Register         │  Map values to physical registers
│  Allocation       │  Handle spills to stack
└─────────┬─────────┘
          │
          ▼
┌───────────────────┐
│  Encoding         │  Generate machine code bytes
│                   │  e.g., ADD → 0x48 0x01 ...
└─────────┬─────────┘
          │
          ▼
┌───────────────────┐
│  Object Emission  │  Write ELF file with sections,
│                   │  symbols, and relocations
└───────────────────┘
```


## Instruction Selection

Instruction selection (isel) transforms platform-independent IR into target-specific opcodes.

### Entry Point

```c
void masm_x86_isel(struct Masm *masm);
```

### IR to x86_64 Mapping

| IR Opcode | x86_64 Opcode(s) |
|-----------|------------------|
| `MASM_IR_MOV` | `MASM_OP_X86_MOV_RR`, `MASM_OP_X86_MOV_RI` |
| `MASM_IR_ADD` | `MASM_OP_X86_ADD_RR`, `MASM_OP_X86_ADD_RI` |
| `MASM_IR_SUB` | `MASM_OP_X86_SUB_RR`, `MASM_OP_X86_SUB_RI` |
| `MASM_IR_MUL` | `MASM_OP_X86_IMUL_RR` |
| `MASM_IR_DIV` | `MASM_OP_X86_CQO` + `MASM_OP_X86_IDIV` |
| `MASM_IR_LOAD` | `MASM_OP_X86_MOV_RM`, `MASM_OP_X86_MOVQ` |
| `MASM_IR_STORE` | `MASM_OP_X86_MOV_MR` |
| `MASM_IR_SEQ` | `MASM_OP_X86_CMP_RR` + `MASM_OP_X86_SETE` |
| `MASM_IR_BEQ` | `MASM_OP_X86_CMP_RR` + `MASM_OP_X86_JE` |
| `MASM_IR_CALL` | `MASM_OP_X86_CALL_REL` |
| `MASM_IR_RET` | `MASM_OP_X86_RET` |
| `MASM_IR_SYSCALL` | `MASM_OP_X86_SYSCALL` |

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

**Integer Registers**:
```c
static uint32_t scratch_regs[] = {
    MASM_X86_RAX, MASM_X86_RCX, MASM_X86_RDX,
    MASM_X86_R8, MASM_X86_R9, MASM_X86_R10, MASM_X86_R11
};
```

**Reserved Registers**:
```c
static uint32_t reserved_regs[] = {
    MASM_X86_RSP, MASM_X86_RBP,  // stack/frame pointers
    MASM_X86_RBX, MASM_X86_R12,  // callee-saved
    MASM_X86_R13, MASM_X86_R14, MASM_X86_R15
};
```

### Register Role Helpers

The ISA spec provides role-based register access:

```c
MasmOperand (*reg_result)(uint8_t size);   // return value register
MasmOperand (*reg_tmp0)(uint8_t size);     // first temporary
MasmOperand (*reg_tmp1)(uint8_t size);     // second temporary
MasmOperand (*reg_arg)(int index, uint8_t size);  // argument register
MasmOperand (*reg_sp)(uint8_t size);       // stack pointer
MasmOperand (*reg_fp)(uint8_t size);       // frame pointer
```


## Encoding

Encoding transforms target-specific opcodes into machine code bytes.

### Entry Point

```c
int masm_x86_encode(MasmInstruction inst, uint8_t *buffer, size_t size);
```

Returns the number of bytes written.

### x86_64 Instruction Format

```
┌─────────┬─────────┬─────────┬─────────┬─────────┬─────────┐
│ Prefix  │   REX   │ Opcode  │ ModR/M  │   SIB   │ Disp/Imm│
│ (opt)   │ (opt)   │ 1-3 B   │ (opt)   │ (opt)   │ (opt)   │
└─────────┴─────────┴─────────┴─────────┴─────────┴─────────┘
```

### REX Prefix

For 64-bit operands and extended registers (R8-R15):

```c
uint8_t rex = 0x40;
rex |= 0x08;  // REX.W - 64-bit operand
rex |= 0x04;  // REX.R - extend ModR/M reg field
rex |= 0x02;  // REX.X - extend SIB index field
rex |= 0x01;  // REX.B - extend ModR/M r/m or SIB base
```

### ModR/M Byte

```c
uint8_t modrm(uint8_t mod, uint8_t reg, uint8_t rm) {
    return (mod << 6) | ((reg & 7) << 3) | (rm & 7);
}
```

| Mod | Description |
|-----|-------------|
| 00 | Memory, no displacement |
| 01 | Memory, 8-bit displacement |
| 10 | Memory, 32-bit displacement |
| 11 | Register direct |

### Example: `add rax, rbx`

```c
// add r64, r/m64: REX.W + 03 /r
emit_byte(0x48);  // REX.W
emit_byte(0x03);  // ADD opcode
emit_byte(0xC3);  // ModR/M: mod=11, reg=rax, rm=rbx
```


## Floating-Point Operations

Floating-point uses SSE instructions with XMM registers.

### Common SSE Instructions

| IR | x86_64 | Encoding |
|----|--------|----------|
| `fadd` | `addsd` | `F2 0F 58 /r` |
| `fsub` | `subsd` | `F2 0F 5C /r` |
| `fmul` | `mulsd` | `F2 0F 59 /r` |
| `fdiv` | `divsd` | `F2 0F 5E /r` |
| `load` (f64) | `movsd` | `F2 0F 10 /r` |
| `store` (f64) | `movsd` | `F2 0F 11 /r` |

### Type-Directed Selection

```c
if (is_float_type(operand_type)) {
    emit_sse_instruction(inst);
} else {
    emit_integer_instruction(inst);
}
```


## Relocations

References to symbols generate relocations in the object file.

### Relocation Types

| Type | Usage |
|------|-------|
| `R_X86_64_PC32` | 32-bit PC-relative (data references) |
| `R_X86_64_PLT32` | 32-bit PLT-relative (function calls) |
| `R_X86_64_32` | 32-bit absolute |
| `R_X86_64_64` | 64-bit absolute |

### Example: Function Call

```c
// emit call with relocation
emit_byte(0xE8);  // CALL rel32
emit_relocation(current_offset, symbol_name, R_X86_64_PLT32, -4);
emit_dword(0);    // placeholder, filled by linker
```


## Object Emission

Final output is an ELF object file.

### ELF Structure

```
┌──────────────────┐
│    ELF Header    │
├──────────────────┤
│  Section: .text  │ ← Encoded machine code
├──────────────────┤
│ Section: .rodata │ ← Constants, strings
├──────────────────┤
│  Section: .data  │ ← Initialized globals
├──────────────────┤
│  Section: .bss   │ ← Uninitialized globals
├──────────────────┤
│    .rela.text    │ ← Relocations for .text
├──────────────────┤
│     .symtab      │ ← Symbol table
├──────────────────┤
│     .strtab      │ ← String table
├──────────────────┤
│  Section Headers │
└──────────────────┘
```

### Entry Point

```c
int masm_emit_object(Masm *masm, const char *filename);
```


## Inline Assembly

Inline `masm` blocks can contain portable IR or target-specific instructions.

### Portable Syntax

```mach
fun add_values(a: u64, b: u64) u64 {
    var result: u64 = 0;
    masm {
        add result, a, b
    }
    ret result;
}
```

Parsed and lowered through normal isel.

### ISA-Specific Syntax

```mach
fun syscall_exit(code: u64) {
    masm {
        x86_64 {
            mov rax, 60
            mov rdi, code
            syscall
        }
    }
}
```

Parsed by ISA-specific parser, emits target opcodes directly.


## Implementation Files

| Component | Location |
|-----------|----------|
| ISA Spec Interface | `masm/isa/spec.h` |
| x86_64 Opcodes | `masm/isa/x86_64/x86_64.h` |
| x86_64 Isel | `masm/isa/x86_64/` |
| x86_64 Inline Asm | `masm/isa/x86_64/asm.h` |
| ELF Emission | `masm/of/elf.c` |
| Object Format Spec | `masm/of/spec.h` |


## Adding a New Backend

To add a new ISA (e.g., ARM64):

1. **Create opcode definitions** - `masm/isa/arm64/arm64.h`
   - Define `MasmArm64Opcode` enum
   - Add register definitions

2. **Implement ISA spec** - `masm/isa/arm64/arm64.c`
   - Register role functions
   - Instruction selection
   - Encoding

3. **Add opcode kind** - `masm/ir.h`
   - `MASM_OPCODE_ARM64` to `MasmOpcodeKind`

4. **Register ISA** - `masm/isa/spec.c`
   - Update `masm_isa_spec_select()`


## See Also

- [Overview](overview.md) - MASM architecture
- [IR Opcodes](ir.md) - Portable instructions
- [ABI](abi.md) - Calling conventions
- [Target Configuration](target.md) - Platform selection