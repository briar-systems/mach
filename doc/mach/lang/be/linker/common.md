# mach.lang.be.linker.common

## fun grow_cap

```mach
pub fun grow_cap[T](alloc: *std_allocator.Allocator, data: **T, cap: *u32, initial: u32) err[fail.Fail];
```

## fun is_loadable_kind

```mach
pub fun is_loadable_kind(kind: target_of.SectionKind) bool;
```

a kind outside the catalog is never loadable; every image here was validated
at entry, so such a kind never reaches this predicate

## fun fill_section_bases

```mach
pub fun fill_section_bases(modules: *target_of.ObjectImage, module_count: u32, sec_base: *u32) err[fail.Fail];
```

## fun add_u32_counts

```mach
pub fun add_u32_counts[Unit](left: u32, right: u32) res[u32, layout.CheckCause];
```

## fun total_sections

```mach
pub fun total_sections(modules: *target_of.ObjectImage, module_count: u32) res[u32, fail.Fail];
```

## fun total_symbols

```mach
pub fun total_symbols(modules: *target_of.ObjectImage, module_count: u32) res[u32, fail.Fail];
```

## fun fill_symbol_bases

```mach
pub fun fill_symbol_bases(modules: *target_of.ObjectImage, module_count: u32, sym_base: *u32) err[fail.Fail];
```

## fun atom_reloc_traits

```mach
pub fun atom_reloc_traits(r: *target_of.Relocation, section: *target_of.Section,
arch: *isa.IsaVTable,
codegen_image: bool) res[of_reloc.RelocTraits, of_reloc.RelocError];
```

## fun atom_field_bias

```mach
pub fun atom_field_bias(r: *target_of.Relocation, mode: of_reloc.RelocAddendMode, text_bias_min: i32,
text_bias_max: i32, source_text: bool, lo: *i64, hi: *i64);
```

the bias between a field-bias operand's addend and the address it reaches:
the exact distance to the instruction's end when the encoder recorded it,
otherwise the target's bound for a field in text, and none outside text

## fun atom_effective_offset

```mach
pub fun atom_effective_offset(base: u32, addend: i64, out: *u32) bool;
```

## rec ExportSurface

```mach
pub rec ExportSurface;
```

the names the link's objects asked it to export whoever defines them: the
union of every module's `fwd` re-exports. a hidden definition under one of
these names is on the surface after all

## fun export_surface_build

```mach
pub fun export_surface_build(s: *session.Session, modules: *target_of.ObjectImage,
module_count: u32) res[ExportSurface, fail.Fail];
```

## fun export_surface_free

```mach
pub fun export_surface_free(surface: *ExportSurface);
```

## fun export_surface_requests

```mach
pub fun export_surface_requests(surface: *ExportSurface, name: intern.StrId) bool;
```

## fun is_exported_symbol

```mach
pub fun is_exported_symbol(m: *target_of.ObjectImage, sym: *target_of.Symbol, surface: *ExportSurface) bool;
```

## fun any_exported_symbol

```mach
pub fun any_exported_symbol(modules: *target_of.ObjectImage, module_count: u32,
surface: *ExportSurface) bool;
```

## fun is_function_branch_kind

```mach
pub fun is_function_branch_kind(kind: target_of.RelocKind) bool;
```

## fun is_got_kind

```mach
pub fun is_got_kind(kind: target_of.RelocKind) bool;
```

## val COMPACT_ENTRY_SIZE

```mach
pub val COMPACT_ENTRY_SIZE:  u32 = 32
```

a mach-o `__compact_unwind` entry: the function start, its length and its
encoding, then the personality and the lsda, each a pointer its relocation fills

## val COMPACT_PERSONALITY

```mach
pub val COMPACT_PERSONALITY: u32 = 16
```

## val COMPACT_LSDA

```mach
pub val COMPACT_LSDA:        u32 = 24
```

## fun is_personality_reloc

```mach
pub fun is_personality_reloc(img: *target_of.ObjectImage, r: *target_of.Relocation) bool;
```

a relocation naming an unwind index entry's personality, which the image
reaches through a pointer slot as a GOT-kind relocation would

## fun named_message

```mach
pub fun named_message(s: *session.Session, prefix: str, name_id: intern.StrId, suffix: str, fallback: str) str;
```

## fun oor_section_message

```mach
pub fun oor_section_message(s: *session.Session, sym_name: intern.StrId, obj_name: intern.StrId) str;
```

## fun dup_message

```mach
pub fun dup_message(s: *session.Session, name: intern.StrId) str;
```

## fun join2

```mach
pub fun join2(s: *session.Session, a: str, b: str, prefix: str) str;
```

## fun join3

```mach
pub fun join3(s: *session.Session, a: str, b: str, c: str, prefix: str) str;
```

## fun lt_name

```mach
pub fun lt_name(s: *session.Session, name: str) intern.StrId;
```

