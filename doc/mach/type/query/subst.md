[mach](../../README.md) / [type](../README.md) / [query](README.md) / **subst**

# mach.type.query.subst

a generic's body under type arguments: substitution, and the field table an instance reads

Source: [`src/type/query/subst.mach`](../../../../src/type/query/subst.mach)

Uses types from [`mach.base.fail`](../../base/fail.md), [`mach.session`](../../session.md), [`mach.type`](../../type.md), [`mach.type.kind`](../kind.md), [`mach.type.query`](../query.md).

| Functions | |
| :--- | :--- |
| [`apply`](#apply) | `body_type` with `who`'s parameters replaced by `args` |
| [`ensure`](#ensure) | builds the field table of instance `tid` for the current projection, once |
| [`apply_or_self`](#apply_or_self) | `tid` under `who`'s arguments, or itself when there are none |
| [`substitute`](#substitute) | replace `who`'s parameters in `body_type` with `args`, position for position; a parameter of any other declaration (an enclosing generic's, in an open instance) stays, so a substituted type reads as the identity under a second pass |
| [`ensure_fields`](#ensure_fields) | |

---

## Functions

### apply

```mach
pub fun apply(q: *mach.type.query.Query, body_type: mach.type.kind.TypeId, who: *mach.type.GenericOwner, args: *mach.type.kind.TypeId, arg_count: u32) mach.type.kind.TypeId
```

`body_type` with `who`'s parameters replaced by `args`. the walk recurses as deep as the type
nests, so it asks for stack before it descends, and a type nested deeper refuses the query
(`type_query.descend`) and reads as the error type

### ensure

```mach
pub fun ensure(q: *mach.type.query.Query, tid: mach.type.kind.TypeId)
```

builds the field table of instance `tid` for the current projection, once

### apply_or_self

```mach
pub fun apply_or_self(q: *mach.type.query.Query, tid: mach.type.kind.TypeId, who: *mach.type.GenericOwner, args: *mach.type.kind.TypeId, arg_len: u32) mach.type.kind.TypeId
```

`tid` under `who`'s arguments, or itself when there are none

### substitute

```mach
pub fun substitute(s: *mach.session.Session, body_type: mach.type.kind.TypeId, who: *mach.type.GenericOwner, args: *mach.type.kind.TypeId, arg_count: u32) res[mach.type.kind.TypeId, mach.base.fail.Fail]
```

replace `who`'s parameters in `body_type` with `args`, position for position;
a parameter of any other declaration (an enclosing generic's, in an open
instance) stays, so a substituted type reads as the identity under a second pass

### ensure_fields

```mach
pub fun ensure_fields(s: *mach.session.Session, tid: mach.type.kind.TypeId) err[mach.base.fail.Fail]
```

