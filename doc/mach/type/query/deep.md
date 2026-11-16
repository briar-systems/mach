[mach](../../README.md) / [type](../README.md) / [query](README.md) / **deep**

# mach.type.query.deep

whether a type reaches a property anywhere, through pointers included, answered over
the declarations so a generic that grows through a pointer still has a finite answer

<sub>source: [`src/type/query/deep.mach`](../../../../src/type/query/deep.mach) · imported by 8 modules</sub>

<sub>`type_kind` is [`mach.type.kind`](../kind.md) · `type_query` is [`mach.type.query`](../query.md) · `session` is [`mach.session`](../../session.md) · `fail` is [`mach.base.fail`](../../base/fail.md)</sub>

| Functions | |
| :--- | :--- |
| [`public_leaf`](#public_leaf) | the kinds that carry no secret of their own; false for every other kind, which a walk descends (a partition, enumerated by its test) |
| [`secret`](#secret) | whether `tid` reaches a secret |
| [`float`](#float) | whether `tid` reaches a floating-point value, a lane of a vector included |
| [`contains_secret`](#contains_secret) · [`contains_float`](#contains_float) | |

---

## Functions

### public_leaf

```mach
pub fun public_leaf(k: type_kind.TypeKind) bool
```

the kinds that carry no secret of their own; false for every other kind, which a
walk descends (a partition, enumerated by its test)

### secret

```mach
pub fun secret(q: *type_query.Query, tid: type_kind.TypeId) bool
```

whether `tid` reaches a secret

### float

```mach
pub fun float(q: *type_query.Query, tid: type_kind.TypeId) bool
```

whether `tid` reaches a floating-point value, a lane of a vector included

### contains_secret

```mach
pub fun contains_secret(s: *session.Session, tid: type_kind.TypeId) res[bool, fail.Fail]
```

### contains_float

```mach
pub fun contains_float(s: *session.Session, tid: type_kind.TypeId) res[bool, fail.Fail]
```

