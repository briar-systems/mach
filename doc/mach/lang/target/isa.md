# mach.lang.target.isa

Instruction set interface and id registry. Defines an
[`IsaVTable`](#isavtable) that every per-architecture implementation
under `target/isa/` registers, plus stable numeric ids consumed by
comptime predicates such as `$mach.arch.<name>.id` and
`$mach.build.target.arch.id`. The ids in [Constants](#constants) are
already in source; the vtable surface is the design intent the
implementations land against.

## Types

### `IsaVTable`

```mach
pub rec IsaVTable {
    id:            u32;
    name:          intern.StrId;
    pointer_width: u32;
    endianness:    u32;
    reg_classes:   *RegClass;
    reg_class_count: u32;
}
```

Uniform per-architecture interface. The middle-end / backend reach
into this vtable via [`Target.arch`](../target.md#target).

| Field           | Type                                          | Description                                                |
|-----------------|-----------------------------------------------|------------------------------------------------------------|
| id              | `u32`                                         | One of [`ARCH_*`](#constants) — self-identifying.          |
| name            | [`intern.StrId`](../intern.md#strid)          | Canonical name (`"x86_64"`, `"aarch64"`).                  |
| pointer_width   | `u32`                                         | Bytes.                                                     |
| endianness      | `u32`                                         | One of [`target.ENDIAN_*`](../target.md#constants).        |
| reg_classes     | [`*RegClass`](#regclass)                      | Register classes exposed for register allocation.          |
| reg_class_count | `u32`                                         | Number of entries in `reg_classes`.                        |

### `RegClass`

```mach
pub rec RegClass {
    name:      intern.StrId;
    bit_width: u32;
    count:     u32;
}
```

One register class (e.g. general-purpose 64-bit on x86\_64). The
backend's register allocator consumes the per-class counts and bit
widths; the class entries themselves do not enumerate individual
register names — those live in the platform impl.

| Field     | Type                                          | Description                                          |
|-----------|-----------------------------------------------|------------------------------------------------------|
| name      | [`intern.StrId`](../intern.md#strid)          | Class identifier (`"gpr"`, `"xmm"`).                 |
| bit_width | `u32`                                         | Width of each register in bits.                      |
| count     | `u32`                                         | Number of registers in the class.                    |

## Constants

```mach
pub val ARCH_UNKNOWN: u32 = 0;
pub val ARCH_X86_64:  u32 = 1;
pub val ARCH_AARCH64: u32 = 2;
```

Canonical architecture ids. Stable across compiler versions; user
code dispatches on them through `$mach.arch.<name>.id` and
`$mach.build.target.arch.id`.

| Constant         | Value | Meaning                                            |
|------------------|-------|----------------------------------------------------|
| `ARCH_UNKNOWN`   | 0     | Unrecognised architecture (default fallback).      |
| `ARCH_X86_64`    | 1     | x86\_64.                                           |
| `ARCH_AARCH64`   | 2     | AArch64 / ARM64.                                   |

## Functions

### `lookup`

```mach
pub fun lookup(arch_id: u32) Option[*IsaVTable]
```

Returns the registered [`IsaVTable`](#isavtable) for `arch_id`, or
`none` if no impl is linked into this build. Called by
[`target.select`](../target.md#select).

### `register`

```mach
pub fun register(vt: *IsaVTable)
```

Registers a platform impl's vtable in the ISA registry. Each
implementation file under [`target/isa/`](../target.md) self-registers
from its module-init function; the compiler imports every ISA impl it
supports, so each runs exactly once before `main`. The registry is
process-global, write-once at startup, read-only thereafter, and
never freed — it holds static platform data, not per-session state.

## Dependencies

`std.types.bool`, `std.types.option`, `std.types.size`,
[`mach.lang.intern`](../intern.md).
