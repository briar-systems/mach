# mach.lang.fe.sema

## fwd sema_context.SemaResult

```mach
fwd sema_context.SemaResult
```

forwards [`mach.lang.fe.sema.context.SemaResult`](sema/context.md#rec-semaresult)

## fwd sema_context.ModuleSema

```mach
fwd sema_context.ModuleSema
```

forwards [`mach.lang.fe.sema.context.ModuleSema`](sema/context.md#rec-modulesema)

## fwd sema_context.SemaDeps

```mach
fwd sema_context.SemaDeps
```

forwards [`mach.lang.fe.sema.context.SemaDeps`](sema/context.md#rec-semadeps)

## fwd sema_context.FieldEntry

```mach
fwd sema_context.FieldEntry
```

forwards [`mach.lang.type.FieldEntry`](../type.md#rec-fieldentry)

## fwd sema_context.FieldTable

```mach
fwd sema_context.FieldTable
```

forwards [`mach.lang.type.FieldTable`](../type.md#rec-fieldtable)

## fwd sema_context.SemaContext

```mach
fwd sema_context.SemaContext
```

forwards [`mach.lang.fe.sema.context.SemaContext`](sema/context.md#rec-semacontext)

## fwd sema_context.InstWorklist

```mach
fwd sema_context.InstWorklist
```

forwards [`mach.lang.fe.sema.context.InstWorklist`](sema/context.md#rec-instworklist)

## fwd sema_context.inst_worklist_new

```mach
fwd sema_context.inst_worklist_new
```

forwards [`mach.lang.fe.sema.context.inst_worklist_new`](sema/context.md#fun-inst_worklist_new)

## fwd sema_context.inst_worklist_free

```mach
fwd sema_context.inst_worklist_free
```

forwards [`mach.lang.fe.sema.context.inst_worklist_free`](sema/context.md#fun-inst_worklist_free)

## fwd sema_context.resolved_type_of

```mach
fwd sema_context.resolved_type_of
```

forwards [`mach.lang.fe.sema.context.resolved_type_of`](sema/context.md#fun-resolved_type_of)

## fwd sema_context.decl_type_for

```mach
fwd sema_context.decl_type_for
```

forwards [`mach.lang.fe.sema.context.decl_type_for`](sema/context.md#fun-decl_type_for)

## fwd sema_context.symbol_for_expr

```mach
fwd sema_context.symbol_for_expr
```

forwards [`mach.lang.fe.sema.context.symbol_for_expr`](sema/context.md#fun-symbol_for_expr)

## fwd sema_context.symbol_for_type

```mach
fwd sema_context.symbol_for_type
```

forwards [`mach.lang.fe.sema.context.symbol_for_type`](sema/context.md#fun-symbol_for_type)

## fwd sema_context.symbol_by_id

```mach
fwd sema_context.symbol_by_id
```

forwards [`mach.lang.fe.sema.context.symbol_by_id`](sema/context.md#fun-symbol_by_id)

## fwd sema_context.report

```mach
fwd sema_context.report
```

forwards [`mach.lang.fe.sema.context.report`](sema/context.md#fun-report)

## fwd sema_context.field_table_stage

```mach
fwd sema_context.field_table_stage
```

forwards [`mach.lang.fe.sema.context.field_table_stage`](sema/context.md#fun-field_table_stage)

## fwd sema_context.field_table_publish

```mach
fwd sema_context.field_table_publish
```

forwards [`mach.lang.fe.sema.context.field_table_publish`](sema/context.md#fun-field_table_publish)

## fwd sema_context.field_table_for

```mach
fwd sema_context.field_table_for
```

forwards [`mach.lang.fe.sema.context.field_table_for`](sema/context.md#fun-field_table_for)

## fwd sema_context.field_lookup

```mach
fwd sema_context.field_lookup
```

forwards [`mach.lang.fe.sema.context.field_lookup`](sema/context.md#fun-field_lookup)

## fun sema

```mach
pub fun sema(
s: *session.Session,
a: *ast.Ast,
rr: *resolve.ResolveResult,
deps: *sema_context.SemaDeps,
ctx: *comptime.ComptimeCtx,
own_module: session.ModuleId,
diags: *diagnostic.DiagnosticStore) res[sema_context.SemaResult, fail.Fail];
```

## fun reinfer_pack_each_body

```mach
pub fun reinfer_pack_each_body(
s: *session.Session,
a: *ast.Ast,
rr: *resolve.ResolveResult,
deps: *sema_context.SemaDeps,
ctx: *comptime.ComptimeCtx,
own_module: session.ModuleId,
sema_result: *sema_context.SemaResult,
fn_ret: type.TypeId,
body_start: u32,
body_len: u32,
subst_owner: type.GenericOwner,
subst: *type.TypeId,
subst_len: u32,
insts: *sema_context.InstWorklist,
diags: *diagnostic.DiagnosticStore) err[fail.Fail];
```

## fun reinfer_instance_body

```mach
pub fun reinfer_instance_body(
s: *session.Session,
a: *ast.Ast,
rr: *resolve.ResolveResult,
deps: *sema_context.SemaDeps,
ctx: *comptime.ComptimeCtx,
own_module: session.ModuleId,
sema_result: *sema_context.SemaResult,
fn_ret: type.TypeId,
body: ast_id.StmtId,
subst_owner: type.GenericOwner,
subst: *type.TypeId,
subst_len: u32,
diags: *diagnostic.DiagnosticStore) err[fail.Fail];
```

## fun dnit_result

```mach
pub fun dnit_result(r: *sema_context.SemaResult);
```

