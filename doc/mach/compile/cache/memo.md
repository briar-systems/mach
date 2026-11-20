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

Source: [`src/compile/cache/memo.mach`](../../../../src/compile/cache/memo.mach)

Uses types from [`mach.base.fail`](../../base/fail.md).

| Type | | |
| :--- | :--- | :--- |
| [`Digests`](#digests) | `rec` | what a source text digests to (compile/surface): its bytes, its text without any function or test body, and its template text without positions and with the positions of what it omits |
| [`Memo`](#memo) | `rec` | the digest memo of one compiler identity |

| On `Memo` | |
| :--- | :--- |
| [`init`](#init) | an empty memo of a compiler |
| [`dnit`](#dnit) | release a memo's entries and index |
| [`lookup`](#lookup) | the surface digests remembered for the text whose digest `out.content` holds, written into `out`; false when the memo holds none |
| [`record`](#record) | remember the digests of a text, under its digest |
| [`decode`](#decode) | the memo `bytes` hold for `compiler`: every entry, or none at all when the bytes are not a complete memo of that compiler |
| [`encode`](#encode) | the memo's used entries as bytes, allocated in a and owned by the caller |
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
    alloc:    *std.allocator.Allocator;
    compiler: [32]u8;
    entries:  Vector[Entry];
    index:    std.collections.map.Map[u64, u32];
    dirty:    bool;
}
```

the digest memo of one compiler identity

| Field | Type | |
| :--- | :--- | :--- |
| `alloc` | `*std.allocator.Allocator` | the allocator holding the entries and the index |
| `compiler` | `[32]u8` | the digest of the compiler whose memo this is |
| `entries` | `Vector[Entry]` | the remembered digests in the order they were added |
| `index` | `std.collections.map.Map[u64, u32]` | each entry's index key to its position |
| `dirty` | `bool` | an entry was added since the memo was read or stored |

### init

```mach
pub fun init(a: *std.allocator.Allocator, compiler: *[32]u8) Memo
```

an empty memo of a compiler

| Parameter | |
| :--- | :--- |
| `a` | the allocator the memo's entries are kept in |
| `compiler` | the compiler's 32-byte digest, copied |
| **returns** | the memo, released with dnit |

### dnit

```mach
pub fun dnit(m: *Memo)
```

release a memo's entries and index

| Parameter | |
| :--- | :--- |
| `m` | the memo to release |

### lookup

```mach
pub fun lookup(m: *Memo, out: *Digests) bool
```

the surface digests remembered for the text whose digest `out.content` holds,
written into `out`; false when the memo holds none

### record

```mach
pub fun record(m: *Memo, d: *Digests) err[std.allocator.Error]
```

remember the digests of a text, under its digest

### decode

```mach
pub fun decode(a: *std.allocator.Allocator, compiler: *[32]u8, bytes: *u8, len: usize) res[Memo, std.allocator.Error]
```

the memo `bytes` hold for `compiler`: every entry, or none at all when the
bytes are not a complete memo of that compiler. err only when allocation fails

### encode

```mach
pub fun encode(m: *Memo, a: *std.allocator.Allocator, out_len: *usize) res[*u8, std.allocator.Error]
```

the memo's used entries as bytes, allocated in a and owned by the caller

### load

```mach
pub fun load(a: *std.allocator.Allocator, compiler: *[32]u8, file: str) res[Memo, std.allocator.Error]
```

the memo stored at `file`, empty when there is none or it cannot be read;
the file's bytes pass through `a` and are released before return

### store

```mach
pub fun store(m: *Memo, out_dir: str, file: str) err[mach.base.fail.Fail]
```

write the memo to `file` through a sibling temporary when it would change: an
entry was added or one went unused. the encoding passes through the memo's
allocator

