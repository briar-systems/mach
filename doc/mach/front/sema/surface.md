[mach](../../README.md) / [front](../README.md) / [sema](README.md) / **surface**

# mach.front.sema.surface

the typed surface a module publishes to its importers: each public symbol's
type and bound constant, and the field recipes of the types they reach

Source: [`src/front/sema/surface.mach`](../../../../src/front/sema/surface.mach)

Uses types from [`mach.base.fail`](../../base/fail.md), [`mach.base.intern`](../../base/intern.md), [`mach.front.comptime.scope`](../comptime/scope.md), [`mach.front.resolve`](../resolve.md), [`mach.front.sema.fields`](fields.md), [`mach.front.sema.product`](product.md), [`mach.session`](../../session.md), [`mach.session.module`](../../session/module.md).

| Type | | |
| :--- | :--- | :--- |
| [`Owner`](#owner) | `rec` | the module whose surface is built: its ids and its load scope |
| [`Origins`](#origins) | `rec` | where the surfaces of the modules a surface forwards from are read: a loaded module's published surface, owned by the caller |

| On `Owner` | |
| :--- | :--- |
| [`exports`](#exports) | the exports of `owner`'s typed surface, allocated from `a`; a symbol it forwards is read from the surface its origin published |

| Functions | |
| :--- | :--- |
| [`graph`](#graph) | the field recipes of every type the exports of `surface` reach, and of the type each exported constant names, read from the session's type table |

---

## Owner

```mach
pub rec Owner {
    module: mach.session.module.ModuleId;
    stable: mach.session.module.StableModuleId;
    path:   mach.base.intern.StrId;
    ctx:    *mach.front.comptime.scope.ComptimeCtx;
}
```

the module whose surface is built: its ids and its load scope

### exports

```mach
pub fun exports(s: *mach.session.Session, owner: *Owner, rr: *mach.front.resolve.ResolveResult, sr: *mach.front.sema.product.SemaResult,
    origins: *Origins, a: *std.allocator.Allocator) res[mach.front.sema.product.ModuleSema, mach.base.fail.Fail]
```

the exports of `owner`'s typed surface, allocated from `a`; a symbol it forwards is read
from the surface its origin published

## Origins

```mach
pub rec Origins {
    ctx:  ptr;
    read: fun(ptr, mach.session.module.ModuleId, *std.allocator.Allocator) res[mach.front.sema.product.ModuleSema, mach.base.fail.Fail];
}
```

where the surfaces of the modules a surface forwards from are read: a loaded module's
published surface, owned by the caller

## Functions

### graph

```mach
pub fun graph(s: *mach.session.Session, surface: *mach.front.sema.product.ModuleSema) res[mach.front.sema.fields.Graph, mach.base.fail.Fail]
```

the field recipes of every type the exports of `surface` reach, and of the type each
exported constant names, read from the session's type table

