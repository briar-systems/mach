# mach.lang.be.linker.reloc

## val DWARF_TOMBSTONE_ADDRESS

```mach
pub val DWARF_TOMBSTONE_ADDRESS: u64 = 0xFFFFFFFFFFFFFFFF
```

## val DWARF_LINE_TOMBSTONE_ADDRESS

```mach
pub val DWARF_LINE_TOMBSTONE_ADDRESS: u64 = 0
```

a dead line sequence starts at 0, as GNU ld and lld resolve it: a sequence begun at
the -1 tombstone advances past the top of the address space into real code, and
libbfd before 2.43 grows without bound decoding one (#3466)

## fun dwarf_tombstone_address

```mach
pub fun dwarf_tombstone_address(pointer_width: u32) u64;
```

the -1 tombstone at the width of the field it fills: all ones across a
32-bit code address, so a 32-bit target's absolute relocation takes it

## rec LocalGotPlan

```mach
pub rec LocalGotPlan;
```

## fun patch_relocations

```mach
pub fun patch_relocations(s: *session.Session, out_img: *of.ObjectImage,
modules: *of.ObjectImage, module_count: u32,
placements: *Placement, sec_base: *u32, merged_to_out: *u32,
sym_locs: *map.Map[intern.StrId, SymbolLoc],
dyn: *DynState, local_got: *LocalGotPlan,
strm: *StrMerge, format: *of.OfVTable,
arch: *isa.IsaVTable,
image_base: u64, atoms: *AtomPlan) err[fail.Fail];
```

## fun init_local_got_plan

```mach
pub fun init_local_got_plan(plan: *LocalGotPlan);
```

## fun free_local_got_plan

```mach
pub fun free_local_got_plan(alloc: *A.Allocator, plan: *LocalGotPlan);
```

## fun build_local_got_plan

```mach
pub fun build_local_got_plan(s: *session.Session, arch: *isa.IsaVTable,
modules: *of.ObjectImage,
module_count: u32, sym_locs: *map.Map[intern.StrId, SymbolLoc],
sec_base: *u32, atoms: *AtomPlan,
merged: *MergedSection, groups: *SectionGroups,
plan: *LocalGotPlan) err[fail.Fail];
```

## fun finalize_local_got_vaddrs

```mach
pub fun finalize_local_got_vaddrs(merged: *MergedSection, plan: *LocalGotPlan);
```

