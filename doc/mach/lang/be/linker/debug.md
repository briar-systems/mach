# mach.lang.be.linker.debug

## rec StrMerge

```mach
pub rec StrMerge;
```

## fun collect_debug_sections

```mach
pub fun collect_debug_sections(alloc: *A.Allocator, out_img: *target_of.ObjectImage, count: *u32) res[*target_of.Section, fail.Fail];
```

## fun debug_abbrev_tables_equal

```mach
pub fun debug_abbrev_tables_equal(a: *target_of.Section, b: *target_of.Section) bool;
```

## fun module_debug_is_linkable

```mach
pub fun module_debug_is_linkable(modules: *target_of.ObjectImage, m: u32) bool;
```

## fun warn_debug_dropped

```mach
pub fun warn_debug_dropped(s: *session.Session, diags: *diagnostic.DiagnosticStore, name: intern.StrId);
```

record that a module's debug info is left out of the link, into the store the caller gathers

## fun discover_debug_names

```mach
pub fun discover_debug_names(alloc: *A.Allocator, modules: *target_of.ObjectImage, module_count: u32,
out_names: **intern.StrId, out_count: *u32) err[fail.Fail];
```

## fun build_str_merge

```mach
pub fun build_str_merge(s: *session.Session, modules: *target_of.ObjectImage, module_count: u32,
patching: bool) res[StrMerge, fail.Fail];
```

## fun free_str_merge

```mach
pub fun free_str_merge(sm: *StrMerge);
```

## fun debug_str_source_offset

```mach
pub fun debug_str_source_offset(target_offset: u32, addend: i64,
section_len: u32) res[u32, fail.Fail];
```

