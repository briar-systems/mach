[mach](../README.md) / [lower](README.md) / **expr**

# mach.lower.expr

the lowering of expressions into mirl values, places and branches

it covers reads and stores of places, calls with their class copies and `#[intrinsic]`
instructions, members and tag payloads, a guarded payload checked by a trap in the debug
profile, vector ranges, comptime intrinsics, the elements of a `$each`, and the constant a
global's initializer folds to.

Source: [`src/lower/expr.mach`](../../../src/lower/expr.mach)

Uses types from [`mach.base.fail`](../base/fail.md), [`mach.base.intern`](../base/intern.md), [`mach.base.source`](../base/source.md), [`mach.front.ast.expr`](../front/ast/expr.md), [`mach.front.ast.id`](../front/ast/id.md), [`mach.front.comptime.value`](../front/comptime/value.md), [`mach.lower.context`](context.md), [`mach.lower.value`](value.md), [`mach.type.kind`](../type/kind.md).

| Functions | |
| :--- | :--- |
| [`lower_rvalue`](#lower_rvalue) | |
| [`symbol_linkage_name`](#symbol_linkage_name) | the linkage name of the module-level binding `eid` names, nil when it names a binding with none |
| [`try_lower_comptime_intrinsic`](#try_lower_comptime_intrinsic) | |
| [`expr_span_of`](#expr_span_of) | where expression `eid` is written; an expression its syntax lacks records an internal failure and has the empty span |
| [`lower_stored_rvalue`](#lower_stored_rvalue) | the value a store consumes |
| [`branch_on`](#branch_on) | branch to `then_b` where the condition `eid` holds, else to `else_b` |
| [`condition_value`](#condition_value) | |
| [`refuse_mixed`](#refuse_mixed) | write `v` to `ptr`, a place of source type `ty`, stating the memory it writes as secret or public as `ty` is refuse the pointer `v` written where `at` holds a pointer of the type `held` names, when one of the two is physical and the other logical: memory, a result and a parameter no call names a storage of hold physical pointers only, and a pointer local holds the class its initializer gave it, so a logical pointer stored there, or a physical one where a logical one is held, is a register one path makes logical and another physical (types.md, "pointers on spir-v") |
| [`store_stated`](#store_stated), [`const_value_of_init`](#const_value_of_init), [`type_is_volatile_record`](#type_is_volatile_record) | |

---

## Functions

### lower_rvalue

```mach
pub fun lower_rvalue(ctx: *mach.lower.context.LowerContext, eid: mach.front.ast.id.ExprId) res[mach.lower.value.Value, mach.base.fail.Fail]
```

### symbol_linkage_name

```mach
pub fun symbol_linkage_name(ctx: *mach.lower.context.LowerContext, eid: mach.front.ast.id.ExprId) res[mach.base.intern.StrId, mach.base.fail.Fail]
```

the linkage name of the module-level binding `eid` names, nil when it names a binding with
none

### try_lower_comptime_intrinsic

```mach
pub fun try_lower_comptime_intrinsic(ctx: *mach.lower.context.LowerContext, eid: mach.front.ast.id.ExprId, e: *mach.front.ast.expr.Expr) opt[res[mach.lower.value.Value, mach.base.fail.Fail]]
```

### expr_span_of

```mach
pub fun expr_span_of(ctx: *mach.lower.context.LowerContext, eid: mach.front.ast.id.ExprId) mach.base.source.Span
```

where expression `eid` is written; an expression its syntax lacks records an internal failure
and has the empty span

### lower_stored_rvalue

```mach
pub fun lower_stored_rvalue(ctx: *mach.lower.context.LowerContext, eid: mach.front.ast.id.ExprId, in_place: bool, borrowed: *bool) res[mach.lower.value.Value, mach.base.fail.Fail]
```

the value a store consumes. when `in_place` holds, nothing writes memory between this read
and the store, so an aggregate place is read where it lies and the store copies from it:
the aggregate copy is overlap-safe, so no snapshot sits between them

### branch_on

```mach
pub fun branch_on(ctx: *mach.lower.context.LowerContext, eid: mach.front.ast.id.ExprId, then_b: mirl.ir.id.Block, else_b: mirl.ir.id.Block) err[mach.base.fail.Fail]
```

branch to `then_b` where the condition `eid` holds, else to `else_b`. a logical and, or and
not that holds no secret branches on its operands in turn, so no bool is made for it and
its right operand runs only where the left does not decide

### condition_value

```mach
pub fun condition_value(ctx: *mach.lower.context.LowerContext, eid: mach.front.ast.id.ExprId, v: mach.lower.value.Value) res[mach.lower.value.Value, mach.base.fail.Fail]
```

### refuse_mixed

```mach
pub fun refuse_mixed(ctx: *mach.lower.context.LowerContext, v: mach.lower.value.Value, at: mach.lower.value.Value, held: Type, span: mach.base.source.Span) err[mach.base.fail.Fail]
```

write `v` to `ptr`, a place of source type `ty`, stating the memory it writes as secret or public as `ty` is
refuse the pointer `v` written where `at` holds a pointer of the type `held` names, when one of
the two is physical and the other logical: memory, a result and a parameter no call names a
storage of hold physical pointers only, and a pointer local holds the class its initializer
gave it, so a logical pointer stored there, or a physical one where a logical one is held, is
a register one path makes logical and another physical (types.md, "pointers on spir-v")

### store_stated

```mach
pub fun store_stated(ctx: *mach.lower.context.LowerContext, v: mach.lower.value.Value, ptr: mach.lower.value.Value, ty: mach.type.kind.TypeId, vol: bool) err[mach.base.fail.Fail]
```

### const_value_of_init

```mach
pub fun const_value_of_init(ctx: *mach.lower.context.LowerContext, eid: mach.front.ast.id.ExprId, v: mach.front.comptime.value.CTValue) res[mach.lower.value.Value, mach.base.fail.Fail]
```

### type_is_volatile_record

```mach
pub fun type_is_volatile_record(ctx: *mach.lower.context.LowerContext, sem_ty: mach.type.kind.TypeId) bool
```

