# mach.lang.fe.sema.module

typing one module: its declarations and bodies, the instances they ask for, and the result it
publishes; and typing one instance on its own (instance_sema)

## fun sema

```mach
pub fun sema(
s: *session.Session,
a: *ast.Ast,
rr: *resolve.ResolveResult,
deps: *sema_product.SemaDeps,
load: *comptime_scope.ComptimeCtx,
own_module: module.ModuleId,
diags: *diagnostic.DiagnosticStore) res[sema_product.SemaResult, fail.Fail];
```

type a module in a scope over the load's, `load`, and resolve's bindings in `rr`, which sema
reads and never writes; what sema binds is its result's

## fun instance_sema

```mach
pub fun instance_sema(s: *session.Session, item: *sema_instance.Instance, deps: *sema_product.SemaDeps) res[sema_product.InstanceResult, fail.Fail];
```

types the instance `item` on its own: a generic under its type arguments, a function with
comptime value parameters with them bound, a function with a pack over its element types. it
reads its declaring module's typed definition and the surfaces `deps` holds, and writes only
its own result: its frames, the instances its body asks for after it in a set of its own,
the constants its body builds, and its reports, those it would make where it is asked for
placed nowhere. who asked for it is no part of its typing, so one typing serves every user

