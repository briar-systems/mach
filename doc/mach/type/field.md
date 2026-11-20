[mach](../README.md) / [type](README.md) / **field**

# mach.type.field

the field tables, tag discriminators and layout decorations of one type projection

a table is staged entry by entry and then published for its type. publishing over a table that
held other fields says so, since every fact read from the old one is then stale, and a reset of
the projection clears all of it.

a projection may be layered over a base, as its type table is (mach.type): it reads the base
without a lock, so the base takes nothing new while the layer lives. it borrows the base's
entries, numbering its own past them, so a table names its entries the same way in either, and
answers the base's tables, discriminators and decorations besides its own. a layer never
publishes over a table its base holds: it holds the same fields or the publish fails, since a
reset is the one way tables change and no layer lives across one

Source: [`src/type/field.mach`](../../../src/type/field.mach)

Uses types from [`mach.base.fail`](../base/fail.md), [`mach.base.handle`](../base/handle.md), [`mach.base.intern`](../base/intern.md), [`mach.type.deprecation`](deprecation.md).

| Type | | |
| :--- | :--- | :--- |
| [`Entry`](#entry) | `rec` | one field of a record or union, or one case of a tag |
| [`Decoration`](#decoration) | `rec` | the layout decorations a nominal declares: its alignment, zero when it declares none, and whether it is packed or volatile |
| [`Table`](#table) | `rec` | the published field table of one type |
| [`Projection`](#projection) | `rec` | the field tables of every aggregate, the declared discriminator of every tag and the layout decorations of every nominal, for one type projection; a projection reset clears all of it |

| On `Projection` | |
| :--- | :--- |
| [`projection_init`](#projection_init) | an empty projection |
| [`layer_init`](#layer_init) | a projection layered over `base`, which takes nothing new while it lives |
| [`projection_dnit`](#projection_dnit) | release everything a projection holds |
| [`projection_reset`](#projection_reset) | clear every table, staged entry, discriminator and decoration, keeping the projection usable |
| [`stage`](#stage) | reserves room for `additional` entries and answers where the next staged run starts |
| [`entry_push`](#entry_push) | stage one entry after the last |
| [`publish`](#publish) | publish the staged run as the fields of `ty` |
| [`table_for`](#table_for) | the table published for `ty`, by the projection or a projection under it |
| [`own_count`](#own_count) | how many tables the projection itself published, its base's not counted |
| [`own_at`](#own_at) | the table the projection itself published `at`th, its base's not counted |
| [`entry_at`](#entry_at) | entry `ix` of the published table `ft`, which holds more than `ix` entries |
| [`discriminator_of`](#discriminator_of) | the declared discriminator of tag nominal `nom`, TYPE_NIL when it has none |
| [`discriminator_set`](#discriminator_set) | record the declared discriminator of a tag nominal |
| [`discriminator_del`](#discriminator_del) | forget the declared discriminator of a tag nominal |
| [`decoration_of`](#decoration_of) | the decorations `ty` declares; none when it declares none |
| [`decoration_set`](#decoration_set) | record the layout decorations a nominal declares |
| [`fold`](#fold) | `layer` folded into its base, which took nothing new since the layer was made: its entries appended there in order keep their numbers, and its tables, discriminators and decorations are the base's after |

---

## Entry

```mach
pub rec Entry {
    name: mach.base.intern.StrId;
    ty:   TypeId;
    # a tag case's deprecation notice; absent on every record and union field
    deprecation: mach.type.deprecation.Deprecation;
}
```

one field of a record or union, or one case of a tag

| Field | Type | |
| :--- | :--- | :--- |
| `name` | `mach.base.intern.StrId` | its name |
| `ty` | `TypeId` | its type, TYPE_NIL for a tag case with no payload |
| `deprecation` | `mach.type.deprecation.Deprecation` | its deprecation notice |

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

the published field table of one type

| Field | Type | |
| :--- | :--- | :--- |
| `ty` | `TypeId` | the type it belongs to |
| `fields_start` | `u32` | where its entries start in the projection's pool |
| `fields_len` | `u32` | how many entries it holds |

## Projection

```mach
pub rec Projection {
    a: *std.allocator.Allocator;

    # the published tables, and in `index`, by type id, one past the index of its table in
    # `tables`, 0 where it has none: dense on a root, by type in `index_at` on a layer, whose
    # tables are few
    tables:   mach.base.handle.StableChunks[Table];
    index:    Vector[u32];
    index_at: std.collections.map.Map[TypeId, u32];
    pool:     mach.base.handle.StableChunks[Entry];

    # the projection this one is layered over, nil for a root, and how many entries it held then
    base:      *Projection;
    pool_base: u32;

    # the declared discriminator type per tag nominal, re-installed with the case table
    discriminators: std.collections.map.Map[TypeId, TypeId];
    # the declared decorations per nominal, re-installed with its field table
    decorations: std.collections.map.Map[TypeId, Decoration];
}
```

the field tables of every aggregate, the declared discriminator of every tag and
the layout decorations of every nominal, for one type projection; a projection
reset clears all of it

### projection_init

```mach
pub fun projection_init(a: *std.allocator.Allocator) Projection
```

an empty projection

| Parameter | |
| :--- | :--- |
| `a` | the allocator its tables are held in |
| **returns** | the projection, released by projection_dnit |

### layer_init

```mach
pub fun layer_init(a: *std.allocator.Allocator, base: *Projection) Projection
```

a projection layered over `base`, which takes nothing new while it lives

### projection_dnit

```mach
pub fun projection_dnit(p: *Projection)
```

release everything a projection holds

| Parameter | |
| :--- | :--- |
| `p` | the projection |

### projection_reset

```mach
pub fun projection_reset(p: *Projection)
```

clear every table, staged entry, discriminator and decoration, keeping the projection usable

| Parameter | |
| :--- | :--- |
| `p` | the projection |

### stage

```mach
pub fun stage(p: *Projection, additional: u32) res[u32, mach.base.fail.Fail]
```

reserves room for `additional` entries and answers where the next staged run starts

### entry_push

```mach
pub fun entry_push(p: *Projection, fe: Entry) err[mach.base.fail.Fail]
```

stage one entry after the last

| Parameter | |
| :--- | :--- |
| `p` | the projection |
| `fe` | the entry |
| **returns** | ok, or the failure growing the pool |

### publish

```mach
pub fun publish(p: *Projection, ty: TypeId, fields_start: u32, fields_len: u32) res[bool, mach.base.fail.Fail]
```

publish the staged run as the fields of `ty`. true when `ty` already had a table and this
one holds other fields, which makes every fact read from the old table stale

### table_for

```mach
pub fun table_for(p: *Projection, ty: TypeId) opt[*Table]
```

the table published for `ty`, by the projection or a projection under it

### own_count

```mach
pub fun own_count(p: *Projection) u32
```

how many tables the projection itself published, its base's not counted

### own_at

```mach
pub fun own_at(p: *Projection, at: u32) *Table
```

the table the projection itself published `at`th, its base's not counted

| Parameter | |
| :--- | :--- |
| `p` | the projection |
| `at` | the position among its own tables, below own_count |
| **returns** | the table |

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
pub fun discriminator_set(p: *Projection, nom: TypeId, disc: TypeId) err[mach.base.fail.Fail]
```

record the declared discriminator of a tag nominal

| Parameter | |
| :--- | :--- |
| `p` | the projection |
| `nom` | the tag nominal |
| `disc` | its discriminator type |
| **returns** | ok, or the failure growing the map |

### discriminator_del

```mach
pub fun discriminator_del(p: *Projection, nom: TypeId)
```

forget the declared discriminator of a tag nominal

| Parameter | |
| :--- | :--- |
| `p` | the projection |
| `nom` | the tag nominal |

### decoration_of

```mach
pub fun decoration_of(p: *Projection, ty: TypeId) Decoration
```

the decorations `ty` declares; none when it declares none

### decoration_set

```mach
pub fun decoration_set(p: *Projection, ty: TypeId, d: Decoration) err[mach.base.fail.Fail]
```

record the layout decorations a nominal declares

| Parameter | |
| :--- | :--- |
| `p` | the projection |
| `ty` | the nominal |
| `d` | its decorations |
| **returns** | ok, or the failure growing the map |

### fold

```mach
pub fun fold(layer: *Projection) err[mach.base.fail.Fail]
```

`layer` folded into its base, which took nothing new since the layer was made: its entries
appended there in order keep their numbers, and its tables, discriminators and decorations are
the base's after

