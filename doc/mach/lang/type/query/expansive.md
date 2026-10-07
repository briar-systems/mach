# mach.lang.type.query.expansive

whether a generic has unboundedly many instances, decided on the first question about
it once its fields exist and recorded for the projection it was decided under

the instantiation graph is over declarations: a node is one parameter of one generic,
and an edge says that parameter flows into an argument of a generic its fields mention.
an edge grows when the argument wraps the parameter (`L[*T]` in `L[T]`) rather than
being it, `^T` included as the parameter itself since `^` is idempotent. a cycle with a
growing edge mints a fresh instance at every trip, so its generics are expansive

## fun nominal

```mach
pub fun nominal(q: *type_query.Query, generic: type.TypeId) bool;
```

whether generic nominal `generic` lies on an instantiation cycle that grows an
argument, so it has unboundedly many instances; true, with nothing recorded, when a
generic the cycle could pass through has no fields yet

## fun holds

```mach
pub fun holds(q: *type_query.Query, tid: type.TypeId) bool;
```

whether `tid` holds, by value, an instance of an expansive generic

## fun instance_is

```mach
pub fun instance_is(types: *type.TypeInterner, facts: *type_query.Facts, tid: type.TypeId) res[bool, fail.Fail];
```

whether `tid` is an instance of an expansive generic

## fun holds_instance

```mach
pub fun holds_instance(types: *type.TypeInterner, facts: *type_query.Facts, tid: type.TypeId) res[bool, fail.Fail];
```

whether `tid` holds, by value, an instance of an expansive generic

