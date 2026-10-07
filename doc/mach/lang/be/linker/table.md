# mach.lang.be.linker.table

table: the tables a format asks the linker to reserve before anything is
given an address, at the end of the code or at the end of the read-only data
the loader writes once. the format writes the table's bytes, and reads back
where it lies from the span the link hands it

## rec At

```mach
pub rec At;
```

where a reserved table lies: an offset into the merged section its placement
ends

## fun reserve

```mach
pub fun reserve(s: *session.Session, merged: *MergedSection, groups: *SectionGroups,
shape: *target_of.TableShape, flag: u32, overflow: str) res[At, fail.Fail];
```

reserves the table `shape` declares where it places it, its one section named
by the shape and flagged `flag`. a table at the end of the read-only data
extends a section of the same name that already ends it, as a GOT the link
synthesized for defined symbols does

overflow: why a table that takes its section past the maximum size is refused

## fun vaddr_of

```mach
pub fun vaddr_of(at: At, merged: *MergedSection) u64;
```

the address a reserved table starts at, once the layout is final

