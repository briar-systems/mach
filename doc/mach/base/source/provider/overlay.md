[mach](../../../README.md) / [base](../../README.md) / [source](../README.md) / [provider](README.md) / **overlay**

# mach.base.source.provider.overlay

the source provider that reads an editor's open buffers in front of another provider

the buffers are kept by cleaned path, so a buffer is read in place of its file, and a buffer
whose file is not on disk yet still names a module.

Source: [`src/base/source/provider/overlay.mach`](../../../../../src/base/source/provider/overlay.mach)

Uses types from [`mach.base.fail`](../../fail.md), [`mach.base.source.provider`](../provider.md).

| Type | | |
| :--- | :--- | :--- |
| [`Overlay`](#overlay) | `rec` | the provider an editor reads through: the session's open buffers stand in front of the provider under them, so a buffer is read for its file and a buffer whose file is not on disk yet still names a module |
| [`Store`](#store) | `rec` | the open buffers, one per file however its path is spelled; scanned by path, as an editor holds a handful |
| [`Prepared`](#prepared) | `rec` | a staged change to the buffers, made by commit or released by discard |

| On `Overlay` | |
| :--- | :--- |
| [`provider_of`](#provider_of) | the provider contract over `o`, which must outlive it |

| On `Store` | |
| :--- | :--- |
| [`init`](#init) | the overlay of the buffers in `s` over `base` |
| [`store_init`](#store_init) | an empty buffer store owning its entries through `a`, released with store_dnit |
| [`store_dnit`](#store_dnit) | release every buffer and the store's list |
| [`prepare`](#prepare) | the caller exclusively borrows overlay state through commit or discard |
| [`set`](#set) | set the text standing in for `path`; true when it changed the texts |
| [`get`](#get) | the text standing in for `path`, none when no buffer does |

| On `Prepared` | |
| :--- | :--- |
| [`discard`](#discard) | release an overlay that was prepared and not committed; one already committed or discarded owns nothing |
| [`commit`](#commit) | the prepared change made, and whether it changed the texts; a change expires the views borrowed from the session holding the store |

---

## Overlay

```mach
pub rec Overlay {
    base: mach.base.source.provider.Provider;
    s:    *Store;
}
```

the provider an editor reads through: the session's open buffers stand in front of the
provider under them, so a buffer is read for its file and a buffer whose file is not on
disk yet still names a module

| Field | Type | |
| :--- | :--- | :--- |
| `base` | `mach.base.source.provider.Provider` | the provider the buffers stand in front of |
| `s` | `*Store` | the buffers, keyed by the path the base locates |

### provider_of

```mach
pub fun provider_of(o: *Overlay) mach.base.source.provider.Provider
```

the provider contract over `o`, which must outlive it

## Store

```mach
pub rec Store {
    a:     *std.allocator.Allocator;
    items: Vector[Entry];
}
```

the open buffers, one per file however its path is spelled; scanned by path, as an
editor holds a handful

| Field | Type | |
| :--- | :--- | :--- |
| `a` | `*std.allocator.Allocator` | owns every entry's path and text |
| `items` | `Vector[Entry]` | the buffers |

### init

```mach
pub fun init(base: mach.base.source.provider.Provider, s: *Store) Overlay
```

the overlay of the buffers in `s` over `base`

### store_init

```mach
pub fun store_init(a: *std.allocator.Allocator) Store
```

an empty buffer store owning its entries through `a`, released with store_dnit

### store_dnit

```mach
pub fun store_dnit(s: *Store)
```

release every buffer and the store's list

### prepare

```mach
pub fun prepare(s: *Store, path: str, text: str, remove: bool) res[Prepared, mach.base.fail.Fail]
```

the caller exclusively borrows overlay state through commit or discard

### set

```mach
pub fun set(s: *Store, path: str, text: str) res[bool, mach.base.fail.Fail]
```

set the text standing in for `path`; true when it changed the texts

### get

```mach
pub fun get(s: *Store, path: str) opt[str]
```

the text standing in for `path`, none when no buffer does

## Prepared

```mach
pub rec Prepared {
    owner:   *Store;
    key:     str;
    text:    str;
    index:   usize;
    remove:  bool;
    changed: bool;
}
```

a staged change to the buffers, made by commit or released by discard

| Field | Type | |
| :--- | :--- | :--- |
| `owner` | `*Store` | the store, nil once committed or discarded |
| `key` | `str` | the cleaned path, owned until a new entry takes it |
| `text` | `str` | the copied text, owned until commit takes it, nil for a removal or no change |
| `index` | `usize` | the buffer the path names, the store's length for a new one |
| `remove` | `bool` | the change removes the buffer |
| `changed` | `bool` | the change alters the texts |

### discard

```mach
pub fun discard(prepared: *Prepared)
```

release an overlay that was prepared and not committed; one already committed or
discarded owns nothing

### commit

```mach
pub fun commit(prepared: *Prepared) bool
```

the prepared change made, and whether it changed the texts; a change expires the views borrowed
from the session holding the store

