# mach.lang.fe.sema

## fwd context.SemaResult

```mach
fwd context.SemaResult
```

forwards [`mach.lang.fe.sema.context.SemaResult`](sema/context.md#rec-semaresult)

## fwd context.ModuleSema

```mach
fwd context.ModuleSema
```

forwards [`mach.lang.fe.sema.context.ModuleSema`](sema/context.md#rec-modulesema)

## fwd context.SemaDeps

```mach
fwd context.SemaDeps
```

forwards [`mach.lang.fe.sema.context.SemaDeps`](sema/context.md#rec-semadeps)

## fwd context.FieldEntry

```mach
fwd context.FieldEntry
```

forwards [`mach.lang.type.FieldEntry`](../type.md#rec-fieldentry)

## fwd context.FieldTable

```mach
fwd context.FieldTable
```

forwards [`mach.lang.type.FieldTable`](../type.md#rec-fieldtable)

## fwd context.SemaContext

```mach
fwd context.SemaContext
```

forwards [`mach.lang.fe.sema.context.SemaContext`](sema/context.md#rec-semacontext)

## fwd context.InstWorklist

```mach
fwd context.InstWorklist
```

forwards [`mach.lang.fe.sema.context.InstWorklist`](sema/context.md#rec-instworklist)

## fwd context.inst_worklist_new

```mach
fwd context.inst_worklist_new
```

forwards [`mach.lang.fe.sema.context.inst_worklist_new`](sema/context.md#fun-inst_worklist_new)

## fwd context.inst_worklist_free

```mach
fwd context.inst_worklist_free
```

forwards [`mach.lang.fe.sema.context.inst_worklist_free`](sema/context.md#fun-inst_worklist_free)

## fwd context.resolved_type_of

```mach
fwd context.resolved_type_of
```

forwards [`mach.lang.fe.sema.context.resolved_type_of`](sema/context.md#fun-resolved_type_of)

## fwd context.decl_type_for

```mach
fwd context.decl_type_for
```

forwards [`mach.lang.fe.sema.context.decl_type_for`](sema/context.md#fun-decl_type_for)

## fwd context.symbol_for_expr

```mach
fwd context.symbol_for_expr
```

forwards [`mach.lang.fe.sema.context.symbol_for_expr`](sema/context.md#fun-symbol_for_expr)

## fwd context.symbol_for_type

```mach
fwd context.symbol_for_type
```

forwards [`mach.lang.fe.sema.context.symbol_for_type`](sema/context.md#fun-symbol_for_type)

## fwd context.symbol_by_id

```mach
fwd context.symbol_by_id
```

forwards [`mach.lang.fe.sema.context.symbol_by_id`](sema/context.md#fun-symbol_by_id)

## fwd context.report

```mach
fwd context.report
```

forwards [`mach.lang.fe.sema.context.report`](sema/context.md#fun-report)

## fwd context.field_table_stage

```mach
fwd context.field_table_stage
```

forwards [`mach.lang.fe.sema.context.field_table_stage`](sema/context.md#fun-field_table_stage)

## fwd context.field_table_publish

```mach
fwd context.field_table_publish
```

forwards [`mach.lang.fe.sema.context.field_table_publish`](sema/context.md#fun-field_table_publish)

## fwd context.field_table_for

```mach
fwd context.field_table_for
```

forwards [`mach.lang.fe.sema.context.field_table_for`](sema/context.md#fun-field_table_for)

## fwd context.field_lookup

```mach
fwd context.field_lookup
```

forwards [`mach.lang.fe.sema.context.field_lookup`](sema/context.md#fun-field_lookup)

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
subst_owner: type.GenericOwner,
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
subst_owner: type.GenericOwner,
subst: *type.TypeId,
subst_len: u32,
diags: *diagnostic.DiagnosticStore) err[fail.Fail];
```

## fun dnit_result

```mach
pub fun dnit_result(r: *context.SemaResult);
```

