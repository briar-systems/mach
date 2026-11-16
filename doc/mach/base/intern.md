[mach](../README.md) / [base](README.md) / **intern**

# mach.base.intern

interned strings: one stable id per distinct byte string

a string may hold NUL bytes and is equal to another by its bytes and length. an interner may be
layered over a base: it answers every id the base had issued when it was made and issues its
own past them, and `child_reintern` maps a child's strings into its base.

<sub>source: [`src/base/intern.mach`](../../../src/base/intern.mach) · imported by 120 modules</sub>

<sub>`A` is `std.allocator` · `Entry` is private · `map` is `std.collections.map`</sub>

| Type | | |
| :--- | :--- | :--- |
| [`StrId`](#strid) | `def` | the identity of one interned string |
| [`Interner`](#interner) | `rec` |  |
| [`Remap`](#remap) | `rec` |  |

| On `StrId` | |
| :--- | :--- |
| [`STR_NIL`](#strid) · [`str_id`](#str_id) · [`str_index`](#str_index) · [`str_same`](#str_same) · [`str_is_nil`](#str_is_nil) | |

| On `Interner` | |
| :--- | :--- |
| [`init`](#init) | |
| [`child_init`](#child_init) | an interner layered over `base`: it answers every id `base` and its own bases had issued when it was made, and issues its own past them |
| [`descends`](#descends) | whether `itn` answers every id `base` has issued: it is `base`, or layered over it |
| [`dnit`](#dnit) | release an interner and its strings; a nil interner holds nothing |
| [`text`](#text) | the text of `id` |
| [`get`](#get) | the text of `id`, or none for an id the interner never issued |
| [`bytes`](#bytes) | every byte of `id`, NULs included, or none for an id the interner never issued |
| [`add`](#add) · [`bytes_add`](#bytes_add) | |
| [`range_add`](#range_add) | the `len` bytes of `source` from `offset`, interned |
| [`find`](#find) | the id `text` was interned under, none when it never was; nothing is interned, so a reader that shares the interner may ask while no one adds |
| [`child_reintern`](#child_reintern) | a non-child interner is a contract violation of the receiver, reported as the allocator layer reports its own (`invalid`) |

| On `Remap` | |
| :--- | :--- |
| [`remap_dnit`](#remap_dnit) | release a remap; a nil remap holds nothing |

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

### str_index

```mach
pub fun str_index(id: StrId) u32
```

### str_same

```mach
pub fun str_same(left: StrId, right: StrId) bool
```

### str_is_nil

```mach
pub fun str_is_nil(id: StrId) bool
```

## Interner

```mach
pub rec Interner {
    a:        *A.Allocator;
    strings:  *Entry;
    len:      usize;
    cap:      usize;
    dedup:    map.MapBy[Entry, StrId];
    base:     *Interner;
    base_len: usize;
}
```

### init

```mach
pub fun init(a: *A.Allocator) Interner
```

### child_init

```mach
pub fun child_init(base: *Interner, a: *A.Allocator) Interner
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
pub fun add(itn: *Interner, text: str) res[StrId, A.Error]
```

### bytes_add

```mach
pub fun bytes_add(itn: *Interner, data: str, len: usize) res[StrId, A.Error]
```

### range_add

```mach
pub fun range_add(itn: *Interner, source: str, offset: usize, len: usize) res[StrId, A.Error]
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
pub fun child_reintern(child: *Interner, alloc_: *A.Allocator) res[Remap, A.Error]
```

a non-child interner is a contract violation of the receiver, reported as
the allocator layer reports its own (`invalid`)

## Remap

```mach
pub rec Remap {
    base_len: usize;
    ids:      *StrId;
    len:      usize;
}
```

### remap_dnit

```mach
pub fun remap_dnit(alloc: *A.Allocator, remap: *Remap)
```

release a remap; a nil remap holds nothing

