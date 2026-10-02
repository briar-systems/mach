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
pub fun patch_relocations(s: *session.Session, out_img: *target_of.ObjectImage,
modules: *target_of.ObjectImage, module_count: u32,
placements: *Placement, sec_base: *u32, merged_to_out: *u32,
sym_locs: *map.Map[intern.StrId, SymbolLoc],
dyn: *DynState, local_got: *LocalGotPlan,
strm: *StrMerge, format: *target_of.OfVTable,
arch: *isa.IsaVTable,
image_base: u64, atoms: *AtomPlan, thunks: *ThunkPlan) err[fail.Fail];
```

## fun relocation_target_vaddr

```mach
pub fun relocation_target_vaddr(s: *session.Session, modules: *target_of.ObjectImage, m: u32, ri: u32,
placements: *Placement, sec_base: *u32, merged_to_out: *u32, out_img: *target_of.ObjectImage,
sym_locs: *map.Map[intern.StrId, SymbolLoc], format: *target_of.OfVTable, arch: *isa.IsaVTable,
image_base: u64, atoms: *AtomPlan) res[opt[u64], fail.Fail];
```

where relocation `ri` of module `m` points once the layout is final: its
target's address plus its addend, none when the link collected the target or
defines no such symbol

## fun personality_slot

```mach
pub fun personality_slot(s: *session.Session, modules: *target_of.ObjectImage, m: u32, ri: u32,
placements: *Placement, sec_base: *u32, merged_to_out: *u32, out_img: *target_of.ObjectImage,
sym_locs: *map.Map[intern.StrId, SymbolLoc], dyn: *DynState, local_got: *LocalGotPlan,
format: *target_of.OfVTable, arch: *isa.IsaVTable, image_base: u64, atoms: *AtomPlan) res[target_of.UnwindPersonality, fail.Fail];
```

the pointer slot relocation `ri` of module `m` reaches a personality through:
the local GOT slot of a symbol the link defines, filled with its address, or
the import GOT slot of one the loader binds

## fun init_local_got_plan

```mach
pub fun init_local_got_plan(plan: *LocalGotPlan);
```

## fun free_local_got_plan

```mach
pub fun free_local_got_plan(alloc: *std_allocator.Allocator, plan: *LocalGotPlan);
```

## fun build_local_got_plan

```mach
pub fun build_local_got_plan(s: *session.Session, arch: *isa.IsaVTable,
modules: *target_of.ObjectImage,
module_count: u32, sym_locs: *map.Map[intern.StrId, SymbolLoc],
sec_base: *u32, atoms: *AtomPlan,
merged: *MergedSection, groups: *SectionGroups,
plan: *LocalGotPlan) err[fail.Fail];
```

## fun fill_local_got_slot

```mach
pub fun fill_local_got_slot(s: *session.Session, out_img: *target_of.ObjectImage, merged_to_out: *u32, dyn: *DynState,
plan: *LocalGotPlan, arch: *isa.IsaVTable, module: u32, symbol: u32, name: intern.StrId, local: bool,
vaddr: u64, absolute: bool) res[u32, fail.Fail];
```

the local GOT slot a defined symbol is read through, holding its address from
the first reference on and rebased when the image moves and the symbol does

## fun finalize_local_got_vaddrs

```mach
pub fun finalize_local_got_vaddrs(merged: *MergedSection, plan: *LocalGotPlan);
```

