# mach.lang.be.linker.emit

## fun collect_exports

```mach
pub fun collect_exports(s: *session.Session, modules: *of.ObjectImage, module_count: u32,
sym_locs: *map.Map[intern.StrId, SymbolLoc], surface: *ExportSurface, count: *u32) res[*of.ExportSym, fail.Fail];
```

## fun collect_exception_sections

```mach
pub fun collect_exception_sections(alloc: *A.Allocator, out_img: *of.ObjectImage,
count: *u32) res[*of.Section, fail.Fail];
```

## fun entry_vaddr

```mach
pub fun entry_vaddr(s: *session.Session, tgt: *target.Target,
sym_locs: *map.Map[intern.StrId, SymbolLoc]) res[u64, fail.Fail];
```

## fun build_load_segments

```mach
pub fun build_load_segments(alloc: *A.Allocator, out_img: *of.ObjectImage, merged: *MergedSection,
groups: *SectionGroups, seg_count: *u32) res[*of.LoadSegment, fail.Fail];
```

## fun free_load_segments

```mach
pub fun free_load_segments(alloc: *A.Allocator, segs: *of.LoadSegment, count: u32);
```

## fun build_exec_functions

```mach
pub fun build_exec_functions(alloc: *A.Allocator, out_img: *of.ObjectImage,
segs: *of.LoadSegment, seg_count: u32,
func_count: *u32) res[*of.ExecFunction, fail.Fail];
```

## fun build_symtab_entries

```mach
pub fun build_symtab_entries(alloc: *A.Allocator, out_img: *of.ObjectImage,
segs: *of.LoadSegment, seg_count: u32, surface: *ExportSurface,
entry_count: *u32) res[*of.SymtabEntry, fail.Fail];
```

## fun header_reserve_bytes

```mach
pub fun header_reserve_bytes(tgt: *target.Target, pie: bool, shape: *of.HeaderShape) u64;
```

## fun measure_header_shape

```mach
pub fun measure_header_shape(s: *session.Session, out_img: *of.ObjectImage, merged: *MergedSection,
groups: *SectionGroups, dyn: *DynState, pie: bool,
shape: *of.HeaderShape);
```

