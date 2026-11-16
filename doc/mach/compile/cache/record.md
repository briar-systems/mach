[mach](../../README.md) / [compile](../README.md) / [cache](README.md) / **record**

# mach.compile.cache.record

the record an `obj/` object carries for the object cache in its `.mach.cache`
section: the key the object was built under and what the engine reads from a
module's lowered unit besides its object, so a module whose object is reused
never has to lower. the object itself is what mink reads back from the file.
the layout is `"MCRB"`, the 32-byte key, the 32-byte digest of the
source its back half's warnings are placed in, zero when it holds none, the test count, then per test its qualified name and
its line, then the length and bytes of the module's warnings (compile/cache
encodes them), then the count of the instances the module's code asks for and
per instance its symbol, its reach and the offset and length of where the
module's source asks for it, then the count of the symbols the module defines
for its test object to declare and per symbol its name, then the count of the
instances the object holds a copy of and per instance its symbol, its
32-byte key and the count and names of the modules it reads, every number a
little-endian u32 and every name its length then its bytes

<sub>source: [`src/compile/cache/record.mach`](../../../../src/compile/cache/record.mach) · imported by 5 modules</sub>

<sub>`intern` is [`mach.base.intern`](../../base/intern.md) · `A` is `std.allocator` · `fail` is [`mach.base.fail`](../../base/fail.md) · `base_source` is [`mach.base.source`](../../base/source.md)</sub>

| Type | | |
| :--- | :--- | :--- |
| [`TestFact`](#testfact) | `rec` |  |
| [`Facts`](#facts) | `rec` |  |
| [`Copy`](#copy) | `rec` | an instance the object holds a copy of: its symbol, its key (compile/cache, instance_key) and the modules it reads, the `count` names of `copy_reads` from `first`, its declaring module first |
| [`Asked`](#asked) | `rec` | an instance the module's code asks for: its symbol, the objects the module's code reaches it from (a sema_instance.Reach) and where in the module's source it is asked for |
| [`Record`](#record) | `rec` |  |

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
    name: intern.StrId;
    line: u32;
}
```

## Facts

```mach
pub rec Facts {
    # the digest of the source the object's back half's warnings are placed in, all
    # zero when it reported none: an instance unit is keyed without its origin's
    # other code, so it is reused across an edit there, and its recorded warnings
    # stand only for the source they were reported in
    source:     [32]u8;
    tests:      *TestFact;
    test_count: u32;
    # the diagnostics short of errors building the module reported, which a
    # build that reuses the object reports again
    warnings:     *u8;
    warnings_len: u32;
    # every instance the module's code asks for, as its typing found them: a build that
    # skips the module's front end reads its demand here. a test object's record holds none
    asked:       *Asked;
    asked_count: u32;
    # every symbol the module defines under a binding other objects see, which its test object
    # declares instead: kept for a module that declares tests, none otherwise
    defined:       *intern.StrId;
    defined_count: u32;
    # every instance the object holds a copy of, which constant-time code calls, under the key it
    # had when the object was built: the object stands only while each keys the same
    copies:      *Copy;
    copy_count:  u32;
    copy_reads:  *intern.StrId;
    reads_count: u32;
}
```

### facts_dnit

```mach
pub fun facts_dnit(alloc: *A.Allocator, f: *Facts)
```

release recorded facts; nil facts hold nothing

### encode

```mach
pub fun encode(alloc: *A.Allocator, itn: *intern.Interner, key: *[32]u8, facts: *Facts,
    out_bytes: **u8, out_len: *u32) err[fail.Fail]
```

the record for an object built under `key` whose module lowered to `facts`;
the caller owns the bytes, allocated in alloc

## Copy

```mach
pub rec Copy {
    name:  intern.StrId;
    key:   [32]u8;
    first: u32;
    count: u32;
}
```

an instance the object holds a copy of: its symbol, its key (compile/cache, instance_key) and the
modules it reads, the `count` names of `copy_reads` from `first`, its declaring module first

## Asked

```mach
pub rec Asked {
    name:  intern.StrId;
    reach: u8;
    span:  base_source.Span;
}
```

an instance the module's code asks for: its symbol, the objects the module's code reaches it
from (a sema_instance.Reach) and where in the module's source it is asked for

## Record

```mach
pub rec Record {
    key:   [32]u8;
    facts: Facts;
}
```

### record_dnit

```mach
pub fun record_dnit(alloc: *A.Allocator, r: *Record)
```

release a record; a nil record holds nothing

### decode

```mach
pub fun decode(alloc: *A.Allocator, itn: *intern.Interner, bytes: *u8, len: usize) res[opt[Record], fail.Fail]
```

the record in `bytes`, or none when it is missing, truncated or malformed:
an object whose record cannot be read is not reused. err only when
allocation fails

