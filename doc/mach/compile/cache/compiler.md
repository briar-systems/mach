[mach](../../README.md) / [compile](../README.md) / [cache](README.md) / **compiler**

# mach.compile.cache.compiler

the running compiler's identity: the sha-256 of its executable file, once per process

Source: [`src/compile/cache/compiler.mach`](../../../../src/compile/cache/compiler.mach)

| Type | | |
| :--- | :--- | :--- |
| [`Error`](#error) | `rec` | why the compiler identity could not be read |
| [`Identity`](#identity) | `rec` | the sha-256 of the running image's file |

| On `Identity` | |
| :--- | :--- |
| [`identity_clear`](#identity_clear) | zero an identity's digest bytes and length |
| [`identity`](#identity-1) | identifies the current executable, never argv[0], once per process, by the sha-256 of its file success fills the identity, failure leaves the output and the memo untouched |

---

## Error

```mach
pub rec Error { kind: u8; message: str; }
```

why the compiler identity could not be read

| Field | Type | |
| :--- | :--- | :--- |
| `kind` | `u8` | the host fault kind |
| `message` | `str` | what went wrong |

## Identity

```mach
pub rec Identity { bytes: [32]u8; len: usize; }
```

the sha-256 of the running image's file

### identity_clear

```mach
pub fun identity_clear(id: *Identity)
```

zero an identity's digest bytes and length

| Parameter | |
| :--- | :--- |
| `id` | the identity to clear |

### identity

```mach
pub fun identity(alloc: *std.allocator.Allocator, out: *Identity) err[Error]
```

identifies the current executable, never argv[0], once per process, by the
sha-256 of its file
success fills the identity, failure leaves the output and the memo untouched

