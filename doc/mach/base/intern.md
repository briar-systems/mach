[mach](../README.md) / [base](README.md) / **intern**

# mach.base.intern

interned strings: one stable id per distinct byte string

a string may hold NUL bytes and is equal to another by its bytes and length. an interner may be
layered over a base: it answers every id the base had issued when it was made and issues its
own past them, and `child_reintern` maps a child's strings into its base.

a child is how work off the owner thread names strings. its contract:
- a child reads its base without a lock, so the base takes no new names while the child lives.
  a child with no base reads nothing shared and may live on any thread
- only the child's own thread adds to it, and its ids are its own until it is merged
- `child_reintern` merges it on the owner: each string the child issued is interned in the target
  in the order the child issued it, and the remap names the target's id for each of the child's.
  the target is the child's base or another child of that base, which the owner names strings in
  while the base is frozen and `fold` gives back to the base when it thaws
- so when the owner merges children at the points a single thread would have interned their
  strings, and in that order, every merged id is the id the single thread would have issued

Source: [`src/base/intern.mach`](../../../src/base/intern.mach)

| Type | | |
| :--- | :--- | :--- |
| [`StrId`](#strid) | `def` | the identity of one interned string |
| [`Interner`](#interner) | `rec` | a table of interned strings, each id naming one entry |
| [`Remap`](#remap) | `rec` | the ids a merged child's strings got in the merge's target |

| On `StrId` | |
| :--- | :--- |
| [`STR_NIL`](#strid) | |
| [`str_id`](#str_id) | the string id at `index` |
| [`str_index`](#str_index) | the index a string id names |
| [`str_same`](#str_same) | whether two ids name the same interned string |
| [`str_is_nil`](#str_is_nil) | whether an id is STR_NIL |

| On `Interner` | |
| :--- | :--- |
| [`init`](#init) | an empty interner with no base, released with dnit |
| [`child_init`](#child_init) | an interner layered over `base`: it answers every id `base` and its own bases had issued when it was made, and issues its own past them |
| [`descends`](#descends) | whether `itn` answers every id `base` has issued: it is `base`, or layered over it |
| [`dnit`](#dnit) | release an interner and its strings; a nil interner holds nothing |
| [`text`](#text) | the text of `id` |
| [`get`](#get) | the text of `id`, or none for an id the interner never issued |
| [`bytes`](#bytes) | every byte of `id`, NULs included, or none for an id the interner never issued |
| [`add`](#add) | the id of the text `text` up to its first NUL, interned when it was not yet |
| [`bytes_add`](#bytes_add) | the id of the `len` bytes at `data`, NULs included, interned when they were not yet |
| [`range_add`](#range_add) | the `len` bytes of `source` from `offset`, interned |
| [`find`](#find) | the id `text` was interned under, none when it never was; nothing is interned, so a reader that shares the interner may ask while no one adds |
| [`child_reintern`](#child_reintern) | `child` merged into `into`, its base, another child of its base or, for a child with no base, any interner: each string the child issued interned there in the order it issued them, and the ids they got |
| [`fold`](#fold) | `layer` folded into its base, which issued nothing since the layer was made: each string the layer issued moved there under the id it has, the layer left holding none |

| On `Remap` | |
| :--- | :--- |
| [`remap_dnit`](#remap_dnit) | release a remap; a nil remap holds nothing |
| [`remapped`](#remapped) | the id in the merge's target that `id`, issued by the merged child, names: an id below the child's own range is its base's and stays, one in it is mapped |

---

## StrId

```mach
pub def StrId: u32
```

the identity of one interned string

a plain `def` alias of `u32`, so the type system does not keep a string id
apart from another id or a bare integer. absence is `STR_NIL`, never an
`opt[StrId]`.

| Name | Value | |
| :--- | :--- | :--- |
| `STR_NIL` | `0xFFFFFFFF` |  |

### str_id

```mach
pub fun str_id(index: u32) StrId
```

the string id at `index`

### str_index

```mach
pub fun str_index(id: StrId) u32
```

the index a string id names

### str_same

```mach
pub fun str_same(left: StrId, right: StrId) bool
```

whether two ids name the same interned string

### str_is_nil

```mach
pub fun str_is_nil(id: StrId) bool
```

whether an id is STR_NIL

## Interner

```mach
pub rec Interner {
    a:        *std.allocator.Allocator;
    strings:  *Entry;
    len:      usize;
    cap:      usize;
    dedup:    std.collections.map.MapBy[Entry, StrId];
    base:     *Interner;
    base_len: usize;
}
```

a table of interned strings, each id naming one entry

| Field | Type | |
| :--- | :--- | :--- |
| `a` | `*std.allocator.Allocator` | owns every entry's bytes and the table |
| `strings` | `*Entry` | the entries this layer issued, in issue order |
| `len` | `usize` | how many entries this layer issued |
| `cap` | `usize` | the capacity of `strings` |
| `dedup` | `std.collections.map.MapBy[Entry, StrId]` | each entry this layer issued, to its id |
| `base` | `*Interner` | the interner this one is layered over, nil for none |
| `base_len` | `usize` | how many ids the base and its own bases had issued when this one was made |

### init

```mach
pub fun init(a: *std.allocator.Allocator) Interner
```

an empty interner with no base, released with dnit

### child_init

```mach
pub fun child_init(base: *Interner, a: *std.allocator.Allocator) Interner
```

an interner layered over `base`: it answers every id `base` and its own
bases had issued when it was made, and issues its own past them. a base may
itself be a child, so a session over a plan's interner layers its parallel
workers over the session's

### descends

```mach
pub fun descends(itn: *Interner, base: *Interner) bool
```

whether `itn` answers every id `base` has issued: it is `base`, or layered over it

### dnit

```mach
pub fun dnit(itn: *Interner)
```

release an interner and its strings; a nil interner holds nothing

### text

```mach
pub fun text(itn: *Interner, id: StrId) str
```

the text of `id`. an interner answers every id it or its base issued for as
long as it lives, so the read is total: an id it never issued, STR_NIL among
them, is a broken precondition of the caller, and it stops the compiler

### get

```mach
pub fun get(itn: *Interner, id: StrId) opt[str]
```

the text of `id`, or none for an id the interner never issued. it ends at
the first NUL, so a string that may hold one is read through `bytes`

### bytes

```mach
pub fun bytes(itn: *Interner, id: StrId) opt[View]
```

every byte of `id`, NULs included, or none for an id the interner never issued

### add

```mach
pub fun add(itn: *Interner, text: str) res[StrId, std.allocator.Error]
```

the id of the text `text` up to its first NUL, interned when it was not yet

| Parameter | |
| :--- | :--- |
| `itn` | the interner |
| `text` | the text |
| **returns** | its id, or the allocator's refusal |

### bytes_add

```mach
pub fun bytes_add(itn: *Interner, data: str, len: usize) res[StrId, std.allocator.Error]
```

the id of the `len` bytes at `data`, NULs included, interned when they were not yet

| Parameter | |
| :--- | :--- |
| `itn` | the interner |
| `data` | the bytes |
| `len` | how many |
| **returns** | their id, or the allocator's refusal |

### range_add

```mach
pub fun range_add(itn: *Interner, source: str, offset: usize, len: usize) res[StrId, std.allocator.Error]
```

the `len` bytes of `source` from `offset`, interned. the bytes are taken as
they are, so a NUL among them is part of the string

### find

```mach
pub fun find(itn: *Interner, text: str) opt[StrId]
```

the id `text` was interned under, none when it never was; nothing is interned, so a reader
that shares the interner may ask while no one adds

### child_reintern

```mach
pub fun child_reintern(child: *Interner, into: *Interner, alloc_: *std.allocator.Allocator) res[Remap, std.allocator.Error]
```

`child` merged into `into`, its base, another child of its base or, for a child with no base, any
interner: each string the child issued interned there in the order it issued them, and the ids
they got. a child layered over another interner is a contract violation of the receiver, reported
as the allocator layer reports its own (`invalid`)

### fold

```mach
pub fun fold(layer: *Interner) err[std.allocator.Error]
```

`layer` folded into its base, which issued nothing since the layer was made: each string the layer
issued moved there under the id it has, the layer left holding none. strings move, not copy, so
both draw on one allocator. a base that issued since is a contract violation of the receiver
(`invalid`); a failure leaves the strings not yet moved in the layer, still layered over the base

## Remap

```mach
pub rec Remap {
    base_len: usize;
    ids:      *StrId;
    len:      usize;
}
```

the ids a merged child's strings got in the merge's target

| Field | Type | |
| :--- | :--- | :--- |
| `base_len` | `usize` | where the child's own range began, an id below it left as it is |
| `ids` | `*StrId` | the target's id for each string the child issued, in issue order |
| `len` | `usize` | how many |

### remap_dnit

```mach
pub fun remap_dnit(alloc: *std.allocator.Allocator, remap: *Remap)
```

release a remap; a nil remap holds nothing

### remapped

```mach
pub fun remapped(remap: *Remap, id: StrId) StrId
```

the id in the merge's target that `id`, issued by the merged child, names: an id below the
child's own range is its base's and stays, one in it is mapped

