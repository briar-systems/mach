[mach](../../README.md) / [compile](../README.md) / [cache](README.md) / **link**

# mach.compile.cache.link

the link records of a cell: for each output a link wrote, by its canonical path, the key of the
link that wrote it, the sha-256 of the bytes it wrote, the files beyond its keyed inputs it read
and whether it read a file a search found. a build that compiled nothing and keys its link as the
record of its output does skips the link when the file there still hashes to the recorded
digest. the records live at `<out>/.cache/link`, written after the output they describe is in
place, so an interrupted build leaves a record naming the old bytes, never one naming bytes that
were not written. a store that is missing, truncated or damaged reads as empty. the layout is
`"MLK2"`, the record count, the records, then the sha-256 of everything before it. a record is
its path (length then bytes), its 32-byte link key, its 32-byte digest, a byte that is one when
the link read a file a search found, the count of the files it read and each file's path
(length then bytes), every number a little-endian u32

Source: [`src/compile/cache/link.mach`](../../../../src/compile/cache/link.mach)

Uses types from [`mach.base.fail`](../../base/fail.md).

| Type | | |
| :--- | :--- | :--- |
| [`Written`](#written) | `rec` | what wrote the output at a path: the link's key, the digest of the bytes it wrote, whether it read a file a search found, and the files beyond its keyed inputs it read, which a record owns and a caller's lends |
| [`Store`](#store) | `rec` | the link records of a cell, indexed by the hash of their paths |

| On `Store` | |
| :--- | :--- |
| [`init`](#init) | an empty store |
| [`dnit`](#dnit) | release a store's records, their paths and their files |
| [`written`](#written-1) | what the record of `path` says wrote it, its files the store's, none when no link is recorded there |
| [`put`](#put) | the record of `path` set to `w`, a copy of its files the store's; a path whose hash another path holds is not recorded, so its link always runs |
| [`decode`](#decode) | the store `bytes` hold: every record, or none at all when the bytes are not a complete store |
| [`load`](#load) | the store at `file`, empty when there is none or it cannot be read |
| [`store`](#store-1) | write the store to `file` through a sibling temporary |

---

## Written

```mach
pub rec Written {
    key:      [32]u8;
    digest:   [32]u8;
    searched: bool;
    files:    Vector[str];
}
```

what wrote the output at a path: the link's key, the digest of the bytes it wrote, whether it
read a file a search found, and the files beyond its keyed inputs it read, which a record owns
and a caller's lends

## Store

```mach
pub rec Store {
    alloc:   *std.allocator.Allocator;
    records: Vector[Record];
    index:   std.collections.map.Map[u64, u32];
}
```

the link records of a cell, indexed by the hash of their paths

| Field | Type | |
| :--- | :--- | :--- |
| `alloc` | `*std.allocator.Allocator` | the allocator owning every record's path and files |
| `records` | `Vector[Record]` | the records in the order they were put |
| `index` | `std.collections.map.Map[u64, u32]` | each recorded path's hash to its record's position |

### init

```mach
pub fun init(a: *std.allocator.Allocator) Store
```

an empty store

| Parameter | |
| :--- | :--- |
| `a` | the allocator the store's records are kept in |
| **returns** | the store, released with dnit |

### dnit

```mach
pub fun dnit(st: *Store)
```

release a store's records, their paths and their files

| Parameter | |
| :--- | :--- |
| `st` | the store to release |

### written

```mach
pub fun written(st: *Store, path: str) opt[Written]
```

what the record of `path` says wrote it, its files the store's, none when no link is recorded
there

### put

```mach
pub fun put(st: *Store, path: str, w: *Written) err[std.allocator.Error]
```

the record of `path` set to `w`, a copy of its files the store's; a path whose hash another path
holds is not recorded, so its link always runs

### decode

```mach
pub fun decode(a: *std.allocator.Allocator, bytes: *u8, len: usize) res[Store, std.allocator.Error]
```

the store `bytes` hold: every record, or none at all when the bytes are not a complete store.
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

write the store to `file` through a sibling temporary

