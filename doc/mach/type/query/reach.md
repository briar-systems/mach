[mach](../../README.md) / [type](../README.md) / [query](README.md) / **reach**

# mach.type.query.reach

what a type holds or reaches, each property a leaf predicate over the one type walk

<sub>source: [`src/type/query/reach.mach`](../../../../src/type/query/reach.mach) · imported by 4 modules</sub>

<sub>`session` is [`mach.session`](../../session.md) · `type_kind` is [`mach.type.kind`](../kind.md) · `fail` is [`mach.base.fail`](../../base/fail.md) · `type_query` is [`mach.type.query`](../query.md)</sub>

| Functions | |
| :--- | :--- |
| [`module_secret`](#module_secret) | whether any of a module's `len` resolved types reaches a secret |
| [`tag_value`](#tag_value) | whether `tid` holds a tag value in its own storage |
| [`vector`](#vector) | whether `tid` holds a vector in its own storage, with the field tables built so far |
| [`generic_union`](#generic_union) | whether a generic union is reachable from `nominal`, through any member, element, pointee, parameter, argument or declaration a mention names |

---

## Functions

### module_secret

```mach
pub fun module_secret(s: *session.Session, resolved: *type_kind.TypeId, len: u32) res[bool, fail.Fail]
```

whether any of a module's `len` resolved types reaches a secret

### tag_value

```mach
pub fun tag_value(s: *session.Session, tid: type_kind.TypeId) res[bool, fail.Fail]
```

whether `tid` holds a tag value in its own storage

### vector

```mach
pub fun vector(q: *type_query.Query, tid: type_kind.TypeId) bool
```

whether `tid` holds a vector in its own storage, with the field tables built so far

### generic_union

```mach
pub fun generic_union(q: *type_query.Query, nominal: type_kind.TypeId) bool
```

whether a generic union is reachable from `nominal`, through any member, element,
pointee, parameter, argument or declaration a mention names

