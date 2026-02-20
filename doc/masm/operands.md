# Operands

This document describes the operand types used in MASM instructions.

## Overview

Operands are the inputs and outputs of MASM instructions. Each operand has a kind that determines how it is interpreted during code generation.


## Operand Kinds

| Kind | Value | Description | Example |
|------|-------|-------------|---------|
| `OPK_NONE` | 0 | No operand (unused slot) | - |
| `OPK_REG` | 1 | Physical or virtual register | `%rax`, `%v42` |
| `OPK_IMM` | 2 | Immediate constant | `42`, `-1` |
| `OPK_MEM` | 3 | Memory reference | `[FP - 8]` |
| `OPK_LABEL` | 4 | Code label reference | `.loop_start` |
| `OPK_SYM` | 5 | Symbol reference (by index) | `@my_function` |


## Operand Structure

Operands are flat records with a kind discriminant. Fields are interpreted based on `kind`:

```mach
pub rec Operand {
    kind:   ValueKind;    # discriminant (OPK_NONE, OPK_REG, etc.)
    size:   u8;           # operand size in bytes
    reg:    i32;          # register id (OPK_REG)
    imm:    i64;          # immediate value (OPK_IMM)
    base:   i32;          # base register for memory (OPK_MEM)
    index:  i32;          # index register for memory (-1 = none)
    scale:  i32;          # scale factor for memory (1, 2, 4, 8)
    disp:   i64;          # displacement for memory
    sym_id: u64;          # symbol index (OPK_SYM)
    label:  str;          # label name (OPK_LABEL)
}
```

Field interpretation by kind:

| Kind | Fields used |
|------|-------------|
| `OPK_REG` | `reg`, `size` |
| `OPK_IMM` | `imm`, `size` |
| `OPK_MEM` | `base`, `index`, `scale`, `disp`, `size` |
| `OPK_LABEL` | `label` |
| `OPK_SYM` | `sym_id` |


## Register Operands

Registers represent storage locations for values during execution.

### Physical vs. Virtual Registers

Register IDs below `VREG_START` (1024) are physical registers with ISA-specific numbering. IDs at or above `VREG_START` are virtual registers assigned by the lowering phase and resolved to physical registers during register allocation.

Two special virtual registers are defined:

| Register | Value | Description |
|----------|-------|-------------|
| `VREG_FP` | `VREG_START - 1` | Virtual frame pointer, resolved to physical FP by regalloc |
| `VREG_SP` | `VREG_START - 2` | Virtual stack pointer, resolved after frame size is known |

### Creating Register Operands

```mach
val reg: Operand = ir.make_reg(MASM_X86_RAX, 8);    # physical register
val vreg: Operand = ir.make_reg(vreg_id, 8);         # virtual register
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

```mach
val imm: Operand = ir.make_imm(42, 8);     # 64-bit immediate
val neg: Operand = ir.make_imm(-1, 4);     # 32-bit immediate
```

### Immediate Range

Immediates are stored as `i64` and may be constrained by the target instruction encoding:
- x86_64 most instructions: 32-bit sign-extended
- x86_64 `mov` to 64-bit register: full 64-bit immediate


## Memory Operands

Memory operands reference values in memory using base register, optional index register, scale, and displacement.

### Addressing Modes

**Simple**: `[base + disp]`
```mach
val mem: Operand = ir.make_mem(VREG_FP, -1, 1, -8, 8);
# equivalent to: [FP - 8]
```

**Full**: `[base + index * scale + disp]`
```mach
val mem: Operand = ir.make_mem(base_reg, index_reg, 4, 0, 8);
# equivalent to: [base + index * 4]
```

An `index` of `-1` means no index register (simple addressing).

### Common Patterns

| Pattern | Description | Example |
|---------|-------------|---------|
| `[FP - N]` | Local variable | Stack-allocated locals |
| `[SP + N]` | Stack argument | Outgoing call arguments |
| `[base + idx*scale]` | Array access | Indexed array element |


## Symbol Operands

Symbol operands reference named locations in the program (functions, global variables, string literals) by index into the symbol table.

### Creating Symbol Operands

```mach
val sym: Operand = ir.make_sym(symbol_index);
```

### Usage

Symbols are resolved during linking. In code generation, they typically become:
- RIP-relative addresses (x86_64)
- Relocations in the object file


## Label Operands

Label operands reference code locations within the current function for control flow.

### Creating Label Operands

```mach
val lbl: Operand = ir.make_label(".loop_start");
val exit: Operand = ir.make_label(".func_end");
```

### Label Naming

Labels are function-local. By convention:
- Start with `.` to indicate local scope
- Use descriptive names: `.for_body_N`, `.if_then_N`, `.return`


## Operand Constructors

| Constructor | Description |
|-------------|-------------|
| `ir.make_none()` | Create empty operand |
| `ir.make_reg(reg, size)` | Register (physical or virtual) |
| `ir.make_imm(imm, size)` | Immediate constant |
| `ir.make_mem(base, index, scale, disp, size)` | Memory reference |
| `ir.make_label(label)` | Code label |
| `ir.make_sym(sym_id)` | Symbol reference |


## Examples

### Load from Local Variable

```mach
# load value from [FP - 16]
val dst: Operand = ir.make_reg(vreg_id, 8);
val src: Operand = ir.make_mem(ir.VREG_FP, -1, 1, -16, 8);
ir.block_append(alloc, block, ir.OP_LOAD, dst, src, ir.make_none(), ir.CC_NONE);
```

### Conditional Branch

```mach
# compare and branch: if %a < %b goto .loop
val a: Operand = ir.make_reg(vreg_a, 8);
val b: Operand = ir.make_reg(vreg_b, 8);
val cond: Operand = ir.make_reg(vreg_cond, 1);
val target: Operand = ir.make_label(".loop");

ir.block_append(alloc, block, ir.OP_ICMP, cond, a, b, ir.CC_LT);
ir.block_append(alloc, block, ir.OP_BRCOND, target, cond, ir.make_none(), ir.CC_NONE);
```


## See Also

- [IR Opcodes](ir.md) - Instructions that use operands
- [Sections](sections.md) - Where instructions are stored
- [Code Generation](codegen.md) - How operands are encoded
