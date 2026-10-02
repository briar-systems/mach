# mach.lang.be.linker.unwind

## rec UnwindPlan

```mach
pub rec UnwindPlan;
```

what the unwind tables take from the link's foreign objects, gathered when
the tables are reserved and resolved once the layout is final

## fun init_unwind_plan

```mach
pub fun init_unwind_plan(plan: *UnwindPlan);
```

## fun free_unwind_plan

```mach
pub fun free_unwind_plan(alloc: *A.Allocator, plan: *UnwindPlan);
```

## fun claim_unwind_inputs

```mach
pub fun claim_unwind_inputs(tgt: *lang_target.Target, builds_tables: bool, modules: *target_of.ObjectImage, module_count: u32,
sec_base: *u32, placements: *Placement);
```

an image whose format builds unwind tables takes each foreign object's own
unwind input into them: its frame descriptions are placed in the frames
table, and its unwind index is read rather than placed. any other link
places those sections as the merge would

## fun reserve_unwind_tables

```mach
pub fun reserve_unwind_tables(s: *session.Session, tgt: *lang_target.Target,
modules: *target_of.ObjectImage, module_count: u32, sec_base: *u32, atoms: *AtomPlan,
merged: *MergedSection, groups: *SectionGroups, placements: *Placement, plan: *UnwindPlan) res[bool, fail.Fail];
```

reserves the format's unwind tables at the end of the merged code, before
anything is given an address: their size follows from the frame records of
the functions the link keeps and from what its foreign objects bring, and the
writer fills them once the layout is final. the foreign frame descriptions
are placed at the start of the frames table here, so the link relocates them
where they end up. true when a table was reserved, which moves everything
after the code

## fun resolve_native_unwind

```mach
pub fun resolve_native_unwind(s: *session.Session, plan: *UnwindPlan, modules: *target_of.ObjectImage,
placements: *Placement, sec_base: *u32, merged_to_out: *u32, out_img: *target_of.ObjectImage,
sym_locs: *map.Map[intern.StrId, SymbolLoc], dyn: *DynState, local_got: *LocalGotPlan,
format: *target_of.OfVTable, arch: *isa.IsaVTable, image_base: u64, atoms: *AtomPlan, count: *u32) res[*target_of.NativeUnwind, fail.Fail];
```

the functions the foreign unwind indexes describe, at their final addresses,
once the layout is final, with the slot each one's personality is read
through and the address of its lsda; an entry whose function the link
collected is dropped. `count` entries the caller frees

