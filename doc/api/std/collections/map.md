# std.collections.map

## rec Map

```mach
pub rec Map[K, V];
```

generic hash map with user-provided hash and equality functions

alloc: allocator for backing storage
keys: key buffer
values: value buffer
states: per-slot state (empty/occupied/tombstone)
len: number of entries
tombstones: number of tombstone slots
cap: total slot count
hash_fn: hash function taking a ptr, returning u64
eq_fn: equality function taking two ptrs, returning bool

## fun init

```mach
pub fun init[K, V](alloc: *allocator.Allocator, hash_fn: fun(ptr) u64, eq_fn: fun(ptr, ptr) bool) Map[K, V];
```

create an empty map

alloc: allocator to use for backing storage
hash_fn: hash function for keys
eq_fn: equality function for keys
ret: a new empty Map[K, V]

## fun dnit

```mach
pub fun dnit[K, V](m: *Map[K, V]) err[allocator.Error];
```

free all backing storage and reset the map

entries are not visited: a key or value that owns storage is the caller's
to release before this runs. all three buffers are released even when one
release fails, and the first failure is what is reported.

ret: ok when every buffer was released (or there were none), or the first
     refusal

## fun is_empty

```mach
pub fun is_empty[K, V](m: Map[K, V]) bool;
```

check if the map has no entries

ret: true if the map has no entries

## fun length

```mach
pub fun length[K, V](m: Map[K, V]) usize;
```

return the number of entries in the map

ret: number of entries

## fun capacity

```mach
pub fun capacity[K, V](m: Map[K, V]) usize;
```

return the current capacity in slots

ret: capacity in slots

## fun clear

```mach
pub fun clear[K, V](m: *Map[K, V]);
```

remove all entries but keep allocated capacity

## fun insert

```mach
pub fun insert[K, V](m: *Map[K, V], key: K, value: V) res[bool, allocator.Error];
```

insert or update a key-value pair

key and value are copied into the map. updating an existing key overwrites
the stored value in place and does not touch the stored key: a value that
owns storage must be read back with `get` and released by the caller before
it is replaced. a refused growth leaves the map unchanged.

key: key to insert
value: value to associate
ret: true if a new entry was inserted, false if an existing entry was
       updated, or the allocator's refusal

## fun get

```mach
pub fun get[K, V](m: *Map[K, V], key: *K) opt[*V];
```

get a pointer to the value associated with a key

the pointer is into the map's storage and is invalidated by any growth.

key: pointer to the key to look up
ret: pointer to the value, or none when the key is absent

## fun contains

```mach
pub fun contains[K, V](m: *Map[K, V], key: *K) bool;
```

check if the map contains a key

key: pointer to the key to check
ret: true if the key exists

## fun remove

```mach
pub fun remove[K, V](m: *Map[K, V], key: *K) bool;
```

remove a key-value pair from the map

the entry's slot is released; a key or value that owns storage must be read
back with `get` and released by the caller before it is removed. removal
never allocates, so it cannot be refused.

key: pointer to the key to remove
ret: true if removed, false if the key was not found

## fun hash_str

```mach
pub fun hash_str(p: ptr) u64;
```

FNV-1a hash for strings

## fun eq_str

```mach
pub fun eq_str(pa: ptr, pb: ptr) bool;
```

equality function for strings

## fun hash_i64

```mach
pub fun hash_i64(p: ptr) u64;
```

splitmix64 hash for i64

## fun eq_i64

```mach
pub fun eq_i64(pa: ptr, pb: ptr) bool;
```

equality function for i64

## fun hash_u64

```mach
pub fun hash_u64(p: ptr) u64;
```

splitmix64 hash for u64

## fun eq_u64

```mach
pub fun eq_u64(pa: ptr, pb: ptr) bool;
```

equality function for u64

## fun hash_u32

```mach
pub fun hash_u32(p: ptr) u64;
```

splitmix64 hash for u32

## fun eq_u32

```mach
pub fun eq_u32(pa: ptr, pb: ptr) bool;
```

equality function for u32

