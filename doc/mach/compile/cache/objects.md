[mach](../../README.md) / [compile](../README.md) / [cache](README.md) / **objects**

# mach.compile.cache.objects

the digests of the objects a cell's builds wrote into `obj/`: for each object, by its path under
`obj/`, the sha-256 of its bytes as they were written. a restored object must be the object that
was written, so a restore hashes the bytes it reads and reuses the object only when they match
what its path's entry holds; an object with no entry, or whose bytes differ, is rebuilt. the
digests live at `<out>/.cache/objects`, written after the objects they describe are in place, so
an interrupted build leaves an entry that names the old bytes or none, never one naming bytes
that were not written. a store that is missing, truncated or damaged reads as empty. the layout
is `"MOD1"`, the entry count, the entries, then the sha-256 of everything before it. an entry is
its path (length then bytes) then its 32-byte digest, every number a little-endian u32

Source: [`src/compile/cache/objects.mach`](../../../../src/compile/cache/objects.mach)

Uses types from [`mach.base.fail`](../../base/fail.md).

| Type | | |
| :--- | :--- | :--- |
| [`Store`](#store) | `rec` | the object digests of a cell, indexed by the hash of their paths |

| On `Store` | |
| :--- | :--- |
| [`init`](#init) | an empty store |
| [`dnit`](#dnit) | release a store's entries and their paths |
| [`written`](#written) | the digest of the object written at `path` under `obj/`, none when no build recorded one |
| [`put`](#put) | the object at `path` under `obj/` was written as the `len` bytes at `bytes` |
| [`decode`](#decode) | the store `bytes` hold: every entry, or none at all when the bytes are not a complete store |
| [`load`](#load) | the store at `file`, empty when there is none or it cannot be read |
| [`store`](#store-1) | write the store to `file` through a sibling temporary when an entry changed |

---

## Store

```mach
pub rec Store {
    alloc:   *std.allocator.Allocator;
    entries: Vector[Entry];
    index:   std.collections.map.Map[u64, u32];
    dirty:   bool;
}
```

the object digests of a cell, indexed by the hash of their paths

| Field | Type | |
| :--- | :--- | :--- |
| `alloc` | `*std.allocator.Allocator` | the allocator owning every entry's path |
| `entries` | `Vector[Entry]` | each recorded object's path under `obj/` and digest |
| `index` | `std.collections.map.Map[u64, u32]` | each recorded path's hash to its entry's position |
| `dirty` | `bool` | an entry changed since the store was read or stored |

### init

```mach
pub fun init(a: *std.allocator.Allocator) Store
```

an empty store

| Parameter | |
| :--- | :--- |
| `a` | the allocator the store's entries are kept in |
| **returns** | the store, released with dnit |

### dnit

```mach
pub fun dnit(st: *Store)
```

release a store's entries and their paths

| Parameter | |
| :--- | :--- |
| `st` | the store to release |

### written

```mach
pub fun written(st: *Store, path: str) opt[[32]u8]
```

the digest of the object written at `path` under `obj/`, none when no build recorded one

### put

```mach
pub fun put(st: *Store, path: str, bytes: *u8, len: usize) err[std.allocator.Error]
```

the object at `path` under `obj/` was written as the `len` bytes at `bytes`

### decode

```mach
pub fun decode(a: *std.allocator.Allocator, bytes: *u8, len: usize) res[Store, std.allocator.Error]
```

the store `bytes` hold: every entry, or none at all when the bytes are not a complete store.
err only when allocation fails

### load

```mach
pub fun load(a: *std.allocator.Allocator, file: str) res[Store, std.allocator.Error]
```

the store at `file`, empty when there is none or it cannot be read

### store

```mach
pub fun store(st: *Store, out_dir: str, file: str) err[mach.base.fail.Fail]
```

write the store to `file` through a sibling temporary when an entry changed

