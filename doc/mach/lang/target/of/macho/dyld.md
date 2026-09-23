# mach.lang.target.of.macho.dyld

## rec MachoDynLayout

```mach
pub rec MachoDynLayout;
```

## fun func_import_count

```mach
pub fun func_import_count(dyn: *of.DynamicInfo) u32;
```

## fun import_needs_got

```mach
pub fun import_needs_got(dyn: *of.DynamicInfo, import_index: u32) bool;
```

## fun got_import_count

```mach
pub fun got_import_count(dyn: *of.DynamicInfo) u32;
```

## fun macho_stub_size

```mach
pub fun macho_stub_size(arch_id: u32) usize;
```

## fun write_macho_stub

```mach
pub fun write_macho_stub(buf: *u8, arch_id: u32, stub_off: usize, stub_va: u64, slot_va: u64);
```

## fun dylib_ordinal_for

```mach
pub fun dylib_ordinal_for(dyn: *of.DynamicInfo, import_index: u32) u32;
```

## fun measure_bind_info

```mach
pub fun measure_bind_info(itn: *intern.Interner, dyn: *of.DynamicInfo,
segs: *of.LoadSegment, text_va: u64, pz: u32) usize;
```

## fun write_bind_info

```mach
pub fun write_bind_info(buf: *u8, off: usize, itn: *intern.Interner, dyn: *of.DynamicInfo,
segs: *of.LoadSegment, text_va: u64, pz: u32, got_seg_index: u32) usize;
```

## fun base_reloc_cmp

```mach
pub fun base_reloc_cmp(a: *of.BaseReloc, b: *of.BaseReloc) i64;
```

## fun measure_rebase_info

```mach
pub fun measure_rebase_info(dyn: *of.DynamicInfo, segs: *of.LoadSegment, text_va: u64, pz: u32) usize;
```

## fun write_rebase_info

```mach
pub fun write_rebase_info(buf: *u8, off: usize, dyn: *of.DynamicInfo, segs: *of.LoadSegment,
text_va: u64, pz: u32) usize;
```

## fun patch_macho_fixups

```mach
pub fun patch_macho_fixups(buf: *u8, arch_id: u32, lay: *MachoDynLayout, dyn: *of.DynamicInfo,
fixups: *of.PltFixup, fixup_count: u32, seg_offsets: *usize,
segs: *of.LoadSegment, seg_count: u32, nimp: u32) err[fail.Fail];
```

## fun patch_macho_import_addr_fixups

```mach
pub fun patch_macho_import_addr_fixups(buf: *u8, lay: *MachoDynLayout,
dyn: *of.DynamicInfo, seg_offsets: *usize,
segs: *of.LoadSegment,
seg_count: u32) err[fail.Fail];
```

## fun macho_got_ordinal

```mach
pub fun macho_got_ordinal(dyn: *of.DynamicInfo, import_index: u32) u32;
```

