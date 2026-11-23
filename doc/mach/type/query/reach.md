[mach](../../README.md) / [type](../README.md) / [query](README.md) / **reach**

# mach.type.query.reach

what a type holds or reaches, each property a leaf predicate over the one type walk

Source: [`src/type/query/reach.mach`](../../../../src/type/query/reach.mach)

Uses types from [`mach.base.fail`](../../base/fail.md), [`mach.session`](../../session.md), [`mach.type.kind`](../kind.md), [`mach.type.query`](../query.md).

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
pub fun module_secret(s: *mach.session.Session, resolved: *mach.type.kind.TypeId, len: u32) res[bool, mach.base.fail.Fail]
```

whether any of a module's `len` resolved types reaches a secret

### tag_value

```mach
pub fun tag_value(s: *mach.session.Session, tid: mach.type.kind.TypeId) res[bool, mach.base.fail.Fail]
```

whether `tid` holds a tag value in its own storage

### vector

```mach
pub fun vector(q: *mach.type.query.Query, tid: mach.type.kind.TypeId) bool
```

whether `tid` holds a vector in its own storage, with the field tables built so far

### generic_union

```mach
pub fun generic_union(q: *mach.type.query.Query, nominal: mach.type.kind.TypeId) bool
```

whether a generic union is reachable from `nominal`, through any member, element,
pointee, parameter, argument or declaration a mention names

