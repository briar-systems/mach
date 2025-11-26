# ABI (Application Binary Interface)

This document describes calling conventions used in MIR code generation. The compiler currently implements the System V AMD64 ABI but is designed to support multiple ABIs through the `MIRTargetABI` abstraction.

## System V AMD64 ABI

The Mach compiler targets the System V AMD64 ABI, which is the standard for Linux and other Unix-like systems on x86_64.

### Register Usage

| Register | Purpose | Preserved? |
|----------|---------|------------|
| RAX | Return value, syscall number | No |
| RBX | General purpose | **Yes** |
| RCX | 4th integer argument | No |
| RDX | 3rd integer argument, syscall arg 2 | No |
| RSI | 2nd integer argument, syscall arg 1 | No |
| RDI | 1st integer argument, syscall arg 0 | No |
| RBP | Frame pointer | **Yes** |
| RSP | Stack pointer | **Yes** |
| R8 | 5th integer argument, syscall arg 4 | No |
| R9 | 6th integer argument, syscall arg 5 | No |
| R10 | Temporary, syscall arg 3 | No |
| R11 | Temporary | No |
| R12-R15 | General purpose | **Yes** |

### Floating-Point Registers

| Register | Purpose | Preserved? |
|----------|---------|------------|
| XMM0 | 1st float argument, return value | No |
| XMM1 | 2nd float argument | No |
| XMM2-XMM7 | 3rd-8th float arguments | No |
| XMM8-XMM15 | Temporary | No |

## Parameter Passing

### Integer and Pointer Parameters

First 6 parameters in registers, rest on stack:

| Parameter | Register |
|-----------|----------|
| 1st | RDI |
| 2nd | RSI |
| 3rd | RDX |
| 4th | RCX |
| 5th | R8 |
| 6th | R9 |
| 7th+ | Stack (right-to-left) |

### Floating-Point Parameters

First 8 float parameters in XMM registers:

| Parameter | Register |
|-----------|----------|
| 1st | XMM0 |
| 2nd | XMM1 |
| 3rd-8th | XMM2-XMM7 |
| 9th+ | Stack |

### Mixed Parameters

Parameters are assigned sequentially to their respective register classes:

```mach
fun example(a: i64, b: f64, c: i64, d: f64) {
  # a → RDI (1st integer)
  # b → XMM0 (1st float)
  # c → RSI (2nd integer)
  # d → XMM1 (2nd float)
}
```

## Return Values

| Type | Register |
|------|----------|
| Integer, Pointer | RAX |
| Float (f32, f64) | XMM0 |
| Struct (≤16 bytes) | RAX + RDX |
| Struct (>16 bytes) | Memory (pointer in RDI) |

## Stack Frame

The stack frame layout for a function:

```
Higher addresses
┌─────────────────┐
│  Return address │ ← Pushed by CALL
├─────────────────┤
│  Saved RBP      │ ← Pushed by prologue
├─────────────────┤ ← RBP points here
│  Local var 1    │ 
├─────────────────┤
│  Local var 2    │
├─────────────────┤
│  ...            │
├─────────────────┤
│  Spilled regs   │
└─────────────────┘ ← RSP points here
Lower addresses
```

### Stack Alignment

The stack must be 16-byte aligned before a `call` instruction. This is maintained by:
- Each function aligns its stack in the prologue
- Even number of pushes before calls

## Function Prologue

```asm
push rbp           # Save old frame pointer
mov rbp, rsp       # Set up new frame pointer
sub rsp, <size>    # Allocate local variables
```

## Function Epilogue

```asm
mov rsp, rbp       # Restore stack pointer
pop rbp            # Restore old frame pointer
ret                # Return to caller
```

## Syscalls

Linux syscalls use a different convention:

| Argument | Register |
|----------|----------|
| Syscall number | RAX |
| 1st arg | RDI |
| 2nd arg | RSI |
| 3rd arg | RDX |
| 4th arg | R10 (**not RCX**) |
| 5th arg | R8 |
| 6th arg | R9 |

Invoked with `syscall` instruction. Return value in RAX.

### Example

```mir
# exit(42)
syscall(60, 42)
```

Generates:
```asm
mov rax, 60        # SYS_exit
mov rdi, 42        # exit code
syscall
```

## Register Allocation

The register allocator in `boot/src/compiler/mir/codegen/x86_64.c` follows the ABI:

### Parameter Mapping

```c
// Integer parameter registers
static X86_64_Reg integer_param_regs[] = {
    X86_64_RDI, X86_64_RSI, X86_64_RDX,
    X86_64_RCX, X86_64_R8, X86_64_R9
};

// Float parameter registers  
static X86_64_Reg float_param_regs[] = {
    X86_64_XMM0, X86_64_XMM1, X86_64_XMM2, X86_64_XMM3,
    X86_64_XMM4, X86_64_XMM5, X86_64_XMM6, X86_64_XMM7
};
```

### Temporary Allocation

After parameters, temporaries are allocated to available registers:
- Integers: RAX, RBX, R10-R15
- Floats: XMM8-XMM15

## Struct Returns

Small structs (≤16 bytes) are returned in registers:

| Size | Registers |
|------|-----------|
| 1-8 bytes | RAX |
| 9-16 bytes | RAX + RDX |

Large structs (>16 bytes) use hidden pointer:
- Caller allocates space
- Pointer passed in RDI
- Callee fills the memory
- Returns same pointer in RAX

## Caller vs. Callee Saved

### Caller-Saved (Volatile)
Caller must save if needed across calls:
- RAX, RCX, RDX, RSI, RDI, R8-R11
- XMM0-XMM15

### Callee-Saved (Non-Volatile)
Callee must preserve:
- RBX, RBP, RSP, R12-R15

Currently not used by register allocator (simple allocation doesn't need preservation).

## Implementation

ABI logic is implemented in:
- `boot/src/compiler/mir/abi/sysv64.c` - System V ABI specifics
- `boot/src/compiler/mir/codegen/x86_64.c` - Register allocation
- `boot/src/compiler/mir/isa/x86_64.c` - Register definitions

## See Also

- [Functions](functions.md) - Function structure
- [Codegen](codegen.md) - Code generation details
- [Values](values.md) - Register allocation
