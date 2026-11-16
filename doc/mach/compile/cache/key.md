[mach](../../README.md) / [compile](../README.md) / [cache](README.md) / **key**

# mach.compile.cache.key

the object cache's key builder, and the keys of a module's normal and test objects

Source: [`src/compile/cache/key.mach`](../../../../src/compile/cache/key.mach)

Uses types from [`mach.base.fail`](../../base/fail.md), [`mach.base.intern`](../../base/intern.md), [`mach.base.wire`](../../base/wire.md).

| Type | | |
| :--- | :--- | :--- |
| [`Builder`](#builder) | `rec` | a cache key: the sha-256 of a domain then each input, every number a u64 and every byte run its length then its bytes |

| On `Builder` | |
| :--- | :--- |
| [`begin`](#begin), [`number`](#number), [`bytes`](#bytes), [`text`](#text), [`name`](#name), [`finish`](#finish) | |

| Functions | |
| :--- | :--- |
| [`equal`](#equal) | |
| [`object`](#object) | the key of a module's object: the module's key and its name |
| [`test_object`](#test_object) | the key of a module's test object: the key of the normal object it is lowered against, which covers the module's whole source, tests included |

---

## Builder

```mach
pub rec Builder {
    sink: mach.base.wire.Sink;
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
pub fun name(b: *Builder, itn: *mach.base.intern.Interner, id: mach.base.intern.StrId)
```

### finish

```mach
pub fun finish(b: *Builder, digest: *[32]u8) err[mach.base.fail.Fail]
```

## Functions

### equal

```mach
pub fun equal(left: *[32]u8, right: *[32]u8) bool
```

### object

```mach
pub fun object(module_key: *[32]u8, module: str, out: *[32]u8) err[mach.base.fail.Fail]
```

the key of a module's object: the module's key and its name

### test_object

```mach
pub fun test_object(object_key: *[32]u8, out: *[32]u8) err[mach.base.fail.Fail]
```

the key of a module's test object: the key of the normal object it is lowered
against, which covers the module's whole source, tests included

