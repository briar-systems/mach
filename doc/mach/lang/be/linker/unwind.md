# mach.lang.be.linker.unwind

## fun reserve_unwind_tables

```mach
pub fun reserve_unwind_tables(s: *session.Session, tgt: *target.Target,
modules: *of.ObjectImage, module_count: u32, sec_base: *u32, atoms: *AtomPlan,
merged: *MergedSection, groups: *SectionGroups) res[bool, fail.Fail];
```

reserves the format's unwind tables at the end of the merged code, before
anything is given an address: their size follows from the frame records of
the functions the link keeps, and the writer fills them once the layout is
final. true when a table was reserved, which moves everything after the code

