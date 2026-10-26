# mach.provider.git.index

checked Git index decoding; entry paths borrow the caller's input bytes

## val MAX_ENTRIES

```mach
pub val MAX_ENTRIES: usize = 1000000
```

## rec Entry

```mach
pub rec Entry;
```

## rec Index

```mach
pub rec Index;
```

## fun encode

```mach
pub fun encode(a: *A.Allocator, index: *Index, hash_len: usize) res[Vector[u8], str];
```

encode entries in v3 form without derived optional extension caches

## fun decode

```mach
pub fun decode(a: *A.Allocator, data: *u8, len: usize, hash_len: usize) res[Index, str];
```

v2/v3 decoding preserves the stat cache, staging flags and extended flags

