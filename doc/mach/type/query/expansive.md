[mach](../../README.md) / [type](../README.md) / [query](README.md) / **expansive**

# mach.type.query.expansive

whether a generic has unboundedly many instances, decided on the first question about
it once its fields exist and recorded for the projection it was decided under

the instantiation graph is over declarations: a node is one parameter of one generic,
and an edge says that parameter flows into an argument of a generic its fields mention.
an edge grows when the argument wraps the parameter (`L[*T]` in `L[T]`) rather than
being it, `^T` included as the parameter itself since `^` is idempotent. a cycle with a
growing edge mints a fresh instance at every trip, so its generics are expansive

Source: [`src/type/query/expansive.mach`](../../../../src/type/query/expansive.mach)

Uses types from [`mach.base.fail`](../../base/fail.md), [`mach.type`](../../type.md), [`mach.type.kind`](../kind.md), [`mach.type.query`](../query.md).

| Functions | |
| :--- | :--- |
| [`nominal`](#nominal) | whether generic nominal `generic` lies on an instantiation cycle that grows an argument, so it has unboundedly many instances; true, with nothing recorded, when a generic the cycle could pass through has no fields yet |
| [`holds`](#holds) | whether `tid` holds, by value, an instance of an expansive generic |
| [`instance_is`](#instance_is) | whether `tid` is an instance of an expansive generic |

---

## Functions

### nominal

```mach
pub fun nominal(q: *mach.type.query.Query, generic: mach.type.kind.TypeId) bool
```

whether generic nominal `generic` lies on an instantiation cycle that grows an
argument, so it has unboundedly many instances; true, with nothing recorded, when a
generic the cycle could pass through has no fields yet

### holds

```mach
pub fun holds(q: *mach.type.query.Query, tid: mach.type.kind.TypeId) bool
```

whether `tid` holds, by value, an instance of an expansive generic

### instance_is

```mach
pub fun instance_is(types: *mach.type.TypeInterner, facts: *mach.type.query.Facts, tid: mach.type.kind.TypeId) res[bool, mach.base.fail.Fail]
```

whether `tid` is an instance of an expansive generic

