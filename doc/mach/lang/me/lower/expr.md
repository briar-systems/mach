# mach.lang.me.lower.expr

## fun lower_rvalue

```mach
pub fun lower_rvalue(ctx: *context.LowerContext, eid: id.ExprId) res[value.Value, fail.Fail];
```

## fun lower_lvalue

```mach
pub fun lower_lvalue(ctx: *context.LowerContext, eid: id.ExprId) res[value.Value, fail.Fail];
```

## fun lower_lit_str

```mach
pub fun lower_lit_str(ctx: *context.LowerContext, e: *expr.Expr) res[value.Value, fail.Fail];
```

## fun symbol_linkage_name

```mach
pub fun symbol_linkage_name(ctx: *context.LowerContext, eid: id.ExprId) opt[intern.StrId];
```

## fun references_function

```mach
pub fun references_function(ctx: *context.LowerContext, eid: id.ExprId) bool;
```

## fun constant_literal_expr

```mach
pub fun constant_literal_expr(ctx: *context.LowerContext, eid: id.ExprId) opt[id.ExprId];
```

the literal a constant-valued expression stands for, when it stands for one: a `$each` element or
a module `val` initialized with an aggregate literal, a member path into such a literal, or an
identity cast of one. a static initializer spelled over such a name folds the literal it names

## fun try_lower_comptime_intrinsic

```mach
pub fun try_lower_comptime_intrinsic(ctx: *context.LowerContext, eid: id.ExprId, e: *expr.Expr) opt[res[value.Value, fail.Fail]];
```

## fun try_lower_comptime_cast

```mach
pub fun try_lower_comptime_cast(ctx: *context.LowerContext, eid: id.ExprId, e: *expr.Expr) opt[res[value.Value, fail.Fail]];
```

## fun condition_value

```mach
pub fun condition_value(ctx: *context.LowerContext, eid: id.ExprId, v: value.Value) res[value.Value, fail.Fail];
```

## fun case_descriptor_index

```mach
pub fun case_descriptor_index(ctx: *context.LowerContext, desc_eid: id.ExprId) res[u32, fail.Fail];
```

the case ordinal a comptime case descriptor names; type checking already tied it to this tag

## fun literal_head_case

```mach
pub fun literal_head_case(ctx: *context.LowerContext, tid: id.TypeId) token.Span;
```

## fun literal_head_case_desc

```mach
pub fun literal_head_case_desc(ctx: *context.LowerContext, tid: id.TypeId) id.ExprId;
```

## fun literal_head_case_index

```mach
pub fun literal_head_case_index(ctx: *context.LowerContext, tid: id.TypeId, tag_ty: type.TypeId) res[opt[u32], fail.Fail];
```

the case a literal head selects, by name or through a descriptor; none when the head names a plain type

## fun const_value_of

```mach
pub fun const_value_of(ctx: *context.LowerContext, eid: id.ExprId, v: comptime.CTValue) res[value.Value, fail.Fail];
```

## fun const_value_of_init

```mach
pub fun const_value_of_init(ctx: *context.LowerContext, eid: id.ExprId, v: comptime.CTValue) res[value.Value, fail.Fail];
```

## fun type_is_volatile_record

```mach
pub fun type_is_volatile_record(ctx: *context.LowerContext, sem_ty: type.TypeId) bool;
```

## fun access_is_volatile

```mach
pub fun access_is_volatile(ctx: *context.LowerContext, eid: id.ExprId) bool;
```

volatility is a property of the storage an access is rooted in. the chain of
member, projection and index steps is walked to the object it reads or writes,
and any volatile nominal along it, the node's own type included, marks the
access; an indirection ends the chain at its pointee, so a pointer field of a
volatile record reaches ordinary storage and a raw scalar pointer is never
volatile. one predicate for every access form, load and store alike

## fun field_index_in_type

```mach
pub fun field_index_in_type(ctx: *context.LowerContext, rec_ty: type.TypeId, name: token.Span) u32;
```

