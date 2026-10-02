# mach.lang.be.linker.emit

## fun collect_exports

```mach
pub fun collect_exports(s: *session.Session, modules: *target_of.ObjectImage, module_count: u32,
sym_locs: *map.Map[intern.StrId, SymbolLoc], surface: *ExportSurface, count: *u32) res[*target_of.ExportSym, fail.Fail];
```

## fun collect_exception_sections

```mach
pub fun collect_exception_sections(alloc: *std_allocator.Allocator, out_img: *target_of.ObjectImage,
count: *u32) res[*target_of.Section, fail.Fail];
```

## fun entry_vaddr

```mach
pub fun entry_vaddr(s: *session.Session, tgt: *lang_target.Target,
sym_locs: *map.Map[intern.StrId, SymbolLoc]) res[u64, fail.Fail];
```

## fun build_load_segments

```mach
pub fun build_load_segments(alloc: *std_allocator.Allocator, out_img: *target_of.ObjectImage, merged: *MergedSection,
groups: *SectionGroups, seg_count: *u32) res[*target_of.LoadSegment, fail.Fail];
```

## fun free_load_segments

```mach
pub fun free_load_segments(alloc: *std_allocator.Allocator, segs: *target_of.LoadSegment, count: u32);
```

## fun build_exec_functions

```mach
pub fun build_exec_functions(alloc: *std_allocator.Allocator, out_img: *target_of.ObjectImage,
segs: *target_of.LoadSegment, seg_count: u32, natives: *target_of.NativeUnwind, native_count: u32,
func_count: *u32) res[*target_of.ExecFunction, fail.Fail];
```

the image's functions, each with its frame record when it has one, and the
foreign functions an object's unwind index describes, merged by address

## fun build_symtab_entries

```mach
pub fun build_symtab_entries(alloc: *std_allocator.Allocator, out_img: *target_of.ObjectImage,
segs: *target_of.LoadSegment, seg_count: u32, surface: *ExportSurface,
entry_count: *u32) res[*target_of.SymtabEntry, fail.Fail];
```

## fun header_reserve_bytes

```mach
pub fun header_reserve_bytes(tgt: *lang_target.Target, pie: bool, shape: *target_of.HeaderShape) u64;
```

## fun measure_header_shape

```mach
pub fun measure_header_shape(s: *session.Session, out_img: *target_of.ObjectImage, merged: *MergedSection,
groups: *SectionGroups, dyn: *DynState, pie: bool,
shape: *target_of.HeaderShape);
```

