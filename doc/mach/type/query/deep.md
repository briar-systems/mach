[mach](../../README.md) / [type](../README.md) / [query](README.md) / **deep**

# mach.type.query.deep

whether a type reaches a secret anywhere, through pointers included, answered over
the declarations so a generic that grows through a pointer still has a finite answer

Source: [`src/type/query/deep.mach`](../../../../src/type/query/deep.mach)

Uses types from [`mach.base.fail`](../../base/fail.md), [`mach.session`](../../session.md), [`mach.type.kind`](../kind.md), [`mach.type.query`](../query.md).

| Functions | |
| :--- | :--- |
| [`public_leaf`](#public_leaf) | the kinds that carry no secret of their own; false for every other kind, which a walk descends (a partition, enumerated by its test) |
| [`secret`](#secret) | whether `tid` reaches a secret |
| [`contains_secret`](#contains_secret) | whether `tid` reaches a secret anywhere, pointers included, asked as a query of its own |

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

### contains_secret

```mach
pub fun contains_secret(s: *mach.session.Session, tid: mach.type.kind.TypeId) res[bool, mach.base.fail.Fail]
```

whether `tid` reaches a secret anywhere, pointers included, asked as a query of its own

| Parameter | |
| :--- | :--- |
| `s` | the session holding the store and fact cache the query uses |
| `tid` | the type |
| **returns** | the answer, or the internal failure or refusal the query met |

