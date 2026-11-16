[mach](../../README.md) / [front](../README.md) / [sema](README.md) / **surface**

# mach.front.sema.surface

the typed surface a module publishes to its importers: each public symbol's
type and bound constant, and the field recipes of the types they reach

<sub>source: [`src/front/sema/surface.mach`](../../../../src/front/sema/surface.mach) · imported by 1 module</sub>

<sub>`module` is [`mach.session.module`](../../session/module.md) · `intern` is [`mach.base.intern`](../../base/intern.md) · `comptime_scope` is [`mach.front.comptime.scope`](../comptime/scope.md) · `session` is [`mach.session`](../../session.md) · `resolve` is [`mach.front.resolve`](../resolve.md) · `sema_product` is [`mach.front.sema.product`](product.md) · `A` is `std.allocator` · `fail` is [`mach.base.fail`](../../base/fail.md) · `fields` is [`mach.front.sema.fields`](fields.md)</sub>

| Type | | |
| :--- | :--- | :--- |
| [`Owner`](#owner) | `rec` | the module whose surface is built: its ids and its load scope |
| [`Origins`](#origins) | `rec` | where the surfaces of the modules a surface forwards from are read: a loaded module's published surface, owned by the caller |

| On `Owner` | |
| :--- | :--- |
| [`exports`](#exports) | the exports of `owner`'s typed surface, allocated from `a`; a symbol it forwards is read from the surface its origin published |
| [`graph`](#graph) | the field recipes of every type the exports of `surface` reach, under `owner`'s target |

---

## Owner

```mach
pub rec Owner {
    module: module.ModuleId;
    stable: module.StableModuleId;
    path:   intern.StrId;
    ctx:    *comptime_scope.ComptimeCtx;
}
```

the module whose surface is built: its ids and its load scope

### exports

```mach
pub fun exports(s: *session.Session, owner: *Owner, rr: *resolve.ResolveResult, sr: *sema_product.SemaResult,
    origins: *Origins, a: *A.Allocator) res[sema_product.ModuleSema, fail.Fail]
```

the exports of `owner`'s typed surface, allocated from `a`; a symbol it forwards is read
from the surface its origin published

### graph

```mach
pub fun graph(s: *session.Session, owner: *Owner, surface: *sema_product.ModuleSema) res[fields.Graph, fail.Fail]
```

the field recipes of every type the exports of `surface` reach, under `owner`'s target

## Origins

```mach
pub rec Origins {
    ctx:  ptr;
    read: fun(ptr, module.ModuleId, *A.Allocator) res[sema_product.ModuleSema, fail.Fail];
}
```

where the surfaces of the modules a surface forwards from are read: a loaded module's
published surface, owned by the caller

