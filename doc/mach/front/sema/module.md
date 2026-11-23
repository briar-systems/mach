[mach](../../README.md) / [front](../README.md) / [sema](README.md) / **module**

# mach.front.sema.module

typing one module: its declarations and bodies, the instances they ask for, and the result it
publishes; and typing one instance on its own (instance_sema)

Source: [`src/front/sema/module.mach`](../../../../src/front/sema/module.mach)

Uses types from [`mach.base.diagnostic`](../../base/diagnostic.md), [`mach.base.fail`](../../base/fail.md), [`mach.front.ast`](../ast.md), [`mach.front.comptime.scope`](../comptime/scope.md), [`mach.front.resolve`](../resolve.md), [`mach.front.sema.instance`](instance.md), [`mach.front.sema.product`](product.md), [`mach.session`](../../session.md), [`mach.session.module`](../../session/module.md).

| Functions | |
| :--- | :--- |
| [`sema`](#sema) | type a module in a scope over the load's, `load`, and resolve's bindings in `rr`, which sema reads and never writes; what sema binds is its result's |
| [`instance_sema`](#instance_sema) | types the instance `item` on its own: a generic under its type arguments, a function with comptime value parameters with them bound, a function with a pack over its element types |

---

## Functions

### sema

```mach
pub fun sema(
    s: *mach.session.Session,
    a: *mach.front.ast.Ast,
    rr: *mach.front.resolve.ResolveResult,
    deps: *mach.front.sema.product.SemaDeps,
    load: *mach.front.comptime.scope.ComptimeCtx,
    own_module: mach.session.module.ModuleId,
    diags: *mach.base.diagnostic.DiagnosticStore) res[mach.front.sema.product.SemaResult, mach.base.fail.Fail]
```

type a module in a scope over the load's, `load`, and resolve's bindings in `rr`, which sema
reads and never writes; what sema binds is its result's

### instance_sema

```mach
pub fun instance_sema(s: *mach.session.Session, item: *mach.front.sema.instance.Instance, deps: *mach.front.sema.product.SemaDeps) res[mach.front.sema.product.InstanceResult, mach.base.fail.Fail]
```

types the instance `item` on its own: a generic under its type arguments, a function with
comptime value parameters with them bound, a function with a pack over its element types. it
reads its declaring module's typed definition and the surfaces `deps` holds, and writes only
its own result: its frames, the instances its body asks for after it in a set of its own,
the constants its body builds, and its reports, those it would make where it is asked for
placed nowhere. who asked for it is no part of its typing, so one typing serves every user

