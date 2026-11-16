[mach](../../README.md) / [compile](../README.md) / [cache](README.md) / **key**

# mach.compile.cache.key

the object cache's key builder, and the keys of a module's normal and test objects

<sub>source: [`src/compile/cache/key.mach`](../../../../src/compile/cache/key.mach) · imported by 2 modules</sub>

<sub>`wire` is [`mach.base.wire`](../../base/wire.md) · `intern` is [`mach.base.intern`](../../base/intern.md) · `fail` is [`mach.base.fail`](../../base/fail.md)</sub>

| Type | | |
| :--- | :--- | :--- |
| [`Builder`](#builder) | `rec` | a cache key: the sha-256 of a domain then each input, every number a u64 and every byte run its length then its bytes |

| On `Builder` | |
| :--- | :--- |
| [`begin`](#begin) · [`number`](#number) · [`bytes`](#bytes) · [`text`](#text) · [`name`](#name) · [`finish`](#finish) | |

| Functions | |
| :--- | :--- |
| [`equal`](#equal) | |
| [`object`](#object) | the key of a module's object: the module's key and its name |
| [`test_object`](#test_object) | the key of a module's test object: the key of the normal object it is lowered against, which covers the module's whole source, tests included |

---

## Builder

```mach
pub rec Builder {
    sink: wire.Sink;
}
```

a cache key: the sha-256 of a domain then each input, every number a u64
and every byte run its length then its bytes

### begin

```mach
pub fun begin(domain: str) Builder
```

### number

```mach
pub fun number(b: *Builder, value: u64)
```

### bytes

```mach
pub fun bytes(b: *Builder, data: *u8, len: usize)
```

### text

```mach
pub fun text(b: *Builder, value: str)
```

### name

```mach
pub fun name(b: *Builder, itn: *intern.Interner, id: intern.StrId)
```

### finish

```mach
pub fun finish(b: *Builder, digest: *[32]u8) err[fail.Fail]
```

## Functions

### equal

```mach
pub fun equal(left: *[32]u8, right: *[32]u8) bool
```

### object

```mach
pub fun object(module_key: *[32]u8, module: str, out: *[32]u8) err[fail.Fail]
```

the key of a module's object: the module's key and its name

### test_object

```mach
pub fun test_object(object_key: *[32]u8, out: *[32]u8) err[fail.Fail]
```

the key of a module's test object: the key of the normal object it is lowered
against, which covers the module's whole source, tests included

