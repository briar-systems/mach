# Sections

This document describes section management in MASM.

## Overview

Sections are containers for code and data within a MASM module. They correspond to segments in the final object file and determine memory placement, permissions, and linker behavior.

Section management is defined in `src/compiler/masm/section.mach`.


## Section Kinds

| Kind | Value | Default Flags | Purpose |
|------|-------|---------------|---------|
| `SEC_TEXT` | 0 | `FLAG_ALLOC \| FLAG_EXEC` | Executable code |
| `SEC_DATA` | 1 | `FLAG_ALLOC \| FLAG_WRITE` | Initialized mutable data |
| `SEC_RODATA` | 2 | `FLAG_ALLOC` | Read-only data (constants) |
| `SEC_BSS` | 3 | `FLAG_ALLOC \| FLAG_WRITE` | Uninitialized data |
| `SEC_CUSTOM` | 4 | `FLAG_ALLOC` | User-defined sections |


## Section Flags

| Flag | Value | Description |
|------|-------|-------------|
| `FLAG_WRITE` | 1 | Writable at runtime |
| `FLAG_EXEC` | 2 | Executable |
| `FLAG_ALLOC` | 4 | Occupies memory at runtime |


## Section Structure

```mach
pub rec Section {
    name:        str;
    kind:        SectionKind;
    data:        *u8;         # raw byte buffer
    size:        usize;       # bytes written
    capacity:    usize;       # allocated capacity
    align:       usize;       # alignment requirement
    flags:       SectionFlags;
    relocs:      *Relocation;
    reloc_count: i32;
    reloc_cap:   i32;
}
```


## Creating Sections

```mach
use section: mach.compiler.masm.section;

val r: Result[*section.Section, str] = section.create(alloc, ".text", section.SEC_TEXT);
```

### Getting or Creating Sections

Within a MASM context (`masm.mach`), use the context helpers:

```mach
use masm: mach.compiler.masm;

# get existing section (returns nil if not found)
val text: *section.Section = masm.get_section(m, ".text");

# get or create section
val r: Result[*section.Section, str] =
    masm.get_or_create_section(m, ".data", section.SEC_DATA);
```


## Emitting Data

All sections use the same byte-oriented emit API.

### Byte Emission Functions

```mach
# append raw bytes
section.emit_bytes(alloc, sec, bytes_ptr, len)  Result[usize, str]

# single byte
section.emit_byte(alloc, sec, b)                Result[usize, str]

# little-endian integers
section.emit_u16(alloc, sec, v)                 Result[usize, str]
section.emit_u32(alloc, sec, v)                 Result[usize, str]
section.emit_u64(alloc, sec, v)                 Result[usize, str]
```

All emit functions return the byte offset at which the data was written (for use with relocations and symbol offsets).

### Example: String Literal in .rodata

```mach
val rodata_r: Result[*section.Section, str] =
    masm.get_or_create_section(m, ".rodata", section.SEC_RODATA);
val rodata: *section.Section = result_unwrap_ok[*section.Section, str](rodata_r);

# record offset before writing
val offset: usize = section.current_offset(rodata);

# write string bytes
val msg: str = "Hello";
section.emit_bytes(alloc, rodata, msg::*u8, 6);  # includes null terminator
```

### Example: Global Variable in .data

```mach
val data_r: Result[*section.Section, str] =
    masm.get_or_create_section(m, ".data", section.SEC_DATA);
val data: *section.Section = result_unwrap_ok[*section.Section, str](data_r);

val offset: usize = section.current_offset(data);
section.emit_u64(alloc, data, 42);
```


## Alignment

### Section Alignment

Set the required alignment for a section:

```mach
section.set_align(sec, 16);   # 16-byte alignment
```

### Common Alignments

| Type | Alignment |
|------|-----------|
| `u8`, `i8` | 1 byte |
| `u16`, `i16` | 2 bytes |
| `u32`, `i32`, `f32` | 4 bytes |
| `u64`, `i64`, `f64`, `ptr` | 8 bytes |


## Relocations

Relocation entries track symbol references that must be resolved at link time.

### Relocation Types

| Type | Value | Description |
|------|-------|-------------|
| `RELOC_NONE` | 0 | No relocation |
| `RELOC_ABS64` | 1 | 64-bit absolute address |
| `RELOC_REL32` | 2 | 32-bit PC-relative |
| `RELOC_CALL32` | 3 | 32-bit call (maps to PLT/stub) |
| `RELOC_ABS32` | 4 | 32-bit absolute (zero-extended) |
| `RELOC_ABS32S` | 5 | 32-bit absolute (sign-extended) |

### Relocation Structure

```mach
pub rec Relocation {
    offset:      usize;     # byte offset within section
    symbol_name: str;       # name of the referenced symbol
    rtype:       RelocKind;
    addend:      i64;
}
```

### Adding Relocations

```mach
section.add_relocation(alloc, sec, offset, symbol_name, section.RELOC_REL32, addend)
```

### Querying Relocations

```mach
section.has_relocations(sec)          bool
section.get_relocation(sec, index)    *Relocation
```


## Patching

Overwrite a previously written value (e.g., to back-patch a branch target or call offset):

```mach
section.patch_i32(sec, offset, value)    # overwrite 4 bytes at offset
section.patch_i64(sec, offset, value)    # overwrite 8 bytes at offset
```


## Query Helpers

```mach
section.current_offset(sec)    usize   # bytes written so far
section.is_text(sec)           bool
section.is_data(sec)           bool
section.is_rodata(sec)         bool
section.is_bss(sec)            bool
```


## Cleanup

```mach
section.destroy(alloc, sec)
```


## Section Layout

During object file emission, sections are laid out in a standard order:

```
┌──────────────────┐
│    ELF Header    │
├──────────────────┤
│  Program Headers │
├──────────────────┤
│      .text       │ ← Executable code
├──────────────────┤
│     .rodata      │ ← Read-only constants
├──────────────────┤
│      .data       │ ← Initialized globals
├──────────────────┤
│      .bss        │ ← Uninitialized globals (no file space)
├──────────────────┤
│   Symbol Table   │
├──────────────────┤
│   String Table   │
├──────────────────┤
│  Section Headers │
└──────────────────┘
```


## See Also

- [Symbols](symbols.md) - Named locations in sections
- [IR Opcodes](ir.md) - Instructions stored in text sections
- [Code Generation](codegen.md) - How sections are emitted
