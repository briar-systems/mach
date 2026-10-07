# mach.lang.type.query.reach

what a type holds or reaches, each property a leaf predicate over the one type walk

## fun module_secret

```mach
pub fun module_secret(s: *session.Session, resolved: *type.TypeId, len: u32) res[bool, fail.Fail];
```

whether any of a module's `len` resolved types reaches a secret

## fun tag_value

```mach
pub fun tag_value(s: *session.Session, tid: type.TypeId) res[bool, fail.Fail];
```

whether `tid` holds a tag value in its own storage

## fun vector

```mach
pub fun vector(q: *type_query.Query, tid: type.TypeId) bool;
```

whether `tid` holds a vector in its own storage, with the field tables built so far

## fun generic_union

```mach
pub fun generic_union(q: *type_query.Query, nominal: type.TypeId) bool;
```

whether a generic union is reachable from `nominal`, through any member, element,
pointee, parameter, argument or declaration a mention names

