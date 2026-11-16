[mach](../../README.md) / [compile](../README.md) / [cache](README.md) / **memo**

# mach.compile.cache.memo

the digest memo: the surface digests of a source text, remembered under the
sha-256 of that text, so a text the build has seen costs one hash and not
the walks and three hashes its surfaces take. nothing about the file decides
anything: the text load read is hashed every build and only its digest finds
an entry. the memo lives at `<out>/.cache/digests` and belongs to one
compiler identity, whose parser decides the surface digests; a memo written
by another compiler, or one that is missing, truncated or damaged, reads as
empty. a stored memo holds the texts the build that stored it keyed. the
layout is `"MDM5"`, the 32-byte compiler digest, the entry count, the
entries, then the sha-256 of everything before it. an entry is the text's
digest then its three surface digests, every number little endian

<sub>source: [`src/compile/cache/memo.mach`](../../../../src/compile/cache/memo.mach) · imported by 1 module</sub>

<sub>`A` is `std.allocator` · `Entry` is private · `map` is `std.collections.map` · `fail` is [`mach.base.fail`](../../base/fail.md)</sub>

| Type | | |
| :--- | :--- | :--- |
| [`Digests`](#digests) | `rec` | what a source text digests to (compile/surface): its bytes, its text without any function or test body, and its template text without positions and with the positions of what it omits |
| [`Memo`](#memo) | `rec` |  |

| On `Memo` | |
| :--- | :--- |
| [`init`](#init) · [`dnit`](#dnit) | |
| [`lookup`](#lookup) | the surface digests remembered for the text whose digest `out.content` holds, written into `out`; false when the memo holds none |
| [`record`](#record) | remember the digests of a text, under its digest |
| [`decode`](#decode) | the memo `bytes` hold for `compiler`: every entry, or none at all when the bytes are not a complete memo of that compiler |
| [`encode`](#encode) | the memo's used entries as bytes, res_allocate in a and owned by the caller |
| [`load`](#load) | the memo stored at `file`, empty when there is none or it cannot be read; the file's bytes pass through `a` and are released before return |
| [`store`](#store) | write the memo to `file` through a sibling temporary when it would change: an entry was added or one went unused |

---

## Digests

```mach
pub rec Digests {
    content:         [32]u8;
    surface:         [32]u8;
    template:        [32]u8;
    template_placed: [32]u8;
}
```

what a source text digests to (compile/surface): its bytes, its text without any function or test
body, and its template text without positions and with the positions of what it omits

## Memo

```mach
pub rec Memo {
    alloc:    *A.Allocator;
    compiler: [32]u8;
    entries:  Vector[Entry];
    index:    map.Map[u64, u32];
    dirty:    bool;
}
```

### init

```mach
pub fun init(a: *A.Allocator, compiler: *[32]u8) Memo
```

### dnit

```mach
pub fun dnit(m: *Memo)
```

### lookup

```mach
pub fun lookup(m: *Memo, out: *Digests) bool
```

the surface digests remembered for the text whose digest `out.content` holds,
written into `out`; false when the memo holds none

### record

```mach
pub fun record(m: *Memo, d: *Digests) err[A.Error]
```

remember the digests of a text, under its digest

### decode

```mach
pub fun decode(a: *A.Allocator, compiler: *[32]u8, bytes: *u8, len: usize) res[Memo, A.Error]
```

the memo `bytes` hold for `compiler`: every entry, or none at all when the
bytes are not a complete memo of that compiler. err only when allocation fails

### encode

```mach
pub fun encode(m: *Memo, a: *A.Allocator, out_len: *usize) res[*u8, A.Error]
```

the memo's used entries as bytes, res_allocate in a and owned by the caller

### load

```mach
pub fun load(a: *A.Allocator, compiler: *[32]u8, file: str) res[Memo, A.Error]
```

the memo stored at `file`, empty when there is none or it cannot be read;
the file's bytes pass through `a` and are released before return

### store

```mach
pub fun store(m: *Memo, out_dir: str, file: str) err[fail.Fail]
```

write the memo to `file` through a sibling temporary when it would change: an
entry was added or one went unused. the encoding passes through the memo's
allocator

