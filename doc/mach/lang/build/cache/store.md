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

## def Kind

```mach
pub def Kind: u8
```

the store's own failure vocabulary: the kind decides whether a caller
treats the store as unavailable or reports an internal error

## val IO

```mach
pub val IO:          Kind = 0
```

## val MEMORY

```mach
pub val MEMORY:      Kind = 1
```

## val INVALID

```mach
pub val INVALID:     Kind = 2
```

## val CONTAINMENT

```mach
pub val CONTAINMENT: Kind = 3
```

## val REJECTED

```mach
pub val REJECTED:    Kind = 4
```

## def Op

```mach
pub def Op: u8
```

## val OP_OPEN

```mach
pub val OP_OPEN:    Op = 0
```

## val OP_PREPARE

```mach
pub val OP_PREPARE: Op = 1
```

## val OP_RECOVER

```mach
pub val OP_RECOVER: Op = 2
```

## val OP_REMOVE

```mach
pub val OP_REMOVE:  Op = 3
```

## rec Fault

```mach
pub rec Fault;
```

## fun fault

```mach
pub fun fault(kind: Kind, op: Op) Fault;
```

## fun io_fault

```mach
pub fun io_fault(op: Op, code: i64) Fault;
```

## fun fault_message

```mach
pub fun fault_message(e: Fault) str;
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

## fun memory_exhausted

```mach
pub fun memory_exhausted(code: i64) bool;
```

a native code that means the process is out of memory

## fun checksum

```mach
pub fun checksum(bytes: *u8, len: usize) u64;
```

every step is a bijection of the running sum for a fixed word, so a change
confined to one 8-byte word (any single flipped bit) always changes the result

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

