# mach.lang.target.isa.arm64

AArch64 / ARM64 instruction set. Implements
[`isa.IsaVTable`](../isa.md#isavtable) and registers itself at
compiler startup with [`isa.register`](../isa.md#register).

Source is `new/lang/target/isa/arm64.mach` (currently empty).

## Identifying constants

- `id` = [`isa.ARCH_AARCH64`](../isa.md#constants).
- `name` = `"aarch64"`.
- `pointer_width` = 8 bytes.
- `endianness` = [`target.ENDIAN_LITTLE`](../../target.md#constants).

## Register classes

| Class    | Bit width | Count | Notes                                                                  |
|----------|-----------|-------|------------------------------------------------------------------------|
| `gpr`    | 64        | 31    | X0–X30. X31 is context-sensitive (XZR / SP) and is not part of the class. |
| `vector` | 128       | 32    | V0–V31 (NEON / SIMD).                                                  |
| `flags`  | 64        | 1     | NZCV, modelled as a single register for clobber tracking.              |

`sp` and `fp` (X29) are reserved by the
[`frame`](../../be/codegen/frame.md) pass.

## Instruction encoding

The encoder lives in [`be.codegen.encode`](../../be/codegen/encode.md);
this vtable does not own emission. The canonical mapping from
[`MIR`](../../be/codegen/mir.md) opcodes to AArch64 instruction
forms is exposed as part of the
[isel](../../be/codegen/isel.md) dispatch table.

## Dependencies

[`mach.lang.target.isa`](../isa.md),
[`mach.lang.target`](../../target.md),
[`mach.lang.intern`](../../intern.md).
