# mach.lang.target.abi.win64

Microsoft x64 / Windows AArch64 calling convention. Implements
[`abi.AbiVTable`](../abi.md#abivtable) and registers itself at
compiler startup with [`abi.register`](../abi.md#register). The
canonical ABI on Windows for both x86\_64 and aarch64; the same id
([`ABI_WIN64`](../abi.md#constants)) covers both — the
[`classify`](#classification) and marshalling functions dispatch on
the active [`Target.arch_id`](../../target.md#target).

Source is `new/lang/target/abi/win64.mach` (currently empty).

## Identifying constants

- `id` = [`abi.ABI_WIN64`](../abi.md#constants).
- `name` = `"win64"`.
- `stack_align` = 16 bytes.
- `red_zone` = 0 (Windows ABIs do not have a red zone).
- **Shadow space**: callers reserve 32 bytes (x86\_64) on the stack
  for the first four register args, even though the values stay in
  registers. Encoded as a per-call adjustment on prologue, not as a
  classifier output.

## Classification

`classify(type) → ArgClass` for Windows is simpler than SysV:

- **x86\_64 Microsoft**: scalars ≤ 8 bytes pass in the next
  register; > 8 bytes pass by hidden pointer. No splitting of
  aggregates across registers.
- **AArch64 Windows**: matches AAPCS64 for the most part; HFA / HVA
  rules apply.

## Argument registers

| Arch      | Integer / pointer args | Float args             |
|-----------|------------------------|------------------------|
| x86\_64   | RCX, RDX, R8, R9       | XMM0–XMM3              |
| aarch64   | X0–X7                  | V0–V7                  |

Each integer / float register slot is **position-paired** on x86\_64:
the i-th argument uses either the i-th integer register *or* the
i-th SSE register, never advancing the other set.

## Return registers

| Arch      | Integer / pointer return | Float return | Large aggregate           |
|-----------|--------------------------|--------------|---------------------------|
| x86\_64   | RAX                      | XMM0         | Pointer arg in RCX (sret) |
| aarch64   | X0 (+ X1 for pairs)      | V0 (+ V1)    | Pointer arg in X8 (sret)  |

## Callee-saved registers

| Arch      | Callee-saved                                                          |
|-----------|-----------------------------------------------------------------------|
| x86\_64   | RBX, RBP, RDI, RSI, RSP, R12–R15, XMM6–XMM15                          |
| aarch64   | X19–X28, FP (X29), LR (X30) when used as a frame ptr                  |

## Dependencies

[`mach.lang.target.abi`](../abi.md),
[`mach.lang.target.isa`](../isa.md),
[`mach.lang.intern`](../../intern.md).
