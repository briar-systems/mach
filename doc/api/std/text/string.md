# std.text.string

## fun str_dup

```mach
pub fun str_dup(a: *Allocator, s: str) res[str, Error];
```

allocate an owned, null-terminated copy of `s`

a: allocator backing the copy
s: borrowed null-terminated string to copy
ret: an owned copy (freed via str_free), or the allocator's refusal

## fun str_dup_range

```mach
pub fun str_dup_range(a: *Allocator, source: str, offset: usize, len: usize) res[str, Error];
```

allocate an owned, null-terminated copy of `len` bytes of
`source` starting at `offset`.

a: allocator backing the copy
source: borrowed string to slice from
offset: starting byte offset into source
len: number of bytes to copy
ret: an owned copy (freed via str_free), or the allocator's refusal

## fun str_free

```mach
pub fun str_free(a: *Allocator, s: str) err[Error];
```

release a string previously returned by str_dup / str_dup_range

the allocation extent is `str_len + 1`, which is the extent every producer
of a plain owned `str` in std reserves; a string whose extent differs is an
`OwnedString` (std.types.string) and is released through its own record.

a: allocator the string was allocated from
s: owned string to free (nil is a no-op)
ret: ok, or the allocator's refusal of the release

