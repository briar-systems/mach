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
| [`begin`](#begin) | start a key in a domain, the domain hashed as the first input |
| [`number`](#number) | add a u64 input to the key |
| [`bytes`](#bytes) | add a byte run to the key as its length then its bytes a nonempty run with nil bytes marks the builder failed, reported by finish |
| [`text`](#text) | add a string to the key, nil and present text hashed apart by a leading 0 or 1 |
| [`name`](#name) | add an interned name to the key as its text, a nil id hashed as nil text a name missing from the interner marks the builder failed, reported by finish |
| [`finish`](#finish) | finish the key into its 32-byte digest |

| Functions | |
| :--- | :--- |
| [`equal`](#equal) | compare two 32-byte keys |
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

start a key in a domain, the domain hashed as the first input

| Parameter | |
| :--- | :--- |
| `domain` | the text naming what the key is for |
| **returns** | a builder holding the running digest |

### number

```mach
pub fun number(b: *Builder, value: u64)
```

add a u64 input to the key

| Parameter | |
| :--- | :--- |
| `b` | the key being built |
| `value` | the number to add |

### bytes

```mach
pub fun bytes(b: *Builder, data: *u8, len: usize)
```

add a byte run to the key as its length then its bytes
a nonempty run with nil bytes marks the builder failed, reported by finish

| Parameter | |
| :--- | :--- |
| `b` | the key being built |
| `data` | the first byte, nil only when len is 0 |
| `len` | the byte count |

### text

```mach
pub fun text(b: *Builder, value: str)
```

add a string to the key, nil and present text hashed apart by a leading 0 or 1

| Parameter | |
| :--- | :--- |
| `b` | the key being built |
| `value` | the nul-terminated text, or nil |

### name

```mach
pub fun name(b: *Builder, itn: *mach.base.intern.Interner, id: mach.base.intern.StrId)
```

add an interned name to the key as its text, a nil id hashed as nil text
a name missing from the interner marks the builder failed, reported by finish

| Parameter | |
| :--- | :--- |
| `b` | the key being built |
| `itn` | the interner holding the name |
| `id` | the name's id |

### finish

```mach
pub fun finish(b: *Builder, digest: *[32]u8) err[mach.base.fail.Fail]
```

finish the key into its 32-byte digest

| Parameter | |
| :--- | :--- |
| `b` | the key being built |
| `digest` | where the digest is written |
| **returns** | the builder's first refusal or a missing destination as an error, digest untouched |

## Functions

### equal

```mach
pub fun equal(left: *[32]u8, right: *[32]u8) bool
```

compare two 32-byte keys

| Parameter | |
| :--- | :--- |
| `left` | the first key |
| `right` | the second key |
| **returns** | true when every byte matches |

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

