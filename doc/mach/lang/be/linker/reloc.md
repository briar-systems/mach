# mach.lang.be.linker.reloc

## val DWARF_TOMBSTONE_ADDRESS

```mach
pub val DWARF_TOMBSTONE_ADDRESS: u64 = 0xFFFFFFFFFFFFFFFF
```

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
pub fun free_local_got_plan(alloc: *A.Allocator, plan: *LocalGotPlan);
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

## fun finalize_local_got_vaddrs

```mach
pub fun finalize_local_got_vaddrs(merged: *MergedSection, plan: *LocalGotPlan);
```

