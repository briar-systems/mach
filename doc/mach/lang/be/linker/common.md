# mach.lang.be.linker.common

## rec GmTrackEntry

```mach
pub rec GmTrackEntry;
```

## rec GmTracker

```mach
pub rec GmTracker;
```

## fun gm_track_allocate

```mach
pub fun gm_track_allocate(ctx: ptr, size: usize, align: usize) opt[ptr];
```

## fun gm_track_deallocate

```mach
pub fun gm_track_deallocate(ctx: ptr, p: ptr, size: usize, align: usize) i64;
```

## fun gm_track_reallocate

```mach
pub fun gm_track_reallocate(ctx: ptr, p: ptr, old_size: usize, new_size: usize, align: usize) opt[ptr];
```

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

## fun lt_image

```mach
pub fun lt_image(s: *session.Session, name: intern.StrId,
sections: *of.Section, section_count: u32,
symbols: *of.Symbol, symbol_count: u32,
relocations: *of.Relocation, reloc_count: u32) of.ObjectImage;
```

## rec GrowProbe

```mach
pub rec GrowProbe;
```

## fun grow_probe_allocate

```mach
pub fun grow_probe_allocate(ctx: ptr, size: usize, align: usize) opt[ptr];
```

## fun grow_probe_reallocate

```mach
pub fun grow_probe_reallocate(ctx: ptr, p: ptr, old_size: usize, new_size: usize, align: usize) opt[ptr];
```

## fun grow_probe_deallocate

```mach
pub fun grow_probe_deallocate(ctx: ptr, p: ptr, size: usize, align: usize) i64;
```

