[mach](../../README.md) / [type](../README.md) / [query](README.md) / **subst**

# mach.type.query.subst

a generic's body under type arguments: substitution, and the field table an instance reads

<sub>source: [`src/type/query/subst.mach`](../../../../src/type/query/subst.mach) · imported by 13 modules</sub>

<sub>`type_query` is [`mach.type.query`](../query.md) · `type_kind` is [`mach.type.kind`](../kind.md) · `type` is [`mach.type`](../../type.md) · `session` is [`mach.session`](../../session.md) · `fail` is [`mach.base.fail`](../../base/fail.md)</sub>

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
pub fun apply(q: *type_query.Query, body_type: type_kind.TypeId, who: *type.GenericOwner, args: *type_kind.TypeId, arg_count: u32) type_kind.TypeId
```

`body_type` with `who`'s parameters replaced by `args`. the walk recurses as deep as the type
nests, so it asks for stack before it descends, and a type nested deeper refuses the query
(`type_query.descend`) and reads as the error type

### ensure

```mach
pub fun ensure(q: *type_query.Query, tid: type_kind.TypeId)
```

builds the field table of instance `tid` for the current projection, once

### apply_or_self

```mach
pub fun apply_or_self(q: *type_query.Query, tid: type_kind.TypeId, who: *type.GenericOwner, args: *type_kind.TypeId, arg_len: u32) type_kind.TypeId
```

`tid` under `who`'s arguments, or itself when there are none

### substitute

```mach
pub fun substitute(s: *session.Session, body_type: type_kind.TypeId, who: *type.GenericOwner, args: *type_kind.TypeId, arg_count: u32) res[type_kind.TypeId, fail.Fail]
```

replace `who`'s parameters in `body_type` with `args`, position for position;
a parameter of any other declaration (an enclosing generic's, in an open
instance) stays, so a substituted type reads as the identity under a second pass

### ensure_fields

```mach
pub fun ensure_fields(s: *session.Session, tid: type_kind.TypeId) err[fail.Fail]
```

