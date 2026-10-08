# mach.lang.me.lower.expr

## fun lower_rvalue

```mach
pub fun lower_rvalue(ctx: *lower_context.LowerContext, eid: ast_id.ExprId) res[value.Value, fail.Fail];
```

## fun lower_lit_str

```mach
pub fun lower_lit_str(ctx: *lower_context.LowerContext, e: *ast_expr.Expr) res[value.Value, fail.Fail];
```

## fun symbol_linkage_name

```mach
pub fun symbol_linkage_name(ctx: *lower_context.LowerContext, eid: ast_id.ExprId) intern.StrId;
```

## fun try_lower_comptime_intrinsic

```mach
pub fun try_lower_comptime_intrinsic(ctx: *lower_context.LowerContext, eid: ast_id.ExprId, e: *ast_expr.Expr) opt[res[value.Value, fail.Fail]];
```

## fun lower_stored_rvalue

```mach
pub fun lower_stored_rvalue(ctx: *lower_context.LowerContext, eid: ast_id.ExprId, in_place: bool, borrowed: *bool) res[value.Value, fail.Fail];
```

the value a store consumes. when `in_place` holds, nothing writes memory between this read
and the store, so an aggregate place is read where it lies and the store copies from it:
the aggregate copy is overlap-safe, so no snapshot sits between them

## fun condition_value

```mach
pub fun condition_value(ctx: *lower_context.LowerContext, eid: ast_id.ExprId, v: value.Value) res[value.Value, fail.Fail];
```

## fun literal_head_case_index

```mach
pub fun literal_head_case_index(ctx: *lower_context.LowerContext, tid: ast_id.TypeId, tag_ty: type.TypeId) res[opt[u32], fail.Fail];
```

the case a literal head selects, by name or through a descriptor; none when the head names a plain type

## fun const_value_of

```mach
pub fun const_value_of(ctx: *lower_context.LowerContext, eid: ast_id.ExprId, v: comptime.CTValue) res[value.Value, fail.Fail];
```

## fun const_value_of_init

```mach
pub fun const_value_of_init(ctx: *lower_context.LowerContext, eid: ast_id.ExprId, v: comptime.CTValue) res[value.Value, fail.Fail];
```

## fun type_is_volatile_record

```mach
pub fun type_is_volatile_record(ctx: *lower_context.LowerContext, sem_ty: type.TypeId) bool;
```

## fun field_index_in_type

```mach
pub fun field_index_in_type(ctx: *lower_context.LowerContext, rec_ty: type.TypeId, name: lang_source.Span) u32;
```

