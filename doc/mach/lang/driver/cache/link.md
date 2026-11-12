# mach.lang.driver.cache.link

the link records of a cell: for each output a link wrote, by its canonical path, the key of the
link that wrote it and the sha-256 of the bytes it wrote. a build that compiled nothing and keys
its link as the record of its output does skips the link when the file there still hashes to the
recorded digest. the records live at `<out>/.cache/link`, written after the output they describe
is in place, so an interrupted build leaves a record naming the old bytes, never one naming bytes
that were not written. a store that is missing, truncated or damaged reads as empty. the layout
is `"MLK1"`, the record count, the records, then the sha-256 of everything before it. a record is
its path (length then bytes), its 32-byte link key and its 32-byte digest, every number a
little-endian u32

## rec Written

```mach
pub rec Written;
```

what wrote the output at a path: the link's key and the digest of the bytes it wrote

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
pub fun written(st: *Store, path: str) opt[Written];
```

what the record of `path` says wrote it, none when no link is recorded there

## fun put

```mach
pub fun put(st: *Store, path: str, w: *Written) err[A.Error];
```

the record of `path` set to `w`; a path whose hash another path holds is not recorded, so its
link always runs

## fun decode

```mach
pub fun decode(a: *A.Allocator, bytes: *u8, len: usize) res[Store, A.Error];
```

the store `bytes` hold: every record, or none at all when the bytes are not a complete store.
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

write the store to `file` through a sibling temporary

