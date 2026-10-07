# mach.lang.fe.sema.surface

the typed surface a module publishes to its importers: each public symbol's
type and bound constant, and the field recipes of the types they reach

## rec Owner

```mach
pub rec Owner;
```

the module whose surface is built: its ids and its load scope

## rec Origins

```mach
pub rec Origins;
```

where the surfaces of the modules a surface forwards from are read: the stable id of a
loaded module, nil when it is not loaded, and its published surface, owned by the caller

## fun exports

```mach
pub fun exports(s: *session.Session, owner: *Owner, rr: *resolve.ResolveResult, sr: *sema_product.SemaResult,
origins: *Origins, a: *A.Allocator) res[sema_product.ModuleSema, fail.Fail];
```

the exports of `owner`'s typed surface, allocated from `a`; a symbol it forwards is read
from the surface its origin published

## fun graph

```mach
pub fun graph(s: *session.Session, owner: *Owner, surface: *sema_product.ModuleSema) res[fields.Graph, fail.Fail];
```

the field recipes of every type the exports of `surface` reach, under `owner`'s target

