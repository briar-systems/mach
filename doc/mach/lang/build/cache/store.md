# mach.lang.build.cache.store

## val MAX_STORE_BYTES

```mach
pub val MAX_STORE_BYTES: u64   = 536870912
```

logical entry bytes include headers, excluding filesystem metadata and allocation units

## val HIT

```mach
pub val HIT:             u8    = 1
```

## val MISS

```mach
pub val MISS:            u8    = 2
```

## val UNAVAILABLE

```mach
pub val UNAVAILABLE:     u8    = 3
```

## val STORED

```mach
pub val STORED:          u8    = 4
```

## rec Error

```mach
pub rec Error;
```

## rec Status

```mach
pub rec Status;
```

## rec Lookup

```mach
pub rec Lookup;
```

## fun read

```mach
pub fun read(alloc: *A.Allocator, directory: str, key: *[32]u8) res[Lookup, Error];
```

directory and compiler work remain caller-owned, returned hit bytes use alloc.

## fun publish

```mach
pub fun publish(alloc: *A.Allocator, directory: str, key: *[32]u8, payload: *u8, len: usize,
keys: *[32]u8, key_count: usize) res[Status, Error];
```

publication may report unavailable after replacement or partial eviction
keys names the entries this build restored or published, which eviction keeps

