# Code Generation

This document describes how MIR is lowered to machine code, using the x86_64 backend as a reference implementation. The compiler is designed to support multiple targets through pluggable backends.

# Code Generation

This document describes how MIR is lowered to machine code, using the x86_64 backend as a reference implementation. The compiler is designed to support multiple targets through pluggable backends.

## Target Selection

The code generator is selected based on the `MIRTarget` configuration:

```c
MIRTarget target = mir_target_native();  // Detect host platform
// Or explicitly:
MIRTarget target = mir_target_create(MIR_ISA_X86_64, MIR_ABI_SYSV64, 
                                     MIR_OS_LINUX, MIR_OF_ELF);
```

**Current Support:**
- ISA: x86_64
- ABI: System V AMD64
- OS: Linux
- Object Format: ELF

**Future Expansion:**
The architecture supports additional backends by implementing the same interfaces in separate files (e.g., `isa/arm64.c`, `abi/win64.c`, `of/pe.c`).

## x86_64 Backend Overview

The x86_64 backend (`boot/src/compiler/mir/codegen/x86_64.c`) translates MIR to x86_64 assembly, handling:
- Instruction selection
- Register allocation
- Stack frame management
- RIP-relative addressing
- ELF relocation generation

## Code Generation Pipeline

```
MIR Instructions
      ↓
Register Allocation  (map values → physical registers)
      ↓
Instruction Emission (generate machine code bytes)
      ↓
Relocation Tracking  (record symbol references)
      ↓
ELF Object File
```

## Register Allocation

See [ABI](abi.md) for calling convention details.

### Allocation Strategy

1. **Parameters** - Use ABI-designated registers
2. **Temporaries** - Allocate from available registers
3. **Spills** - Use stack slots if registers exhausted

```c
int x86_64_allocate_registers(X86_64_CodegenContext *ctx, MIRFunction *func) {
    // Map parameters to ABI registers
    // Allocate temporaries
    // Track value → register mapping
}
```

### Integer Registers

```c
static X86_64_Reg integer_param_regs[] = {
    X86_64_RDI,  // 1st parameter
    X86_64_RSI,  // 2nd parameter
    X86_64_RDX,  // 3rd parameter
    X86_64_RCX,  // 4th parameter
    X86_64_R8,   // 5th parameter
    X86_64_R9    // 6th parameter
};
```

### Float Registers

```c
static X86_64_Reg float_param_regs[] = {
    X86_64_XMM0,  // 1st float parameter
    X86_64_XMM1,  // 2nd float parameter
    X86_64_XMM2,  // 3rd float parameter
    // ... XMM3-XMM7
};
```

## Instruction Emission

### Integer Arithmetic

| MIR Opcode | x86_64 Instruction |
|------------|---------------------|
| `add` | `add dst, src` |
| `sub` | `sub dst, src` |
| `mul` | `imul dst, src` |
| `and` | `and dst, src` |
| `or` | `or dst, src` |
| `xor` | `xor dst, src` |

Example:
```c
void emit_add_reg_reg(X86_64_CodegenContext *ctx, X86_64_Reg dst, X86_64_Reg src) {
    uint8_t rex = 0x48; // REX.W for 64-bit
    if (dst >= X86_64_R8) rex |= 0x01; // REX.B
    if (src >= X86_64_R8) rex |= 0x04; // REX.R
    emit_byte(ctx, rex);
    emit_byte(ctx, 0x01); // ADD r/m64, r64
    emit_byte(ctx, modrm(dst, src));
}
```

### Floating-Point (SSE)

| MIR Opcode | x86_64 Instruction |
|------------|---------------------|
| `add` (f64) | `addsd dst, src` |
| `sub` (f64) | `subsd dst, src` |
| `mul` (f64) | `mulsd dst, src` |
| `div` (f64) | `divsd dst, src` |
| `mov` (f64) | `movsd dst, src` |

Example:
```c
void emit_addsd_reg_reg(X86_64_CodegenContext *ctx, X86_64_Reg dst, X86_64_Reg src) {
    emit_byte(ctx, 0xF2);  // ADDSD prefix
    // REX prefix if needed
    emit_byte(ctx, 0x0F);
    emit_byte(ctx, 0x58);  // ADDSD opcode
    emit_byte(ctx, modrm(dst, src));
}
```

### Type-Directed Selection

```c
void emit_instruction(X86_64_CodegenContext *ctx, MIRInst *inst) {
    switch (inst->op) {
    case MIR_OP_ADD:
        if (inst->type && type_is_float(inst->type)) {
            emit_addsd_reg_reg(ctx, dst, src);  // SSE
        } else {
            emit_add_reg_reg(ctx, dst, src);    // Integer
        }
        break;
    }
}
```

## Memory Operations

### Load

```c
case MIR_OP_LOAD:
    if (type_is_float(inst->type)) {
        emit_movsd_mem_reg(ctx, dst, offset);  // MOVSD for floats
    } else {
        emit_mov_mem_reg(ctx, dst, offset);    // MOV for integers
    }
```

### Store

```c
case MIR_OP_STORE:
    if (type_is_float(inst->type)) {
        emit_movsd_reg_mem(ctx, src, offset);  // MOVSD
    } else {
        emit_mov_reg_mem(ctx, src, offset);    // MOV
    }
```

### Stack Addressing

Local variables accessed via RBP offset:

```asm
movsd xmm0, [rbp - 8]   # Load local float
mov rax, [rbp - 16]     # Load local integer
```

## Global References

Globals use RIP-relative addressing:

```c
// Load global address
emit_lea_rip_rel(ctx, reg, "@global_name");

// Generate relocation
add_relocation(ctx, offset, "global_name", R_X86_64_PC32, -4);
```

Example assembly:
```asm
lea rax, [rip + __float_const_0]  # RIP-relative
movsd xmm0, [rax]                 # Load float constant
```

## Type Conversions

### Integer Extensions

```asm
movzx rax, al   # Zero-extend (ZEXT)
movsx rax, al   # Sign-extend (SEXT)
```

### Float Conversions

```asm
cvttsd2si rax, xmm0   # f64 → i64 (FPTOSI)
cvtsi2sd xmm0, rax    # i64 → f64 (SITOFP)
```

## Control Flow

### Unconditional Branch

```asm
jmp label   # BR opcode
```

### Conditional Branch

1. Compare operands: `cmp r1, r2`
2. Set condition code
3. Conditional jump: `je`, `jne`, `jl`, `jle`, `jg`, `jge`

```c
emit_cmp_reg_reg(ctx, src1, src2);
int cond = map_comparison_to_x86(inst->op);
emit_setcc(ctx, cond, result_reg);
```

### Function Calls

1. Save caller-saved registers (if needed)
2. Move arguments to parameter registers
3. Call: `call function`
4. Result in RAX (integer) or XMM0 (float)

## Stack Frame

### Prologue

```asm
push rbp
mov rbp, rsp
sub rsp, <locals_size>  # Allocate locals
```

### Epilogue

```asm
mov rsp, rbp
pop rbp
ret
```

## Relocations

The code generator tracks relocations for:
- Global variable references
- Function calls
- String literals
- Float constants

```c
typedef struct X86_64_Relocation {
    uint64_t offset;        // Offset in code
    char *symbol_name;      // Symbol being referenced
    int type;               // R_X86_64_PC32, R_X86_64_PLT32, etc.
    int64_t addend;         // Addend value
    struct X86_64_Relocation *next;
} X86_64_Relocation;
```

Example:
```c
void add_relocation(X86_64_CodegenContext *ctx, 
                   uint64_t offset,
                   const char *symbol,
                   int type,
                   int64_t addend);
```

## ELF Emission

Final object file generation (`boot/src/compiler/mir/of/elf.c`):

1. Create sections: `.text`, `.data`, `.rodata`, `.bss`
2. Write machine code to `.text`
3. Write global data to appropriate sections
4. Create symbol table
5. Add relocations
6. Generate ELF header and section headers
7. Write to file

## Example

### MIR Code

```mir
function add(i64, i64) -> i64 {
  block_0:
    %1 = param 0
    %2 = param 1
    %3 = add %1, %2
    ret %3
}
```

### Generated Assembly

```asm
add:
    push rbp
    mov rbp, rsp
    # %1 already in rdi (param 0)
    # %2 already in rsi (param 1)
    mov rax, rdi        # %3 = move param 0 to result
    add rax, rsi        # %3 = add param 1
    pop rbp
    ret
```

## Implementation Files

| Component | Location |
|-----------|----------|
| Codegen | `boot/src/compiler/mir/codegen/x86_64.c` |
| Register Allocation | `boot/src/compiler/mir/isa/x86_64.c` |
| ELF Emission | `boot/src/compiler/mir/of/elf.c` |
| Relocations | Tracked in codegen context |

## See Also

- [ABI](abi.md) - Calling conventions
- [Opcodes](opcodes.md) - MIR operations
- [Types](types.md) - Type-directed code generation
