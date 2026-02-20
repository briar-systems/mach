# ABI (Application Binary Interface)

This document describes calling conventions used in MASM code generation.

## Overview

The ABI defines how functions communicate: parameter passing, return values, register usage, and stack management. MASM abstracts ABI details through the `MasmABISpec` interface, enabling support for multiple platforms.


## Current Support

| ABI | Target | Description |
|-----|--------|-------------|
| System V AMD64 | Linux x86_64 | Standard Unix calling convention |


## System V AMD64 ABI

The default ABI for Linux, macOS, and BSD on x86_64.


### Register Usage

| Register | Purpose | Preserved? |
|----------|---------|------------|
| RAX | Return value, syscall number | No |
| RBX | General purpose | **Yes** |
| RCX | 4th integer argument | No |
| RDX | 3rd integer argument | No |
| RSI | 2nd integer argument | No |
| RDI | 1st integer argument | No |
| RBP | Frame pointer | **Yes** |
| RSP | Stack pointer | **Yes** |
| R8 | 5th integer argument | No |
| R9 | 6th integer argument | No |
| R10 | Temporary, syscall arg 4 | No |
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

First 6 integer/pointer arguments use registers:

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

First 8 float arguments use XMM registers:

| Parameter | Register |
|-----------|----------|
| 1st | XMM0 |
| 2nd | XMM1 |
| 3rd-8th | XMM2-XMM7 |
| 9th+ | Stack |


### Mixed Parameters

Integer and float parameters are assigned to their respective register classes independently:

```mach
fun example(a: i64, b: f64, c: i64, d: f64) {
    # a → RDI (1st integer)
    # b → XMM0 (1st float)
    # c → RSI (2nd integer)
    # d → XMM1 (2nd float)
}
```


## Return Values

| Type | Location |
|------|----------|
| Integer, Pointer | RAX |
| Float (f32, f64) | XMM0 |
| Struct (≤16 bytes) | RAX + RDX |
| Struct (>16 bytes) | Memory via hidden pointer |


## Stack Frame

### Layout

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

The stack must be 16-byte aligned before a `call` instruction.


### Function Prologue

```asm
push rbp           ; save old frame pointer
mov rbp, rsp       ; set up new frame pointer
sub rsp, <size>    ; allocate local variables
```


### Function Epilogue

```asm
mov rsp, rbp       ; restore stack pointer
pop rbp            ; restore old frame pointer
ret                ; return to caller
```


## Syscalls

Linux syscalls use a different convention than function calls:

| Argument | Register |
|----------|----------|
| Syscall number | RAX |
| 1st arg | RDI |
| 2nd arg | RSI |
| 3rd arg | RDX |
| 4th arg | R10 (**not RCX**) |
| 5th arg | R8 |
| 6th arg | R9 |

Invoked with the `syscall` instruction. Return value is placed in RAX.

### Example

```mach
# exit(42)
asm {
    mov rax, 60    ; SYS_exit
    mov rdi, 42    ; exit code
    syscall
}
```


## ABI Interface

The `abi.ABI` record (defined in `src/compiler/abi.mach`) provides:

- Integer and float argument register lists
- Return register specification
- Stack alignment, pointer size, and red zone flag

The ABI is selected automatically by `target.select()` based on the ISA and OS components of the target triple. Downstream code receives a composed `Target` record and queries it directly — there is no manual ABI selection.


## Red Zone

The System V AMD64 ABI defines a 128-byte "red zone" below RSP that is guaranteed not to be clobbered by signal handlers or interrupts. Leaf functions can use this space without adjusting RSP.


## Caller vs. Callee Saved

### Caller-Saved (Volatile)

Caller must save these if needed across calls:
- RAX, RCX, RDX, RSI, RDI, R8-R11
- XMM0-XMM15


### Callee-Saved (Non-Volatile)

Callee must preserve these before using:
- RBX, RBP, RSP, R12-R15


## Struct Return

### Small Structs (≤16 bytes)

Returned in registers:

| Size | Registers |
|------|-----------|
| 1-8 bytes | RAX |
| 9-16 bytes | RAX + RDX |

For small aggregates that are not 1/2/4/8 bytes (e.g., 3/5/6/7), MASM still treats them as by-value in registers to match the C ABI. The backend is responsible for safe byte-accurate moves, typically by chunking loads/stores into 4/2/1 byte pieces without over-reading or over-writing memory.


### Large Structs (>16 bytes)

Returned via hidden pointer:

1. Caller allocates space for the return value
2. Caller passes pointer in RDI (first argument slot)
3. Callee writes to that memory
4. Callee returns the pointer in RAX


## Variadic Functions

For variadic functions (`...`), the caller must:

1. Pass the number of XMM registers used in AL (lower 8 bits of RAX)
2. Pass arguments normally through registers and stack

```
# calling printf("x=%d, y=%f\n", 42, 3.14)
# AL = 1  (one XMM register used for 3.14)
```


## Adding a New ABI

To add a new ABI, implement the `abi.ABI` interface in a new module under `src/compiler/abi/` and register recognition in `target.select()`. See [Target Configuration](target.md) for the full extension workflow.


## See Also

- [Target Configuration](target.md) - Platform selection
- [Code Generation](codegen.md) - How ABI is applied
- [IR Opcodes](ir.md) - Call and syscall instructions