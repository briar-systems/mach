# mach.lang.me.lower.expr

## fun lower_rvalue

```mach
pub fun lower_rvalue(ctx: *lower_context.LowerContext, eid: ast_id.ExprId) res[value.Value, fail.Fail];
```

## fun lower_lvalue

```mach
pub fun lower_lvalue(ctx: *lower_context.LowerContext, eid: ast_id.ExprId) res[value.Value, fail.Fail];
```

## fun lower_lit_str

```mach
pub fun lower_lit_str(ctx: *lower_context.LowerContext, e: *ast_expr.Expr) res[value.Value, fail.Fail];
```

## fun symbol_linkage_name

```mach
pub fun symbol_linkage_name(ctx: *lower_context.LowerContext, eid: ast_id.ExprId) opt[intern.StrId];
```

## fun references_function

```mach
pub fun references_function(ctx: *lower_context.LowerContext, eid: ast_id.ExprId) bool;
```

## fun constant_literal_expr

```mach
pub fun constant_literal_expr(ctx: *lower_context.LowerContext, eid: ast_id.ExprId) opt[ast_id.ExprId];
```

the literal a constant-valued expression stands for, when it stands for one: a `$each` element or
a module `val` initialized with an aggregate literal, a member path into such a literal, or an
identity cast of one. a range over such a literal, or a `::` between an array and a vector of
it, stands for itself, the window its elements are read from. a static initializer spelled
over such a name folds the literal it names

## fun try_lower_comptime_intrinsic

```mach
pub fun try_lower_comptime_intrinsic(ctx: *lower_context.LowerContext, eid: ast_id.ExprId, e: *ast_expr.Expr) opt[res[value.Value, fail.Fail]];
```

## fun try_lower_comptime_cast

```mach
pub fun try_lower_comptime_cast(ctx: *lower_context.LowerContext, eid: ast_id.ExprId, e: *ast_expr.Expr) opt[res[value.Value, fail.Fail]];
```

## fun lower_stored_rvalue

```mach
pub fun lower_stored_rvalue(ctx: *lower_context.LowerContext, eid: ast_id.ExprId, in_place: bool, borrowed: *bool) res[value.Value, fail.Fail];
```

the value a store consumes. when `in_place` holds, nothing writes memory between this read
and the store, so an aggregate place is read where it lies and the store copies from it:
the aggregate copy is overlap-safe, so no snapshot sits between them (#4235, #4241)

## fun condition_value

```mach
pub fun condition_value(ctx: *lower_context.LowerContext, eid: ast_id.ExprId, v: value.Value) res[value.Value, fail.Fail];
```

## fun case_descriptor_index

```mach
pub fun case_descriptor_index(ctx: *lower_context.LowerContext, desc_eid: ast_id.ExprId) res[u32, fail.Fail];
```

the case ordinal a comptime case descriptor names; type checking already tied it to this tag

## fun literal_head_case

```mach
pub fun literal_head_case(ctx: *lower_context.LowerContext, tid: ast_id.TypeId) token.Span;
```

## fun literal_head_case_desc

```mach
pub fun literal_head_case_desc(ctx: *lower_context.LowerContext, tid: ast_id.TypeId) ast_id.ExprId;
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

## fun access_is_volatile

```mach
pub fun access_is_volatile(ctx: *lower_context.LowerContext, eid: ast_id.ExprId) bool;
```

volatility is a property of the storage an access is rooted in. the chain of
member, projection and index steps is walked to the object it reads or writes,
and any volatile nominal along it, the node's own type included, marks the
access; an indirection ends the chain at its pointee, so a pointer field of a
volatile record reaches ordinary storage and a raw scalar pointer is never
volatile. one predicate for every access form, load and store alike

## fun field_index_in_type

```mach
pub fun field_index_in_type(ctx: *lower_context.LowerContext, rec_ty: type.TypeId, name: token.Span) u32;
```

