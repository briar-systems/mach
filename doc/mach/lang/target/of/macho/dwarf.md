# mach.lang.target.of.macho.dwarf

## rec MachoDwarf

```mach
pub rec MachoDwarf;
```

## fun cmd_bytes

```mach
pub fun cmd_bytes(debug_count: u32) usize;
```

## fun plan

```mach
pub fun plan(alloc: *A.Allocator, fp: *of_plan.FilePlan, debug: *target_of.Section, debug_count: u32,
segment_align: usize) res[MachoDwarf, fail.Fail];
```

## fun write

```mach
pub fun write(buf: *u8, itn: *intern.Interner, fp: *of_plan.FilePlan, debug: *target_of.Section, debug_count: u32,
lc: usize, d: *MachoDwarf, vmaddr: u64) res[usize, fail.Fail];
```

## fun write_bytes

```mach
pub fun write_bytes(buf: *u8, fp: *of_plan.FilePlan, debug: *target_of.Section, debug_count: u32, d: *MachoDwarf);
```

## fun free

```mach
pub fun free(alloc: *A.Allocator, d: *MachoDwarf, debug_count: u32);
```

