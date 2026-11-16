[mach](../README.md) / [type](README.md) / **deprecation**

# mach.type.deprecation

the deprecation notice a declaration or a tag case carries

Source: [`src/type/deprecation.mach`](../../../src/type/deprecation.mach)

Uses types from [`mach.base.intern`](../base/intern.md), [`mach.base.source`](../base/source.md).

| Type | | |
| :--- | :--- | :--- |
| [`Deprecation`](#deprecation) | `rec` | a deprecation notice recorded on a declaration or a tag case: the file that owns the annotation (so it never warns on itself) and the optional message |

| On `Deprecation` | |
| :--- | :--- |
| [`none`](#none) | |
| [`equal`](#equal) | an absent notice is one value whatever its other fields hold, so a zero-filled record and `none()` compare equal |

---

## Deprecation

```mach
pub rec Deprecation {
    present: bool;
    owner:   mach.base.source.FileId;
    message: mach.base.intern.StrId;
}
```

a deprecation notice recorded on a declaration or a tag case: the file that owns the
annotation (so it never warns on itself) and the optional message

### none

```mach
pub fun none() Deprecation
```

### equal

```mach
pub fun equal(a: Deprecation, b: Deprecation) bool
```

an absent notice is one value whatever its other fields hold, so a zero-filled record
and `none()` compare equal

