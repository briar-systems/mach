[mach](../../README.md) / [compile](../README.md) / [cache](README.md) / **record**

# mach.compile.cache.record

the record an `obj/` object carries for the object cache in its `.mach.cache`
section: the key the object was built under and what the engine reads from a
module's lowered unit besides its object, so a module whose object is reused
never has to lower. the object itself is what mink reads back from the file.
the layout is `"MCRC"`, the 32-byte key, the 32-byte digest of the sources
outside the module its warnings are placed in, zero when they place nothing
there, the test count, then per test its qualified name and its line, then the
length and bytes of the module's warnings (compile/cache encodes them), then
the count of the symbols the module defines for its test object to declare
and per symbol its name, then the count of the instances the object holds a
copy of and per instance its symbol, its 32-byte key and the count and names
of the modules it reads, every number a little-endian u32 and every name its
length then its bytes

Source: [`src/compile/cache/record.mach`](../../../../src/compile/cache/record.mach)

Uses types from [`mach.base.fail`](../../base/fail.md), [`mach.base.intern`](../../base/intern.md).

| Type | | |
| :--- | :--- | :--- |
| [`TestFact`](#testfact) | `rec` | a test the module declares, as its object's record keeps it |
| [`Facts`](#facts) | `rec` | what the engine reads from a module's lowered unit besides its object, owned by the allocator that decoded or built them and released with facts_dnit |
| [`Copy`](#copy) | `rec` | an instance the object holds a copy of: its symbol, its key (compile/cache, instance_key) and the modules it reads, the `count` names of `copy_reads` from `first`, its declaring module first |
| [`Record`](#record) | `rec` | an object's cache record |

| On `Facts` | |
| :--- | :--- |
| [`facts_dnit`](#facts_dnit) | release recorded facts; nil facts hold nothing |
| [`encode`](#encode) | the record for an object built under `key` whose module lowered to `facts`; the caller owns the bytes, allocated in alloc |

| On `Record` | |
| :--- | :--- |
| [`record_dnit`](#record_dnit) | release a record; a nil record holds nothing |
| [`decode`](#decode) | the record in `bytes`, or none when it is missing, truncated or malformed: an object whose record cannot be read is not reused |

---

## TestFact

```mach
pub rec TestFact {
    name: mach.base.intern.StrId;
    line: u32;
}
```

a test the module declares, as its object's record keeps it

| Field | Type | |
| :--- | :--- | :--- |
| `name` | `mach.base.intern.StrId` | the test's qualified name |
| `line` | `u32` | the line the test is declared on |

## Facts

```mach
pub rec Facts {
    # the digest of the sources other than the module's own the object's warnings
    # are placed in, all zero when they place nothing there: an instance the object
    # holds reports into its generic's source, which the object's key does not
    # cover, and its recorded warnings stand only for the sources they name
    source:     [32]u8;
    tests:      *TestFact;
    test_count: u32;
    # the diagnostics short of errors building the module reported, which a
    # build that reuses the object reports again
    warnings:     *u8;
    warnings_len: u32;
    # every symbol the module defines under a binding other objects see, which its test object
    # declares instead: kept for a module that declares tests, none otherwise
    defined:       *mach.base.intern.StrId;
    defined_count: u32;
    # every instance the object holds a copy of, under the key it had when the object was built:
    # the object stands only while each keys the same
    copies:      *Copy;
    copy_count:  u32;
    copy_reads:  *mach.base.intern.StrId;
    reads_count: u32;
}
```

what the engine reads from a module's lowered unit besides its object, owned by the allocator
that decoded or built them and released with facts_dnit

| Field | Type | |
| :--- | :--- | :--- |
| `tests` | `*TestFact` | the module's tests, test_count of them |
| `copy_reads` | `*mach.base.intern.StrId` | the modules the copies read, reads_count names, each copy a run of them |

### facts_dnit

```mach
pub fun facts_dnit(alloc: *std.allocator.Allocator, f: *Facts)
```

release recorded facts; nil facts hold nothing

### encode

```mach
pub fun encode(alloc: *std.allocator.Allocator, itn: *mach.base.intern.Interner, key: *[32]u8, facts: *Facts,
    out_bytes: **u8, out_len: *u32) err[mach.base.fail.Fail]
```

the record for an object built under `key` whose module lowered to `facts`;
the caller owns the bytes, allocated in alloc

## Copy

```mach
pub rec Copy {
    name:  mach.base.intern.StrId;
    key:   [32]u8;
    first: u32;
    count: u32;
}
```

an instance the object holds a copy of: its symbol, its key (compile/cache, instance_key) and the
modules it reads, the `count` names of `copy_reads` from `first`, its declaring module first

## Record

```mach
pub rec Record {
    key:   [32]u8;
    facts: Facts;
}
```

an object's cache record

| Field | Type | |
| :--- | :--- | :--- |
| `key` | `[32]u8` | the key the object was built under |
| `facts` | `Facts` | what the module lowered to besides its object |

### record_dnit

```mach
pub fun record_dnit(alloc: *std.allocator.Allocator, r: *Record)
```

release a record; a nil record holds nothing

### decode

```mach
pub fun decode(alloc: *std.allocator.Allocator, itn: *mach.base.intern.Interner, bytes: *u8, len: usize) res[opt[Record], mach.base.fail.Fail]
```

the record in `bytes`, or none when it is missing, truncated or malformed:
an object whose record cannot be read is not reused. err only when
allocation fails

