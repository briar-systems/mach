# mach.lang.fe.sema

## fun sema

```mach
pub fun sema(
s: *session.Session,
a: *ast.Ast,
rr: *resolve.ResolveResult,
deps: *context.SemaDeps,
ctx: *comptime.ComptimeCtx,
own_module: session.ModuleId,
diags: *diagnostic.DiagnosticStore) res[context.SemaResult, fail.Fail];
```

## fun resolve_gates

```mach
pub fun resolve_gates(
s: *session.Session,
a: *ast.Ast,
rr: *resolve.ResolveResult,
deps: *context.SemaDeps,
ctx: *comptime.ComptimeCtx,
own_module: session.ModuleId,
diags: *diagnostic.DiagnosticStore) res[context.SemaResult, fail.Fail];
```

## fun reinfer_pack_each_body

```mach
pub fun reinfer_pack_each_body(
s: *session.Session,
a: *ast.Ast,
rr: *resolve.ResolveResult,
deps: *context.SemaDeps,
ctx: *comptime.ComptimeCtx,
own_module: session.ModuleId,
sema_result: *context.SemaResult,
fn_ret: type.TypeId,
body_start: u32,
body_len: u32,
subst: *type.TypeId,
subst_len: u32,
insts: *context.InstWorklist,
diags: *diagnostic.DiagnosticStore) err[fail.Fail];
```

## fun reinfer_instance_body

```mach
pub fun reinfer_instance_body(
s: *session.Session,
a: *ast.Ast,
rr: *resolve.ResolveResult,
deps: *context.SemaDeps,
ctx: *comptime.ComptimeCtx,
own_module: session.ModuleId,
sema_result: *context.SemaResult,
fn_ret: type.TypeId,
body: id.StmtId,
subst: *type.TypeId,
subst_len: u32,
diags: *diagnostic.DiagnosticStore) err[fail.Fail];
```

## fun dnit_result

```mach
pub fun dnit_result(r: *context.SemaResult);
```

