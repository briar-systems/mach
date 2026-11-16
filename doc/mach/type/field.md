[mach](../README.md) / [type](README.md) / **field**

# mach.type.field

the field tables, tag discriminators and layout decorations of one type projection

a table is staged entry by entry and then published for its type. publishing over a table that
held other fields says so, since every fact read from the old one is then stale, and a reset of
the projection clears all of it.

<sub>source: [`src/type/field.mach`](../../../src/type/field.mach) · imported by 30 modules</sub>

<sub>`intern` is [`mach.base.intern`](../base/intern.md) · `deprecation` is [`mach.type.deprecation`](deprecation.md) · `A` is `std.allocator` · `handle` is [`mach.base.handle`](../base/handle.md) · `map` is `std.collections.map` · `fail` is [`mach.base.fail`](../base/fail.md)</sub>

| Type | | |
| :--- | :--- | :--- |
| [`Entry`](#entry) | `rec` |  |
| [`Decoration`](#decoration) | `rec` | the layout decorations a nominal declares: its alignment, zero when it declares none, and whether it is packed or volatile |
| [`Table`](#table) | `rec` |  |
| [`Projection`](#projection) | `rec` | the field tables of every aggregate, the declared discriminator of every tag and the layout decorations of every nominal, for one type projection; a projection reset clears all of it |

| On `Projection` | |
| :--- | :--- |
| [`projection_init`](#projection_init) · [`projection_dnit`](#projection_dnit) · [`projection_reset`](#projection_reset) | |
| [`stage`](#stage) | reserves room for `additional` entries and answers where the next staged run starts |
| [`entry_push`](#entry_push) | |
| [`publish`](#publish) | publish the staged run as the fields of `ty` |
| [`table_for`](#table_for) | the table published for `ty` |
| [`entry_at`](#entry_at) | entry `ix` of the published table `ft`, which holds more than `ix` entries |
| [`discriminator_of`](#discriminator_of) | the declared discriminator of tag nominal `nom`, TYPE_NIL when it has none |
| [`discriminator_set`](#discriminator_set) · [`discriminator_del`](#discriminator_del) | |
| [`decoration_of`](#decoration_of) | the decorations `ty` declares; none when it declares none |
| [`decoration_set`](#decoration_set) | |

---

## Entry

```mach
pub rec Entry {
    name: intern.StrId;
    ty:   TypeId;
    # a tag case's deprecation notice; absent on every record and union field
    deprecation: deprecation.Deprecation;
}
```

## Decoration

```mach
pub rec Decoration {
    align:     u32;
    packed:    bool;
    volatile_: bool;
}
```

the layout decorations a nominal declares: its alignment, zero when it declares
none, and whether it is packed or volatile

## Table

```mach
pub rec Table {
    ty:           TypeId;
    fields_start: u32;
    fields_len:   u32;
}
```

## Projection

```mach
pub rec Projection {
    a: *A.Allocator;

    # by type id, one past the index of its table in `tables`, 0 where it has none
    tables: handle.StableChunks[Table];
    index:  Vector[u32];
    pool:   handle.StableChunks[Entry];

    # the declared discriminator type per tag nominal, re-installed with the case table
    discriminators: map.Map[TypeId, TypeId];
    # the declared decorations per nominal, re-installed with its field table
    decorations: map.Map[TypeId, Decoration];
}
```

the field tables of every aggregate, the declared discriminator of every tag and
the layout decorations of every nominal, for one type projection; a projection
reset clears all of it

### projection_init

```mach
pub fun projection_init(a: *A.Allocator) Projection
```

### projection_dnit

```mach
pub fun projection_dnit(p: *Projection)
```

### projection_reset

```mach
pub fun projection_reset(p: *Projection)
```

### stage

```mach
pub fun stage(p: *Projection, additional: u32) res[u32, fail.Fail]
```

reserves room for `additional` entries and answers where the next staged run starts

### entry_push

```mach
pub fun entry_push(p: *Projection, fe: Entry) err[fail.Fail]
```

### publish

```mach
pub fun publish(p: *Projection, ty: TypeId, fields_start: u32, fields_len: u32) res[bool, fail.Fail]
```

publish the staged run as the fields of `ty`. true when `ty` already had a table and this
one holds other fields, which makes every fact read from the old table stale

### table_for

```mach
pub fun table_for(p: *Projection, ty: TypeId) opt[*Table]
```

the table published for `ty`

### entry_at

```mach
pub fun entry_at(p: *Projection, ft: *Table, ix: u32) *Entry
```

entry `ix` of the published table `ft`, which holds more than `ix` entries

### discriminator_of

```mach
pub fun discriminator_of(p: *Projection, nom: TypeId) TypeId
```

the declared discriminator of tag nominal `nom`, TYPE_NIL when it has none

### discriminator_set

```mach
pub fun discriminator_set(p: *Projection, nom: TypeId, disc: TypeId) err[fail.Fail]
```

### discriminator_del

```mach
pub fun discriminator_del(p: *Projection, nom: TypeId)
```

### decoration_of

```mach
pub fun decoration_of(p: *Projection, ty: TypeId) Decoration
```

the decorations `ty` declares; none when it declares none

### decoration_set

```mach
pub fun decoration_set(p: *Projection, ty: TypeId, d: Decoration) err[fail.Fail]
```

