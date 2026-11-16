[mach](../../README.md) / [front](../README.md) / [sema](README.md) / **module**

# mach.front.sema.module

typing one module: its declarations and bodies, the instances they ask for, and the result it
publishes; and typing one instance on its own (instance_sema)

<sub>source: [`src/front/sema/module.mach`](../../../../src/front/sema/module.mach) · imported by 3 modules</sub>

<sub>`session` is [`mach.session`](../../session.md) · `ast` is [`mach.front.ast`](../ast.md) · `resolve` is [`mach.front.resolve`](../resolve.md) · `sema_product` is [`mach.front.sema.product`](product.md) · `comptime_scope` is [`mach.front.comptime.scope`](../comptime/scope.md) · `module` is [`mach.session.module`](../../session/module.md) · `diagnostic` is [`mach.base.diagnostic`](../../base/diagnostic.md) · `fail` is [`mach.base.fail`](../../base/fail.md) · `sema_instance` is [`mach.front.sema.instance`](instance.md)</sub>

| Functions | |
| :--- | :--- |
| [`sema`](#sema) | type a module in a scope over the load's, `load`, and resolve's bindings in `rr`, which sema reads and never writes; what sema binds is its result's |
| [`instance_sema`](#instance_sema) | types the instance `item` on its own: a generic under its type arguments, a function with comptime value parameters with them bound, a function with a pack over its element types |

---

## Functions

### sema

```mach
pub fun sema(
    s: *session.Session,
    a: *ast.Ast,
    rr: *resolve.ResolveResult,
    deps: *sema_product.SemaDeps,
    load: *comptime_scope.ComptimeCtx,
    own_module: module.ModuleId,
    diags: *diagnostic.DiagnosticStore) res[sema_product.SemaResult, fail.Fail]
```

type a module in a scope over the load's, `load`, and resolve's bindings in `rr`, which sema
reads and never writes; what sema binds is its result's

### instance_sema

```mach
pub fun instance_sema(s: *session.Session, item: *sema_instance.Instance, deps: *sema_product.SemaDeps) res[sema_product.InstanceResult, fail.Fail]
```

types the instance `item` on its own: a generic under its type arguments, a function with
comptime value parameters with them bound, a function with a pack over its element types. it
reads its declaring module's typed definition and the surfaces `deps` holds, and writes only
its own result: its frames, the instances its body asks for after it in a set of its own,
the constants its body builds, and its reports, those it would make where it is asked for
placed nowhere. who asked for it is no part of its typing, so one typing serves every user

