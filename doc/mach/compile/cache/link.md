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

<sub>source: [`src/compile/cache/link.mach`](../../../../src/compile/cache/link.mach) · imported by 1 module</sub>

<sub>`A` is `std.allocator` · `Record` is private · `map` is `std.collections.map` · `fail` is [`mach.base.fail`](../../base/fail.md)</sub>

| Type | | |
| :--- | :--- | :--- |
| [`Written`](#written) | `rec` | what wrote the output at a path: the link's key, the digest of the bytes it wrote, whether it read a file a search found, and the files beyond its keyed inputs it read, which a record owns and a caller's lends |
| [`Store`](#store) | `rec` |  |

| On `Store` | |
| :--- | :--- |
| [`init`](#init) · [`dnit`](#dnit) | |
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
    alloc:   *A.Allocator;
    records: Vector[Record];
    index:   map.Map[u64, u32];
}
```

### init

```mach
pub fun init(a: *A.Allocator) Store
```

### dnit

```mach
pub fun dnit(st: *Store)
```

### written

```mach
pub fun written(st: *Store, path: str) opt[Written]
```

what the record of `path` says wrote it, its files the store's, none when no link is recorded
there

### put

```mach
pub fun put(st: *Store, path: str, w: *Written) err[A.Error]
```

the record of `path` set to `w`, a copy of its files the store's; a path whose hash another path
holds is not recorded, so its link always runs

### decode

```mach
pub fun decode(a: *A.Allocator, bytes: *u8, len: usize) res[Store, A.Error]
```

the store `bytes` hold: every record, or none at all when the bytes are not a complete store.
err only when allocation fails

### load

```mach
pub fun load(a: *A.Allocator, file: str) res[Store, A.Error]
```

the store at `file`, empty when there is none or it cannot be read

### store

```mach
pub fun store(st: *Store, out_dir: str, file: str) err[fail.Fail]
```

write the store to `file` through a sibling temporary

