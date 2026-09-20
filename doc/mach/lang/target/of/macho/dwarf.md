# mach.lang.target.of.macho.dwarf

## rec MachoDwarf

```mach
pub rec MachoDwarf;
```

## fun macho_dwarf_cmd_bytes

```mach
pub fun macho_dwarf_cmd_bytes(debug_count: u32) usize;
```

## fun macho_dwarf_plan

```mach
pub fun macho_dwarf_plan(alloc: *A.Allocator, fp: *plan.FilePlan, debug: *of.Section, debug_count: u32,
segment_align: usize) res[MachoDwarf, fail.Fail];
```

## fun macho_dwarf_write

```mach
pub fun macho_dwarf_write(buf: *u8, itn: *intern.Interner, fp: *plan.FilePlan, debug: *of.Section, debug_count: u32,
lc: usize, d: *MachoDwarf, vmaddr: u64) res[usize, fail.Fail];
```

## fun macho_dwarf_write_bytes

```mach
pub fun macho_dwarf_write_bytes(buf: *u8, fp: *plan.FilePlan, debug: *of.Section, debug_count: u32, d: *MachoDwarf);
```

## fun macho_dwarf_free

```mach
pub fun macho_dwarf_free(alloc: *A.Allocator, d: *MachoDwarf, debug_count: u32);
```

