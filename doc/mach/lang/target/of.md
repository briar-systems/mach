# mach.lang.target.of

Object-format interface, object model, and id registry. Defines the
format-agnostic [`ObjectImage`](#objectimage) (sections, symbols,
relocations) that the backend produces and the per-format writers
consume; the [`OfVTable`](#ofvtable) writer interface every
implementation under `target/of/` registers; and stable numeric ids
selecting which output format the backend emits.

The object model lives here — rather than in [`be.obj`](../be/obj.md)
— because it is the *contract* between two layers: the backend builds
it, the format writers (`of.elf` / `of.macho` / `of.coff`) consume
it. Keeping it in `target.of` lets [`OfVTable.emit_object`](#ofvtable)
name [`ObjectImage`](#objectimage) directly without a `target.of` ↔
`be.obj` import cycle. Source is `new/lang/target/of.mach` (currently
empty — this spec is the design intent).

## Types

### `OfVTable`

```mach
pub rec OfVTable {
    id:               u32;
    name:             intern.StrId;
    file_extension:   intern.StrId;
    emit_object:      WriterFn;
    relocation_kinds: *RelocationKind;
    relocation_count: u32;
}
```

Uniform per-format interface. The backend's
[`linker`](../be/linker.md) and [`obj`](../be/obj.md) builder reach
into this vtable via [`Target.of`](../target.md#target).

| Field            | Type                                          | Description                                                                  |
|------------------|-----------------------------------------------|------------------------------------------------------------------------------|
| id               | `u32`                                         | One of [`OF_*`](#constants).                                                 |
| name             | [`intern.StrId`](../intern.md#strid)          | Canonical name (`"elf"`, `"macho"`, `"coff"`).                               |
| file_extension   | [`intern.StrId`](../intern.md#strid)          | Conventional extension (`"o"`, `"obj"`).                                     |
| emit_object      | [`WriterFn`](#writerfn)                       | The format-specific writer (see [Writer contract](#writer-contract)).        |
| relocation_kinds | [`*RelocationKind`](#relocationkind)          | Table of relocation forms supported by this format.                          |
| relocation_count | `u32`                                         | Number of entries in `relocation_kinds`.                                     |

### `WriterFn`

```mach
pub def WriterFn: fun(*isa.IsaVTable, *ObjectImage, zstr) Result[bool, str];
```

The signature every format writer shares — a real typed function
pointer, uniform across all formats. Takes the active architecture
(for the header's machine field, endianness, and relocation-type
mapping), the abstract [`ObjectImage`](#objectimage) to serialise,
and the output path. The writer takes
[`*isa.IsaVTable`](isa.md#isavtable) rather than
[`*target.Target`](../target.md#target) deliberately: naming
`Target` here would form a `target` ↔ `target.of` import cycle, and
the ISA vtable carries everything a writer needs.

### `RelocationKind`

```mach
pub rec RelocationKind {
    id:        u32;
    name:      intern.StrId;
    bit_width: u32;
    pc_rel:    bool;
}
```

One relocation form the object writer understands. The backend emits
abstract [`Relocation`](#relocation)s naming a kind; the writer
materialises them into the format-specific encoding.

| Field     | Type                                  | Description                                                |
|-----------|---------------------------------------|------------------------------------------------------------|
| id        | `u32`                                 | Format-private id (e.g. the value of `R_X86_64_PC32` on ELF). |
| name      | [`intern.StrId`](../intern.md#strid)  | Canonical short name — the handle the backend addresses relocations by. |
| bit_width | `u32`                                 | Width of the patched field in bits.                        |
| pc_rel    | `bool`                                | `true` if the relocation is PC-relative.                   |

### `ObjectImage`

```mach
pub rec ObjectImage {
    alloc:         *Allocator;
    name:          intern.StrId;
    sections:      *Section;
    section_count: u32;
    symbols:       *Symbol;
    symbol_count:  u32;
    relocations:   *Relocation;
    reloc_count:   u32;
}
```

One object file's contents in format-agnostic form. Built by the
backend via the [`be.obj`](../be/obj.md) builder; consumed by a
[`WriterFn`](#writerfn).

| Field         | Type                                          | Description                                          |
|---------------|-----------------------------------------------|------------------------------------------------------|
| alloc         | `*Allocator`                                  | Backing allocator.                                   |
| name          | [`intern.StrId`](../intern.md#strid)          | Module FQN (also the source file's basename, for the format-specific file name). |
| sections      | [`*Section`](#section)                        | Section table.                                       |
| section_count | `u32`                                         | Length of `sections`.                                |
| symbols       | [`*Symbol`](#symbol)                          | Symbol table.                                        |
| symbol_count  | `u32`                                         | Length of `symbols`.                                 |
| relocations   | [`*Relocation`](#relocation)                  | Flat relocation list — each entry names its section. |
| reloc_count   | `u32`                                         | Length of `relocations`.                             |

### `Section`

```mach
pub rec Section {
    name:  intern.StrId;
    kind:  SectionKind;
    bytes: *u8;
    len:   u32;
    align: u32;
}
```

| Field | Type                                          | Description                                          |
|-------|-----------------------------------------------|------------------------------------------------------|
| name  | [`intern.StrId`](../intern.md#strid)          | Canonical section name (`".text"`, `".data"`, ...).  |
| kind  | [`SectionKind`](#sectionkind)                 | Discriminator.                                       |
| bytes | `*u8`                                         | Section contents (`nil` for `SK_BSS`).               |
| len   | `u32`                                         | Length in bytes.                                     |
| align | `u32`                                         | Required section alignment in bytes.                 |

### `SectionKind`

```mach
pub def SectionKind: u8;
```

Discriminator for [`Section.kind`](#section). See [Constants](#constants).

### `Symbol`

```mach
pub rec Symbol {
    name:    intern.StrId;
    section: u32;
    offset:  u32;
    size:    u32;
    flags:   u32;
}
```

| Field   | Type                                          | Description                                                |
|---------|-----------------------------------------------|------------------------------------------------------------|
| name    | [`intern.StrId`](../intern.md#strid)          | Symbol name.                                               |
| section | `u32`                                         | Index into [`ObjectImage.sections`](#objectimage); `u32::max` for external (undefined) symbols. |
| offset  | `u32`                                         | Offset within the section.                                 |
| size    | `u32`                                         | Symbol size in bytes (0 for labels).                       |
| flags   | `u32`                                         | Bitfield of [`SYM_OBJ_FLAG_*`](#constants).                |

### `Relocation`

```mach
pub rec Relocation {
    section: u32;
    offset:  u32;
    kind:    intern.StrId;
    symbol:  intern.StrId;
    addend:  i64;
}
```

| Field   | Type                                          | Description                                          |
|---------|-----------------------------------------------|------------------------------------------------------|
| section | `u32`                                         | Index into [`ObjectImage.sections`](#objectimage).   |
| offset  | `u32`                                         | Byte offset of the patch within the section.         |
| kind    | [`intern.StrId`](../intern.md#strid)          | Relocation kind name — matched against [`OfVTable.relocation_kinds`](#ofvtable). |
| symbol  | [`intern.StrId`](../intern.md#strid)          | Target symbol name.                                  |
| addend  | `i64`                                         | Constant addend.                                     |

## Constants

```mach
pub val OF_UNKNOWN: u32 = 0;
pub val OF_ELF:     u32 = 1;
pub val OF_MACHO:   u32 = 2;
pub val OF_COFF:    u32 = 3;
```

| Constant       | Value | Meaning                                                |
|----------------|-------|--------------------------------------------------------|
| `OF_UNKNOWN`   | 0     | Unrecognised object format (default fallback).         |
| `OF_ELF`       | 1     | ELF (Linux, BSD, freestanding).                        |
| `OF_MACHO`     | 2     | Mach-O (macOS, iOS).                                   |
| `OF_COFF`      | 3     | PE/COFF (Windows).                                     |

```mach
pub val SK_TEXT:   SectionKind = 0;
pub val SK_DATA:   SectionKind = 1;
pub val SK_RODATA: SectionKind = 2;
pub val SK_BSS:    SectionKind = 3;
pub val SK_DEBUG:  SectionKind = 4;
```

[`SectionKind`](#sectionkind) values.

| Constant      | Value | Meaning                                                |
|---------------|-------|--------------------------------------------------------|
| `SK_TEXT`     | 0     | Executable code.                                       |
| `SK_DATA`     | 1     | Initialised data.                                      |
| `SK_RODATA`   | 2     | Read-only data.                                        |
| `SK_BSS`      | 3     | Uninitialised (zero-init) data; `bytes` is `nil`.      |
| `SK_DEBUG`    | 4     | Debug info (DWARF, CodeView).                          |

```mach
pub val SYM_OBJ_FLAG_GLOBAL:   u32 = 0x01;
pub val SYM_OBJ_FLAG_WEAK:     u32 = 0x02;
pub val SYM_OBJ_FLAG_EXTERN:   u32 = 0x04;
pub val SYM_OBJ_FLAG_FUNCTION: u32 = 0x08;
pub val SYM_OBJ_FLAG_OBJECT:   u32 = 0x10;
```

Bit flags stored on [`Symbol.flags`](#symbol).

| Constant                  | Meaning                                                |
|---------------------------|--------------------------------------------------------|
| `SYM_OBJ_FLAG_GLOBAL`     | Visible outside the object image.                      |
| `SYM_OBJ_FLAG_WEAK`       | Weak binding (overridable at link time).               |
| `SYM_OBJ_FLAG_EXTERN`     | Undefined in this image; resolved by the linker.       |
| `SYM_OBJ_FLAG_FUNCTION`   | Symbol names a function (text-section).                |
| `SYM_OBJ_FLAG_OBJECT`     | Symbol names a data object (data / bss / rodata).      |

## Writer contract

A [`WriterFn`](#writerfn) — `fun(*isa.IsaVTable, *ObjectImage, zstr)
Result[bool, str]` — is what every format implements. It serialises
the abstract [`ObjectImage`](#objectimage) into the format's bytes:
header, section table, symbol table, and relocation encoding. The
writer must not consult target state beyond the
[`IsaVTable`](isa.md#isavtable) it was handed.

The writer is invoked through [`be.obj.emit`](../be/obj.md#emit),
which looks the [`OfVTable`](#ofvtable) up from
[`Target.of`](../target.md#target) and calls `emit_object`.

## Functions

### `lookup`

```mach
pub fun lookup(of_id: u32) Option[*OfVTable]
```

Returns the registered [`OfVTable`](#ofvtable) for `of_id`, or
`none` if no impl is linked into this build. Called by
[`target.select`](../target.md#select).

### `register`

```mach
pub fun register(vt: *OfVTable)
```

Registers a platform impl's vtable in the object-format registry. Each
implementation file under [`target/of/`](../target.md) self-registers
from its module-init function; the compiler imports every format impl
it supports, so each runs exactly once before `main`. The registry is
process-global, write-once at startup, read-only thereafter, and
never freed — it holds static platform data, not per-session state.

## Dependencies

`std.types.bool`, `std.types.option`, `std.types.size`,
`std.types.string`, `std.types.zstr`, `std.types.result`,
`std.allocator`, [`mach.lang.intern`](../intern.md),
[`mach.lang.target.isa`](isa.md).
