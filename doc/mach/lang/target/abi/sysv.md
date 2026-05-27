# mach.lang.target.abi.sysv

System V AMD64 / AAPCS64 family calling convention. Implements
[`abi.AbiVTable`](../abi.md#abivtable) and registers itself at
compiler startup with [`abi.register`](../abi.md#register). The
canonical ABI on Linux, Darwin, and BSD for both x86\_64 and
aarch64; the same id ([`ABI_SYSV`](../abi.md#constants)) covers
both AMD64 SysV and AAPCS64 — the [`classify`](#classification) and
marshalling functions dispatch on the active
[`Target.arch_id`](../../target.md#target).

Source is `new/lang/target/abi/sysv.mach` (currently empty).

## Identifying constants

- `id` = [`abi.ABI_SYSV`](../abi.md#constants).
- `name` = `"sysv"`.
- `stack_align` = 16 bytes on AMD64; 16 bytes on AAPCS64.
- `red_zone` = 128 bytes on AMD64 SysV; 0 on AAPCS64.

## Classification

`classify(type) → ArgClass` partitions a logical type into the
per-arch class set:

- **AMD64 SysV**: INTEGER, SSE, SSEUP, X87, X87UP, COMPLEX\_X87,
  NO\_CLASS, MEMORY. Aggregates ≤ 16 bytes are recursively
  classified; > 16 bytes go in MEMORY.
- **AAPCS64**: composite types ≤ 16 bytes are passed in
  general-purpose register pairs; HFA / HVA types up to 4 fields are
  passed in floating-point register sets; the rest go on the stack.

The class set itself is opaque to the middle-end; what flows back
out is the per-call register / stack assignment.

## Argument registers

| Arch      | Integer / pointer args                                                | Float args                                              |
|-----------|-----------------------------------------------------------------------|---------------------------------------------------------|
| x86\_64   | RDI, RSI, RDX, RCX, R8, R9                                            | XMM0–XMM7                                               |
| aarch64   | X0–X7                                                                 | V0–V7                                                   |

Args spilling past the register set are pushed onto the stack
right-to-left.

## Return registers

| Arch      | Integer / pointer return | Float return       | Large aggregate           |
|-----------|--------------------------|--------------------|---------------------------|
| x86\_64   | RAX (+ RDX for pairs)    | XMM0 (+ XMM1)      | Pointer arg in RDI (sret) |
| aarch64   | X0 (+ X1 for pairs)      | V0 (+ V1)          | Pointer arg in X8 (sret)  |

## Callee-saved registers

| Arch      | Callee-saved                                          |
|-----------|-------------------------------------------------------|
| x86\_64   | RBX, RBP, R12–R15                                     |
| aarch64   | X19–X28, FP (X29), LR (X30) when used as a frame ptr  |

## Dependencies

[`mach.lang.target.abi`](../abi.md),
[`mach.lang.target.isa`](../isa.md),
[`mach.lang.intern`](../../intern.md).
