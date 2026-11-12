# mach.lang.driver.cache.objects

the digests of the objects a cell's builds wrote into `obj/`: for each object, by its path under
`obj/`, the sha-256 of its bytes as they were written. a restored object must be the object that
was written, so a restore hashes the bytes it reads and reuses the object only when they match
what its path's entry holds; an object with no entry, or whose bytes differ, is rebuilt. the
digests live at `<out>/.cache/objects`, written after the objects they describe are in place, so
an interrupted build leaves an entry that names the old bytes or none, never one naming bytes
that were not written. a store that is missing, truncated or damaged reads as empty. the layout
is `"MOD1"`, the entry count, the entries, then the sha-256 of everything before it. an entry is
its path (length then bytes) then its 32-byte digest, every number a little-endian u32

## rec Store

```mach
pub rec Store;
```

## fun init

```mach
pub fun init(a: *A.Allocator) Store;
```

## fun dnit

```mach
pub fun dnit(st: *Store);
```

## fun written

```mach
pub fun written(st: *Store, path: str) opt[[32]u8];
```

the digest of the object written at `path` under `obj/`, none when no build recorded one

## fun put

```mach
pub fun put(st: *Store, path: str, bytes: *u8, len: usize) err[A.Error];
```

the object at `path` under `obj/` was written as the `len` bytes at `bytes`

## fun decode

```mach
pub fun decode(a: *A.Allocator, bytes: *u8, len: usize) res[Store, A.Error];
```

the store `bytes` hold: every entry, or none at all when the bytes are not a complete store.
err only when allocation fails

## fun load

```mach
pub fun load(a: *A.Allocator, file: str) res[Store, A.Error];
```

the store at `file`, empty when there is none or it cannot be read

## fun store

```mach
pub fun store(st: *Store, out_dir: str, file: str) err[fail.Fail];
```

write the store to `file` through a sibling temporary when an entry changed

