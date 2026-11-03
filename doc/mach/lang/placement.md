# mach.lang.placement

placement: a module global `#[at(offset)]` places, as its object carries it to the link

a placed global is a section of its own, named for its offset, so the link plan that
places it reads the offset back from the object and needs no other record of it. the
lowering writes the name and the link reads it, and nothing else spells it

## fun section_name

```mach
pub fun section_name(itn: *intern.Interner, offset: u64) opt[str];
```

the name of the section a global placed at `offset` is, held by `itn`

## fun offset_of

```mach
pub fun offset_of(name: View) opt[u64];
```

the offset the section named `name` places its global at, none for a section no
`#[at]` names

