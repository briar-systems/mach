[mach](../README.md) / [type](README.md) / **deprecation**

# mach.type.deprecation

the deprecation notice a declaration or a tag case carries

<sub>source: [`src/type/deprecation.mach`](../../../src/type/deprecation.mach) · imported by 7 modules</sub>

<sub>`base_source` is [`mach.base.source`](../base/source.md) · `intern` is [`mach.base.intern`](../base/intern.md)</sub>

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
    owner:   base_source.FileId;
    message: intern.StrId;
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

