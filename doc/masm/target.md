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

Target configuration is defined in `src/compiler/target.mach`.


## Target Structure

```mach
pub rec Target {
    name:       str;              # full triple: "x86_64-linux-elf"
    arch:       str;              # architecture component
    os_name:    str;              # OS component
    format:     str;              # object format component
    isa:        isa.ISA;          # instruction set interface
    abi:        abi.ABI;          # calling convention interface
    of:         of.ObjectFormat;  # object format interface (relocatable .o)
    ef:         of.ExecFormat;    # executable format interface (linked binary)
    obj_reader: of.ObjectReader;  # object reader interface (.o → ObjFile)
    os:         os.OS;            # operating system interface
    di:         di.DebugInfo;     # debug information interface
    ptr_size:   usize;            # pointer size in bytes
    ptr_align:  usize;            # pointer alignment in bytes
    endian:     u8;               # 0 = little, 1 = big
}
```

### Creating a Target

Targets are created by parsing a target triple of the form `"<arch>-<os>-<format>"`:

```mach
use target: mach.compiler.target;

val r: Result[target.Target, str] = target.select("x86_64-linux-elf", alloc);
```

This selects and composes all four interface implementations (ISA, ABI, OF, OS) automatically.


## Target Triples

The triple format is `"<arch>-<os>-<format>"`.

| Component | Supported Values |
|-----------|-----------------|
| arch | `x86_64` |
| os | `linux` |
| format | `elf` |

**Example**: `"x86_64-linux-elf"`


## ISA (Instruction Set Architecture)

The ISA defines the register set, instruction encoding, and machine-level operations.
It is selected automatically based on the `arch` component of the triple.

### ISA Interface

The `isa.ISA` record (defined in `src/compiler/isa.mach`) provides hooks for:

- Register role providers (`reg_result`, `reg_arg`, `reg_fp`, `reg_sp`, etc.)
- Register metadata (scratch registers, reserved registers)
- Instruction selection (`isel`)
- Binary encoding (`encode`)
- Inline assembly parsing

### x86_64 Registers

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

Plus XMM0-XMM15 for floating-point operations.


## ABI (Application Binary Interface)

The ABI defines calling conventions, parameter passing, and register usage rules.
It is selected automatically based on the ISA and OS.

### ABI Interface

The `abi.ABI` record (defined in `src/compiler/abi.mach`) provides:

- Integer and float argument register lists
- Return register specification
- Stack alignment and pointer size

### System V AMD64 Convention

| Parameters | Registers |
|-----------|-----------|
| Integer (1st–6th) | RDI, RSI, RDX, RCX, R8, R9 |
| Integer (7th+) | Stack (right-to-left) |
| Float (1st–8th) | XMM0–XMM7 |
| Float (9th+) | Stack |

| Return | Register |
|--------|----------|
| Integer / Pointer | RAX |
| Float | XMM0 |

See [ABI](abi.md) for complete calling convention details.


## OS (Operating System)

The OS component defines system call conventions and platform-specific behaviors.
It is selected automatically based on the `os` component of the triple.

### Linux Syscall Convention (x86_64)

| Role | Register |
|------|----------|
| Syscall number | RAX |
| 1st arg | RDI |
| 2nd arg | RSI |
| 3rd arg | RDX |
| 4th arg | R10 |
| 5th arg | R8 |
| 6th arg | R9 |

Invoked with the `syscall` instruction. Return value in RAX.


## OF (Object Format)

The object format defines the binary container structure for compiled code.
It is selected automatically based on the `format` component of the triple.

The `of.ObjectFormat` and `of.ExecFormat` interfaces are defined in `src/compiler/of.mach`.

### ELF Sections

The ELF backend generates standard sections:

| Section | Purpose |
|---------|---------|
| `.text` | Executable code |
| `.data` | Initialized data |
| `.rodata` | Read-only data (constants, strings) |
| `.bss` | Uninitialized data |


## Adding Platform Support

To add support for a new target (e.g., ARM64/Linux/ELF):

1. **ISA Implementation** — Create a new module under `src/compiler/isa/`:
   - Implement the `isa.ISA` interface (register roles, isel, encode)

2. **ABI Implementation** — Create a new module under `src/compiler/abi/`:
   - Implement the `abi.ABI` interface (argument registers, return registers)

3. **Register** — Add recognition in `target.select()` for the new arch/os/format strings

No changes are needed to the lowering phase, IR definitions, or MASM context — they are platform-independent.


## See Also

- [Overview](overview.md) - MASM architecture
- [ABI](abi.md) - Calling convention details
- [Code Generation](codegen.md) - Instruction selection and encoding
