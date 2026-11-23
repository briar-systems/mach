[mach](../README.md) / [lower](README.md) / **placement**

# mach.lower.placement

placement: a module global `#[at(offset)]` places, as its object carries it to the link

a placed global is a section of its own, named for its offset, so the link plan that
places it reads the offset back from the object and needs no other record of it. the
lowering writes the name and the link reads it, and nothing else spells it

Source: [`src/lower/placement.mach`](../../../src/lower/placement.mach)

Uses types from [`mach.base.intern`](../base/intern.md).

| Functions | |
| :--- | :--- |
| [`section_name`](#section_name) | the name of the section a global placed at `offset` is, held by `itn` |
| [`offset_of`](#offset_of) | the offset the section named `name` places its global at, none for a section no `#[at]` names |

---

## Functions

### section_name

```mach
pub fun section_name(itn: *mach.base.intern.Interner, offset: u64) opt[str]
```

the name of the section a global placed at `offset` is, held by `itn`

### offset_of

```mach
pub fun offset_of(name: View) opt[u64]
```

the offset the section named `name` places its global at, none for a section no
`#[at]` names

