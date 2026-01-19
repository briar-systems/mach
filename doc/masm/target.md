# Target Configuration

This document describes the MASM target configuration system, which controls how code is generated for different platforms.

## Overview

A MASM target is a combination of four independent components:

| Component | Purpose | Current Support |
|-----------|---------|-----------------|
| **ISA** | Instruction set architecture | x86_64 |
| **ABI** | Application binary interface | System V AMD64 |
| **OS** | Operating system | Linux |
| **OF** | Object file format | ELF |

Each component can be extended independently, allowing for flexible platform support.


## Target Structure

```c
typedef struct MasmTarget {
    MasmTargetISA isa;
    MasmTargetABI abi;
    MasmTargetOS  os;
    MasmTargetOF  of;
} MasmTarget;
```

### Creating Targets

**Native Target** - Detect host platform automatically:

```c
MasmTarget target = masm_target_native();
```

**Explicit Target** - Specify all components:

```c
MasmTarget target = masm_target_create(
    MASM_ISA_X86_64,
    MASM_ABI_SYSV64,
    MASM_OS_LINUX,
    MASM_OF_ELF
);
```


## ISA (Instruction Set Architecture)

The ISA defines the register set, instruction encoding, and machine-level operations.

### Supported ISAs

| Enum | Description |
|------|-------------|
| `MASM_ISA_X86_64` | AMD64 / Intel 64 |

### ISA Specification Interface

Each ISA provides a `MasmISASpec` with the following hooks:

```c
typedef struct MasmISASpec {
    // register role providers
    MasmOperand (*reg_result)(uint8_t size);
    MasmOperand (*reg_tmp0)(uint8_t size);
    MasmOperand (*reg_tmp1)(uint8_t size);
    MasmOperand (*reg_div_hi)(uint8_t size);
    MasmOperand (*reg_div_lo)(uint8_t size);
    MasmOperand (*reg_arg)(int index, uint8_t size);
    MasmOperand (*reg_sp)(uint8_t size);
    MasmOperand (*reg_fp)(uint8_t size);
    
    // register metadata
    uint32_t       reg_count;
    const uint32_t *scratch_regs;
    uint8_t        scratch_count;
    const uint32_t *reserved_regs;
    uint8_t        reserved_count;
    
    // opcode hooks
    uint32_t (*op_syscall)();
    
    // inline asm parsing
    MasmOperand (*parse_reg)(const char *name, uint8_t ptr_size);
    void (*parse_inline_asm)(struct MasmSection *section, 
                             const char *content, 
                             uint8_t ptr_size);
    
    // instruction selection
    void (*isel)(struct Masm *masm);
    
    // binary encoding
    int (*encode)(MasmInstruction inst, uint8_t *buffer, size_t size);
} MasmISASpec;
```

### x86_64 Registers

The x86_64 ISA defines 16 general-purpose registers:

| Register | ID | Purpose |
|----------|-----|---------|
| RAX | 0 | Return value, accumulator |
| RCX | 1 | 4th integer argument |
| RDX | 2 | 3rd integer argument |
| RBX | 3 | Callee-saved |
| RSP | 4 | Stack pointer |
| RBP | 5 | Frame pointer |
| RSI | 6 | 2nd integer argument |
| RDI | 7 | 1st integer argument |
| R8-R15 | 8-15 | Additional registers |

Plus XMM0-XMM15 for floating-point operations.


## ABI (Application Binary Interface)

The ABI defines calling conventions, parameter passing, and register usage rules.

### Supported ABIs

| Enum | Description |
|------|-------------|
| `MASM_ABI_SYSV64` | System V AMD64 (Linux, macOS, BSD) |

### ABI Specification Interface

```c
typedef struct MasmABISpec {
    uint8_t pointer_size;
    uint8_t stack_align;
    bool    has_red_zone;
    
    // integer argument registers
    const uint32_t *int_arg_regs;
    uint8_t        int_arg_count;
    
    // integer return registers
    const uint32_t *int_ret_regs;
    uint8_t        int_ret_count;
    
    // floating-point argument registers
    const uint32_t *float_arg_regs;
    uint8_t        float_arg_count;
    
    // floating-point return registers
    const uint32_t *float_ret_regs;
    uint8_t        float_ret_count;
} MasmABISpec;
```

### System V AMD64 Convention

**Integer Parameters** (in order):
1. RDI
2. RSI
3. RDX
4. RCX
5. R8
6. R9
7. Stack (right-to-left)

**Float Parameters** (in order):
1. XMM0-XMM7
2. Stack

**Return Values**:
- Integer: RAX (+ RDX for 128-bit)
- Float: XMM0

See [ABI](abi.md) for complete calling convention details.


## OS (Operating System)

The OS component defines system call conventions and platform-specific behaviors.

### Supported Operating Systems

| Enum | Description |
|------|-------------|
| `MASM_OS_LINUX` | Linux kernel |

### Linux Syscall Convention

Syscalls on Linux x86_64 use:
- RAX: syscall number
- RDI, RSI, RDX, R10, R8, R9: arguments
- Invoke with `syscall` instruction
- Return value in RAX


## OF (Object Format)

The object format defines the binary container structure for compiled code.

### Supported Object Formats

| Enum | Description |
|------|-------------|
| `MASM_OF_ELF` | Executable and Linkable Format |

### Object Format Specification Interface

```c
typedef struct MasmOFSpec {
    const char *name;
    int (*write_object)(Masm *masm, const char *filename);
} MasmOFSpec;
```

### ELF Sections

The ELF backend generates standard sections:

| Section | Purpose |
|---------|---------|
| `.text` | Executable code |
| `.data` | Initialized data |
| `.rodata` | Read-only data (constants, strings) |
| `.bss` | Uninitialized data |


## String Conversions

Utility functions for target component names:

```c
// to string
const char *masm_target_isa_name(MasmTargetISA isa);
const char *masm_target_abi_name(MasmTargetABI abi);
const char *masm_target_os_name(MasmTargetOS os);
const char *masm_target_of_name(MasmTargetOF of);

// from string
MasmTargetISA masm_target_isa_from_name(const char *name);
MasmTargetABI masm_target_abi_from_name(const char *name);
MasmTargetOS  masm_target_os_from_name(const char *name);
MasmTargetOF  masm_target_of_from_name(const char *name);
```


## Adding a New Target Component

### New ISA

1. Create header `masm/isa/<name>/<name>.h`:
   - Define opcode enum (`MasmXxxOpcode`)
   - Declare register helpers and encode function

2. Create implementation `masm/isa/<name>/<name>.c`:
   - Implement `MasmISASpec` interface
   - Provide isel (IR → target opcodes)
   - Provide encode (instruction → bytes)

3. Add enum value to `MasmTargetISA` in `masm/target.h`

4. Add opcode kind to `MasmOpcodeKind` in `masm/ir.h`

5. Register in `masm_isa_spec_select()` in `masm/isa/spec.c`

### New ABI

1. Create `masm/abi/<name>.c`:
   - Define argument/return register arrays
   - Implement `MasmABISpec` structure

2. Add enum value to `MasmTargetABI` in `masm/target.h`

3. Register in `masm_abi_spec_select()` in `masm/abi/spec.c`

### New Object Format

1. Create `masm/of/<name>.c`:
   - Implement `write_object()` function
   - Define `MasmOFSpec` structure

2. Add enum value to `MasmTargetOF` in `masm/target.h`

3. Register in `masm_of_spec_select()` in `masm/of/spec.c`


## See Also

- [Overview](overview.md) - MASM architecture
- [ABI](abi.md) - Calling convention details
- [Code Generation](codegen.md) - Instruction selection and encoding