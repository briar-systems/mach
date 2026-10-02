# mach.lang.be.linker.gc

## rec LinkRoots

```mach
pub rec LinkRoots;
```

what the image must keep regardless of references

entry: the entry symbol, or STR_NIL for an image without one
exports: every exported definition is kept, for a shared object
names: further definitions the caller reaches by name
name_count: number of names

## rec AtomWinner

```mach
pub rec AtomWinner;
```

## rec AtomRelocRef

```mach
pub rec AtomRelocRef;
```

## rec DeadRange

```mach
pub rec DeadRange;
```

a byte range of one flattened section that nothing live reaches

## fun reloc_ref_lower

```mach
pub fun reloc_ref_lower(refs: *AtomRelocRef, count: u32, section: u32) u32;
```

## fun add_fallback_winners

```mach
pub fun add_fallback_winners(modules: *target_of.ObjectImage, module_count: u32,
winners: *map.Map[intern.StrId, AtomWinner]) err[fail.Fail];
```

a name no object defines binds to the fallback of the first COFF weak
external naming it, as the address resolution binds it, so that weak
external stands as the name's winner

## fun build_reloc_refs

```mach
pub fun build_reloc_refs(s: *session.Session, modules: *target_of.ObjectImage, module_count: u32,
sec_base: *u32,
winners: *map.Map[intern.StrId, AtomWinner],
arch: *isa.IsaVTable, out_total: *u32) res[*AtomRelocRef, fail.Fail];
```

every relocation operand outside debug sections, bound to its definition and
sorted by the flattened section it lands in

## fun collect_dead_atoms

```mach
pub fun collect_dead_atoms(s: *session.Session, modules: *target_of.ObjectImage, module_count: u32,
sec_base: *u32, sec_total: u32,
winners: *map.Map[intern.StrId, AtomWinner],
refs: *AtomRelocRef, ref_total: u32, arch: *isa.IsaVTable,
roots: *LinkRoots, dead_count: *u32) res[*DeadRange, fail.Fail];
```

the byte ranges of a final image that no root reaches through relocations.
an unwind record, an associated section and a section group member are
reached with the function, owner or group they belong to, and a reference to
`__start_<name>` or `__stop_<name>` reaches every section called <name>

refs: the sorted reference table from build_reloc_refs
roots: what the image keeps regardless of references
dead_count: set to the number of ranges returned
ret: the dead ranges, nil when there are none

