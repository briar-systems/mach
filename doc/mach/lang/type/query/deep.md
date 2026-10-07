# mach.lang.type.query.deep

whether a type reaches a property anywhere, through pointers included, answered over
the declarations so a generic that grows through a pointer still has a finite answer

## fun public_leaf

```mach
pub fun public_leaf(k: type.TypeKind) bool;
```

the kinds that carry no secret of their own; false for every other kind, which a
walk descends (a partition, enumerated by its test)

## fun secret

```mach
pub fun secret(q: *type_query.Query, tid: type.TypeId) bool;
```

whether `tid` reaches a secret

## fun float

```mach
pub fun float(q: *type_query.Query, tid: type.TypeId) bool;
```

whether `tid` reaches a floating-point value, a lane of a vector included

## fun contains_secret

```mach
pub fun contains_secret(s: *session.Session, tid: type.TypeId) res[bool, fail.Fail];
```

## fun contains_float

```mach
pub fun contains_float(s: *session.Session, tid: type.TypeId) res[bool, fail.Fail];
```

