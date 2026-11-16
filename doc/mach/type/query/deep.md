[mach](../../README.md) / [type](../README.md) / [query](README.md) / **deep**

# mach.type.query.deep

whether a type reaches a property anywhere, through pointers included, answered over
the declarations so a generic that grows through a pointer still has a finite answer

Source: [`src/type/query/deep.mach`](../../../../src/type/query/deep.mach)

Uses types from [`mach.base.fail`](../../base/fail.md), [`mach.session`](../../session.md), [`mach.type.kind`](../kind.md), [`mach.type.query`](../query.md).

| Functions | |
| :--- | :--- |
| [`public_leaf`](#public_leaf) | the kinds that carry no secret of their own; false for every other kind, which a walk descends (a partition, enumerated by its test) |
| [`secret`](#secret) | whether `tid` reaches a secret |
| [`float`](#float) | whether `tid` reaches a floating-point value, a lane of a vector included |
| [`contains_secret`](#contains_secret), [`contains_float`](#contains_float) | |

---

## Functions

### public_leaf

```mach
pub fun public_leaf(k: mach.type.kind.TypeKind) bool
```

the kinds that carry no secret of their own; false for every other kind, which a
walk descends (a partition, enumerated by its test)

### secret

```mach
pub fun secret(q: *mach.type.query.Query, tid: mach.type.kind.TypeId) bool
```

whether `tid` reaches a secret

### float

```mach
pub fun float(q: *mach.type.query.Query, tid: mach.type.kind.TypeId) bool
```

whether `tid` reaches a floating-point value, a lane of a vector included

### contains_secret

```mach
pub fun contains_secret(s: *mach.session.Session, tid: mach.type.kind.TypeId) res[bool, mach.base.fail.Fail]
```

### contains_float

```mach
pub fun contains_float(s: *mach.session.Session, tid: mach.type.kind.TypeId) res[bool, mach.base.fail.Fail]
```

