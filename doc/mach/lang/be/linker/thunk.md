# mach.lang.be.linker.thunk

## rec ThunkPlan

```mach
pub rec ThunkPlan;
```

## fun init_thunk_plan

```mach
pub fun init_thunk_plan(plan: *ThunkPlan);
```

## fun free_thunk_plan

```mach
pub fun free_thunk_plan(alloc: *A.Allocator, plan: *ThunkPlan);
```

## fun thunks_placed

```mach
pub fun thunks_placed(plan: *ThunkPlan) bool;
```

whether the link placed any thunk, so the code's layout moved

## fun plan_thunks

```mach
pub fun plan_thunks(s: *session.Session, arch: *isa.IsaVTable, plan: *ThunkPlan,
modules: *of.ObjectImage, module_count: u32, sec_total: u32,
merged: *MergedSection, placements: *Placement, sec_base: *u32,
atoms: *AtomPlan, groups: *SectionGroups,
sym_locs: *map.Map[intern.StrId, SymbolLoc], dyn: *DynState) err[fail.Fail];
```

plans and opens the islands a link's out-of-reach branches need. runs once
the code's contents and the call-stub table are laid out and before anything
reads an address the islands move; the caller lays the image out again when
thunks_placed says the code moved

## fun thunk_for

```mach
pub fun thunk_for(plan: *ThunkPlan, module: u32, reloc: u32) opt[u64];
```

the address of the thunk a branch relocation goes through, none when it
branches straight to its target

## fun write_thunks

```mach
pub fun write_thunks(s: *session.Session, plan: *ThunkPlan, out_img: *of.ObjectImage,
merged: *MergedSection, merged_to_out: *u32, dyn: *DynState,
arch: *isa.IsaVTable, image_base: u64) err[fail.Fail];
```

writes every thunk into the linked code once its addresses are final: the
template's fields resolve against the destination, or for an imported
function become call-site fixups the format binds to its stub

