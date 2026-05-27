# mach.lang.target

Target description. Composes an [ISA](target/isa.md), an
[ABI](target/abi.md), an [OS](target/os.md), and an
[object format](target/of.md) into a single
[`Target`](#target) record consumed by the middle-end and the
backend. The driver builds a `Target` from the active
[`TargetEntry`](driver.md#targetentry) at the end of
[target selection](driver.md#target-selection); every code-generation
phase reads it through the [`Session`](session.md#session) rather
than reaching into platform-specific modules directly.

Source is `new/lang/target.mach` (currently empty — this spec is
the design intent; the id constants in [`target.isa`](target/isa.md)
and [`target.os`](target/os.md) are already in source and consumed
by [`driver`](driver.md)).

## Types

### `Target`

```mach
pub rec Target {
    os_id:   u32;
    arch_id: u32;
    abi_id:  u32;
    of_id:   u32;

    os:      *os.OsVTable;
    arch:    *isa.IsaVTable;
    abi:     *abi.AbiVTable;
    of:      *of.OfVTable;
}
```

| Field   | Type                                              | Description                                                |
|---------|---------------------------------------------------|------------------------------------------------------------|
| os_id   | `u32`                                             | One of [`os.OS_*`](target/os.md#constants).                |
| arch_id | `u32`                                             | One of [`isa.ARCH_*`](target/isa.md#constants).            |
| abi_id  | `u32`                                             | One of [`abi.ABI_*`](target/abi.md#constants).             |
| of_id   | `u32`                                             | One of [`of.OF_*`](target/of.md#constants).                |
| os      | [`*os.OsVTable`](target/os.md#osvtable)           | OS dispatch surface for the active OS.                     |
| arch    | [`*isa.IsaVTable`](target/isa.md#isavtable)       | ISA dispatch surface for the active architecture.          |
| abi     | [`*abi.AbiVTable`](target/abi.md#abivtable)       | ABI dispatch surface for the active calling convention.    |
| of      | [`*of.OfVTable`](target/of.md#ofvtable)           | Object-format dispatch surface for the active output.      |

The `*_id` fields are the comptime-visible identity (mirroring the
existing [`OS_*`](target/os.md#constants) /
[`ARCH_*`](target/isa.md#constants) values the driver already
materialises). The vtable pointers are the *runtime* dispatch surface
— the middle-end and backend never branch on `*_id`; they go through
the vtable. Architecture-derived properties live where they're
authoritative: pointer width and endianness are read through
[`target.arch.pointer_width`](target/isa.md#isavtable) /
[`target.arch.endianness`](target/isa.md#isavtable), not duplicated
on `Target`.

## Constants

```mach
pub val ENDIAN_LITTLE: u32 = 0;
pub val ENDIAN_BIG:    u32 = 1;
```

| Constant         | Value | Meaning                       |
|------------------|-------|-------------------------------|
| `ENDIAN_LITTLE`  | 0     | Little-endian target.         |
| `ENDIAN_BIG`     | 1     | Big-endian target.            |

## Functions

### `select`

```mach
pub fun select(os_id: u32, arch_id: u32) Result[Target, str]
```

Resolves a [`Target`](#target) from the driver's already-known
`os_id` + `arch_id` pair. Looks up the corresponding
[`OsVTable`](target/os.md#osvtable) /
[`IsaVTable`](target/isa.md#isavtable) from the per-platform
registries, picks the canonical
[`AbiVTable`](target/abi.md#abivtable) for that
(os, arch) pair (e.g. Linux+x86\_64 → SysV; Windows+x86\_64 →
Win64), and picks the canonical
[`OfVTable`](target/of.md#ofvtable) for that os (Linux → ELF;
Darwin → Mach-O; Windows → COFF). Allocates nothing — the four
vtables are looked up from the process-global registries and the
returned `Target` is a value composed of ids and borrowed vtable
pointers.

| Param   | Type         | Description                                          |
|---------|--------------|------------------------------------------------------|
| os_id   | `u32`        | One of [`os.OS_*`](target/os.md#constants).          |
| arch_id | `u32`        | One of [`isa.ARCH_*`](target/isa.md#constants).      |

Returns the populated [`Target`](#target), or an error string when
the (os, arch) pair has no canonical ABI/OF mapping. Consumers
that need `pointer_width` or `endianness` read them through
[`target.arch`](#target).

This is the sanctioned constructor; the driver replaces its
former flat `(target_os_id, target_arch_id, pointer_width)` triple
on [`Project`](driver.md#project) with the value returned here.

## Selection table

| OS         | Arch      | Canonical ABI                          | Canonical object format               |
|------------|-----------|----------------------------------------|---------------------------------------|
| Linux      | x86\_64   | [`sysv`](target/abi/sysv.md)           | [`elf`](target/of/elf.md)             |
| Linux      | aarch64   | [`sysv`](target/abi/sysv.md) (AAPCS variant) | [`elf`](target/of/elf.md)       |
| Darwin     | x86\_64   | [`sysv`](target/abi/sysv.md)           | [`macho`](target/of/macho.md)         |
| Darwin     | aarch64   | [`sysv`](target/abi/sysv.md) (AAPCS variant) | [`macho`](target/of/macho.md)   |
| Windows    | x86\_64   | [`win64`](target/abi/win64.md)         | [`coff`](target/of/coff.md)           |
| Windows    | aarch64   | [`win64`](target/abi/win64.md) (AAPCS64 variant) | [`coff`](target/of/coff.md) |

User-specified ABI / OF overrides (e.g. emitting ELF on Darwin for
freestanding builds) are deferred to a future `[targets.<name>]`
extension; the spec currently locks in the canonical mappings.

## Dependencies

`std.types.bool`, `std.types.size`, `std.types.string`,
`std.types.result`, `std.allocator`,
[`mach.lang.target.os`](target/os.md),
[`mach.lang.target.isa`](target/isa.md),
[`mach.lang.target.abi`](target/abi.md),
[`mach.lang.target.of`](target/of.md).
