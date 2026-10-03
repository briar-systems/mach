# mach.lang.fe.sema.module

typing one module: its declarations, bodies and instances, and the result it publishes

## fun sema

```mach
pub fun sema(
s: *session.Session,
a: *ast.Ast,
rr: *resolve.ResolveResult,
deps: *sema_product.SemaDeps,
load: *comptime.ComptimeCtx,
own_module: session.ModuleId,
diags: *diagnostic.DiagnosticStore) res[sema_product.SemaResult, fail.Fail];
```

type a module in a scope over the load's, `load`, and resolve's bindings in `rr`, which sema
reads and never writes; what sema binds is its result's

