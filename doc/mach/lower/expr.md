[mach](../README.md) / [lower](README.md) / **expr**

# mach.lower.expr

the lowering of expressions into mirl values, places and branches

it covers reads and stores of places, calls with their class copies and `#[intrinsic]`
instructions, members and tag payloads, a guarded payload checked by a trap in the debug
profile, vector ranges, comptime intrinsics, the elements of a `$each`, and the constant a
global's initializer folds to.

<sub>source: [`src/lower/expr.mach`](../../../src/lower/expr.mach) · imported by 2 modules</sub>

<sub>`lower_context` is [`mach.lower.context`](context.md) · `ast_id` is [`mach.front.ast.id`](../front/ast/id.md) · `value` is [`mach.lower.value`](value.md) · `fail` is [`mach.base.fail`](../base/fail.md) · `intern` is [`mach.base.intern`](../base/intern.md) · `ast_expr` is [`mach.front.ast.expr`](../front/ast/expr.md) · `base_source` is [`mach.base.source`](../base/source.md) · `id` is `mirl.ir.id` · `type_kind` is [`mach.type.kind`](../type/kind.md) · `comptime_value` is [`mach.front.comptime.value`](../front/comptime/value.md)</sub>

| Functions | |
| :--- | :--- |
| [`lower_rvalue`](#lower_rvalue) | |
| [`symbol_linkage_name`](#symbol_linkage_name) | the linkage name of the module-level binding `eid` names, nil when it names a binding with none |
| [`try_lower_comptime_intrinsic`](#try_lower_comptime_intrinsic) | |
| [`expr_span_of`](#expr_span_of) | where expression `eid` is written; an expression its syntax lacks records an internal failure and has the empty span |
| [`lower_stored_rvalue`](#lower_stored_rvalue) | the value a store consumes |
| [`branch_on`](#branch_on) | branch to `then_b` where the condition `eid` holds, else to `else_b` |
| [`condition_value`](#condition_value) | |
| [`store_stated`](#store_stated) | write `v` to `ptr`, a place of source type `ty`, stating the memory it writes as secret or public as `ty` is |
| [`const_value_of_init`](#const_value_of_init) · [`type_is_volatile_record`](#type_is_volatile_record) | |

---

## Functions

### lower_rvalue

```mach
pub fun lower_rvalue(ctx: *lower_context.LowerContext, eid: ast_id.ExprId) res[value.Value, fail.Fail]
```

### symbol_linkage_name

```mach
pub fun symbol_linkage_name(ctx: *lower_context.LowerContext, eid: ast_id.ExprId) res[intern.StrId, fail.Fail]
```

the linkage name of the module-level binding `eid` names, nil when it names a binding with
none

### try_lower_comptime_intrinsic

```mach
pub fun try_lower_comptime_intrinsic(ctx: *lower_context.LowerContext, eid: ast_id.ExprId, e: *ast_expr.Expr) opt[res[value.Value, fail.Fail]]
```

### expr_span_of

```mach
pub fun expr_span_of(ctx: *lower_context.LowerContext, eid: ast_id.ExprId) base_source.Span
```

where expression `eid` is written; an expression its syntax lacks records an internal failure
and has the empty span

### lower_stored_rvalue

```mach
pub fun lower_stored_rvalue(ctx: *lower_context.LowerContext, eid: ast_id.ExprId, in_place: bool, borrowed: *bool) res[value.Value, fail.Fail]
```

the value a store consumes. when `in_place` holds, nothing writes memory between this read
and the store, so an aggregate place is read where it lies and the store copies from it:
the aggregate copy is overlap-safe, so no snapshot sits between them

### branch_on

```mach
pub fun branch_on(ctx: *lower_context.LowerContext, eid: ast_id.ExprId, then_b: id.Block, else_b: id.Block) err[fail.Fail]
```

branch to `then_b` where the condition `eid` holds, else to `else_b`. a logical and, or and
not that holds no secret branches on its operands in turn, so no bool is made for it and
its right operand runs only where the left does not decide

### condition_value

```mach
pub fun condition_value(ctx: *lower_context.LowerContext, eid: ast_id.ExprId, v: value.Value) res[value.Value, fail.Fail]
```

### store_stated

```mach
pub fun store_stated(ctx: *lower_context.LowerContext, v: value.Value, ptr: value.Value, ty: type_kind.TypeId, vol: bool) err[fail.Fail]
```

write `v` to `ptr`, a place of source type `ty`, stating the memory it writes as secret or public as `ty` is

### const_value_of_init

```mach
pub fun const_value_of_init(ctx: *lower_context.LowerContext, eid: ast_id.ExprId, v: comptime_value.CTValue) res[value.Value, fail.Fail]
```

### type_is_volatile_record

```mach
pub fun type_is_volatile_record(ctx: *lower_context.LowerContext, sem_ty: type_kind.TypeId) bool
```

