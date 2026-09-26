# mach.lang.be.linker.common

## fun grow_cap

```mach
pub fun grow_cap[T](alloc: *A.Allocator, data: **T, cap: *u32, initial: u32) err[fail.Fail];
```

## fun is_loadable_kind

```mach
pub fun is_loadable_kind(kind: of.SectionKind) bool;
```

a kind outside the catalog is never loadable; every image here was validated
at entry, so such a kind never reaches this predicate

## fun fill_section_bases

```mach
pub fun fill_section_bases(modules: *of.ObjectImage, module_count: u32, sec_base: *u32) err[fail.Fail];
```

## fun add_u32_counts

```mach
pub fun add_u32_counts[Unit](left: u32, right: u32) res[u32, layout.CheckCause];
```

## fun total_sections

```mach
pub fun total_sections(modules: *of.ObjectImage, module_count: u32) res[u32, fail.Fail];
```

## fun total_symbols

```mach
pub fun total_symbols(modules: *of.ObjectImage, module_count: u32) res[u32, fail.Fail];
```

## fun fill_symbol_bases

```mach
pub fun fill_symbol_bases(modules: *of.ObjectImage, module_count: u32, sym_base: *u32) err[fail.Fail];
```

## fun atom_reloc_traits

```mach
pub fun atom_reloc_traits(r: *of.Relocation, section: *of.Section,
arch: *isa.IsaVTable,
codegen_image: bool) res[rel.RelocTraits, rel.RelocError];
```

## fun atom_field_bias

```mach
pub fun atom_field_bias(r: *of.Relocation, mode: rel.RelocAddendMode, text_bias_min: i32,
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
pub fun export_surface_build(s: *session.Session, modules: *of.ObjectImage,
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
pub fun is_exported_symbol(m: *of.ObjectImage, sym: *of.Symbol, surface: *ExportSurface) bool;
```

## fun any_exported_symbol

```mach
pub fun any_exported_symbol(modules: *of.ObjectImage, module_count: u32,
surface: *ExportSurface) bool;
```

## fun is_function_branch_kind

```mach
pub fun is_function_branch_kind(kind: of.RelocKind) bool;
```

## fun is_got_kind

```mach
pub fun is_got_kind(kind: of.RelocKind) bool;
```

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

