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

## fwd sema_context.GrowthWatch

```mach
fwd sema_context.GrowthWatch
```

forwards [`mach.lang.fe.sema.context.GrowthWatch`](sema/context.md#rec-growthwatch)

## fwd sema_context.FieldEntry

```mach
fwd sema_context.FieldEntry
```

forwards [`mach.lang.type.field.Entry`](../type/field.md#rec-entry)

## fwd sema_context.FieldTable

```mach
fwd sema_context.FieldTable
```

forwards [`mach.lang.type.field.Table`](../type/field.md#rec-table)

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
load: *comptime.ComptimeCtx,
own_module: session.ModuleId,
diags: *diagnostic.DiagnosticStore) res[sema_context.SemaResult, fail.Fail];
```

type a module in a scope over the load's, `load`, and resolve's bindings in `rr`, which sema
reads and never writes; what sema binds is its result's

## fun dnit_result

```mach
pub fun dnit_result(r: *sema_context.SemaResult);
```

