[mach](../../README.md) / [compile](../README.md) / [cache](README.md) / **compiler**

# mach.compile.cache.compiler

the running compiler's identity: the sha-256 of its executable file, once per process

<sub>source: [`src/compile/cache/compiler.mach`](../../../../src/compile/cache/compiler.mach) · imported by 2 modules</sub>

<sub>`A` is `std.allocator`</sub>

| Type | | |
| :--- | :--- | :--- |
| [`Error`](#error) | `rec` |  |
| [`Identity`](#identity) | `rec` | the sha-256 of the running image's file |

| On `Identity` | |
| :--- | :--- |
| [`identity_clear`](#identity_clear) | |
| [`identity`](#identity-1) | identifies the current executable, never argv[0], once per process, by the sha-256 of its file success fills the identity, failure leaves the output and the memo untouched |

---

## Error

```mach
pub rec Error { kind: u8; message: str; }
```

## Identity

```mach
pub rec Identity { bytes: [32]u8; len: usize; }
```

the sha-256 of the running image's file

### identity_clear

```mach
pub fun identity_clear(id: *Identity)
```

### identity

```mach
pub fun identity(alloc: *A.Allocator, out: *Identity) err[Error]
```

identifies the current executable, never argv[0], once per process, by the
sha-256 of its file
success fills the identity, failure leaves the output and the memo untouched

