# mach.lang.driver.cache.memo

the digest memo: the surface digests of a source text, remembered under the
sha-256 of that text, so a text the build has seen costs one hash and not
the walk and three hashes its surfaces take. nothing about the file decides
anything: the text load read is hashed every build and only its digest finds
an entry. the memo lives at `<out>/.cache/digests` and belongs to one
compiler identity, whose parser decides the surface digests; a memo written
by another compiler, or one that is missing, truncated or damaged, reads as
empty. a stored memo holds the texts the build that stored it keyed. the
layout is `"MDM4"`, the 32-byte compiler digest, the entry count, the
entries, then the sha-256 of everything before it. an entry is the text's
digest then its three surface digests, every number little endian

## rec Digests

```mach
pub rec Digests;
```

what a source text digests to: its bytes, its surface without positions, its
surface with the positions of what it omits, and its code without its test
declarations, with their positions

## rec Memo

```mach
pub rec Memo;
```

## fun init

```mach
pub fun init(a: *A.Allocator, compiler: *[32]u8) Memo;
```

## fun dnit

```mach
pub fun dnit(m: *Memo);
```

## fun lookup

```mach
pub fun lookup(m: *Memo, out: *Digests) bool;
```

the surface digests remembered for the text whose digest `out.content` holds,
written into `out`; false when the memo holds none

## fun record

```mach
pub fun record(m: *Memo, d: *Digests) err[A.Error];
```

remember the digests of a text, under its digest

## fun decode

```mach
pub fun decode(a: *A.Allocator, compiler: *[32]u8, bytes: *u8, len: usize) res[Memo, A.Error];
```

the memo `bytes` hold for `compiler`: every entry, or none at all when the
bytes are not a complete memo of that compiler. err only when allocation fails

## fun encode

```mach
pub fun encode(m: *Memo, a: *A.Allocator, out_len: *usize) res[*u8, A.Error];
```

the memo's used entries as bytes, res_allocate in a and owned by the caller

## fun load

```mach
pub fun load(a: *A.Allocator, compiler: *[32]u8, file: str) res[Memo, A.Error];
```

the memo stored at `file`, empty when there is none or it cannot be read;
the file's bytes pass through `a` and are released before return

## fun store

```mach
pub fun store(m: *Memo, out_dir: str, file: str) err[fail.Fail];
```

write the memo to `file` through a sibling temporary when it would change: an
entry was added or one went unused. the encoding passes through the memo's
allocator

