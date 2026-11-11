# mach.lang.me.lower.expr

## fun lower_rvalue

```mach
pub fun lower_rvalue(ctx: *lower_context.LowerContext, eid: ast_id.ExprId) res[value.Value, fail.Fail];
```

## fun symbol_linkage_name

```mach
pub fun symbol_linkage_name(ctx: *lower_context.LowerContext, eid: ast_id.ExprId) res[intern.StrId, fail.Fail];
```

the linkage name of the module-level binding `eid` names, nil when it names a binding with
none

## fun try_lower_comptime_intrinsic

```mach
pub fun try_lower_comptime_intrinsic(ctx: *lower_context.LowerContext, eid: ast_id.ExprId, e: *ast_expr.Expr) opt[res[value.Value, fail.Fail]];
```

## fun expr_span_of

```mach
pub fun expr_span_of(ctx: *lower_context.LowerContext, eid: ast_id.ExprId) lang_source.Span;
```

where expression `eid` is written; an expression its syntax lacks records an internal failure
and has the empty span

## fun lower_stored_rvalue

```mach
pub fun lower_stored_rvalue(ctx: *lower_context.LowerContext, eid: ast_id.ExprId, in_place: bool, borrowed: *bool) res[value.Value, fail.Fail];
```

the value a store consumes. when `in_place` holds, nothing writes memory between this read
and the store, so an aggregate place is read where it lies and the store copies from it:
the aggregate copy is overlap-safe, so no snapshot sits between them

## fun branch_on

```mach
pub fun branch_on(ctx: *lower_context.LowerContext, eid: ast_id.ExprId, then_b: id.Block, else_b: id.Block) err[fail.Fail];
```

branch to `then_b` where the condition `eid` holds, else to `else_b`. a logical and, or and
not that holds no secret branches on its operands in turn, so no bool is made for it and
its right operand runs only where the left does not decide

## fun condition_value

```mach
pub fun condition_value(ctx: *lower_context.LowerContext, eid: ast_id.ExprId, v: value.Value) res[value.Value, fail.Fail];
```

## fun const_value_of_init

```mach
pub fun const_value_of_init(ctx: *lower_context.LowerContext, eid: ast_id.ExprId, v: comptime_value.CTValue) res[value.Value, fail.Fail];
```

## fun type_is_volatile_record

```mach
pub fun type_is_volatile_record(ctx: *lower_context.LowerContext, sem_ty: type_kind.TypeId) bool;
```

