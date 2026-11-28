[mach](../README.md) / [lower](README.md) / **expr**

# mach.lower.expr

the lowering of expressions into mirl values, places and branches

it covers reads and stores of places, calls with their class copies and `#[intrinsic]`
instructions, members and tag payloads, a guarded payload checked by a trap in the debug
profile, vector ranges, comptime intrinsics, the elements of a `$each`, and the constant a
global's initializer folds to.

Source: [`src/lower/expr.mach`](../../../src/lower/expr.mach)

Uses types from [`mach.base.fail`](../base/fail.md), [`mach.base.intern`](../base/intern.md), [`mach.base.source`](../base/source.md), [`mach.front.ast.expr`](../front/ast/expr.md), [`mach.front.ast.id`](../front/ast/id.md), [`mach.front.comptime.value`](../front/comptime/value.md), [`mach.front.resolve`](../front/resolve.md), [`mach.lower.context`](context.md), [`mach.lower.value`](value.md), [`mach.type.kind`](../type/kind.md).

| Type | | |
| :--- | :--- | :--- |
| [`Known`](#known) | `rec` | a tag whose case is known where it is read, as `known_case` finds it |

| On `Known` | |
| :--- | :--- |
| [`known_case`](#known_case) | the case of the tag place `eid`, read in place as `v` (`borrowed`), that a copy of it may move alone, none where the whole tag moves |
| [`copy_known`](#copy_known) | copy a tag whose case is known from `src` to `dst`, both addresses of it: the case's code and the case's payload, if it has one |

| Functions | |
| :--- | :--- |
| [`lower_rvalue`](#lower_rvalue) | lower the expression `eid` as a value, at its own source location, a value of a secret type classified secret |
| [`symbol_linkage_name`](#symbol_linkage_name) | the linkage name of the module-level binding `eid` names, nil when it names a binding with none |
| [`asm_constant`](#asm_constant) | the integer constant `sid` names for an asm block, as an integer of the pointer's width: a comptime parameter's or a module `val`'s, none for a binding of any other value |
| [`asm_read`](#asm_read) | the value of `sem_ty` an asm block reads from the slot `slot` it binds |
| [`calls_noreturn`](#calls_noreturn) | whether the expression `eid` is a call of a function declared `noreturn`, which ends the block it stands in |
| [`string_val`](#string_val) | the string the module `val` `d` declares holds where a reference of pointer type `vty` reads it as the address of the string's bytes: none for a `var`, an `ext` or thread-local binding, or a val of any other value |
| [`store_known`](#store_known) | store `v`, the tag place `eid` read in place when `borrowed`, to `dst`: the known case alone where `known_case` finds one, and the whole value otherwise, as `store_stated` stores it |
| [`try_lower_comptime_intrinsic`](#try_lower_comptime_intrinsic) | lower a call of a comptime intrinsic a runtime value stands for: a comptime path, `$type_name`, `$type_id`, `$zeroize`, or a layout measurement as `$size_of` |
| [`expr_span_of`](#expr_span_of) | where expression `eid` is written; an expression its syntax lacks records an internal failure and has the empty span |
| [`lower_into`](#lower_into) | the value a store into `dest` consumes, as `lower_stored_rvalue` gives it, with `dest` armed as the place an aggregate value of `eid` is built in when `armed` holds |
| [`lower_stored_rvalue`](#lower_stored_rvalue) | the value a store consumes |
| [`branch_on`](#branch_on) | branch to `then_b` where the condition `eid` holds, else to `else_b` |
| [`condition_value`](#condition_value) | the truth a branch tests of `v`, the value of the condition `eid`: a comparison, a logical operator or a runtime `sel` as it is, any other value compared equal to 1 |
| [`refuse_mixed`](#refuse_mixed) | refuse the pointer `v` written where `at` holds a pointer of the type `held` names, when one of the two is physical and the other logical: memory, a result and a parameter no call names a storage of hold physical pointers only, and a pointer local holds the class its initializer gave it, so a logical pointer stored there, or a physical one where a logical one is held, is a register one path makes logical and another physical (types.md, "pointers on spir-v") |
| [`store_stated`](#store_stated) | write `v` to `ptr`, a place of source type `ty`, stating the memory it writes as secret or public as `ty` is |
| [`case_literal`](#case_literal) | the payload of the tag literal `eid` of one payload, whose construction evaluates that payload and stores it before it writes anything else of the value (`lower_tag_case_lit`): a tag not zeroed whole first |
| [`const_value_of_init`](#const_value_of_init) | the comptime scalar `v` as a value of the type of `eid`, as a global's initializer names it |
| [`type_is_volatile_record`](#type_is_volatile_record) | whether `sem_ty`, its secret qualifier stripped, is a record, union or tag declared volatile |

---

## Known

```mach
pub rec Known {
    tag_ty:  mach.type.kind.TypeId;
    case_ix: u32;
}
```

a tag whose case is known where it is read, as `known_case` finds it

| Field | Type | |
| :--- | :--- | :--- |
| `tag_ty` | `mach.type.kind.TypeId` | the tag type |
| `case_ix` | `u32` | the case it holds there |

### known_case

```mach
pub fun known_case(ctx: *mach.lower.context.LowerContext, eid: mach.front.ast.id.ExprId, v: mach.lower.value.Value, borrowed: bool) opt[Known]
```

the case of the tag place `eid`, read in place as `v` (`borrowed`), that a copy of it may move alone, none where the
whole tag moves. sema states the case a live guard fixes of the place where it is read (`guard_case_of`), and a
guard ends at an assignment by name of anything the place is spelled with, so only a pointer or an asm block could
change the case under it, which `place_fixed` rules out: the place is reached through fields alone from a local or
parameter nothing takes the address of, stores whole or names in an asm block. a function that requires constant
time copies the whole tag, since its walk refuses a whole read of a slot holding bytes no store wrote
(mirl.pass.analysis.slots), and the destination of a copy is read whole wherever it is passed, returned or copied on

### copy_known

```mach
pub fun copy_known(ctx: *mach.lower.context.LowerContext, dst: mach.lower.value.Value, src: mach.lower.value.Value, k: Known) err[mach.base.fail.Fail]
```

copy a tag whose case is known from `src` to `dst`, both addresses of it: the case's code and
the case's payload, if it has one. the bytes of a case not active are no part of the value a
copy moves, so a copy of one known case moves only its own

## Functions

### lower_rvalue

```mach
pub fun lower_rvalue(ctx: *mach.lower.context.LowerContext, eid: mach.front.ast.id.ExprId) res[mach.lower.value.Value, mach.base.fail.Fail]
```

lower the expression `eid` as a value, at its own source location, a value of a secret type
classified secret

| Parameter | |
| :--- | :--- |
| `ctx` | the lowering context, positioned in the block the expression is evaluated in |
| `eid` | the expression |
| **returns** | the value, the address of its storage for an aggregate not held whole, or the failure located at the expression |

### symbol_linkage_name

```mach
pub fun symbol_linkage_name(ctx: *mach.lower.context.LowerContext, eid: mach.front.ast.id.ExprId) res[mach.base.intern.StrId, mach.base.fail.Fail]
```

the linkage name of the module-level binding `eid` names, nil when it names a binding with
none

### asm_constant

```mach
pub fun asm_constant(ctx: *mach.lower.context.LowerContext, sid: mach.front.resolve.SymbolId) res[opt[mach.lower.value.Value], mach.base.fail.Fail]
```

the integer constant `sid` names for an asm block, as an integer of the pointer's width: a comptime parameter's
or a module `val`'s, none for a binding of any other value

### asm_read

```mach
pub fun asm_read(ctx: *mach.lower.context.LowerContext, slot: mach.lower.value.Value, ity: Type, sem_ty: mach.type.kind.TypeId) res[mach.lower.value.Value, mach.base.fail.Fail]
```

the value of `sem_ty` an asm block reads from the slot `slot` it binds

### calls_noreturn

```mach
pub fun calls_noreturn(ctx: *mach.lower.context.LowerContext, eid: mach.front.ast.id.ExprId) res[bool, mach.base.fail.Fail]
```

whether the expression `eid` is a call of a function declared `noreturn`, which ends the block it stands in

### string_val

```mach
pub fun string_val(ctx: *mach.lower.context.LowerContext, d: mach.lower.context.Declared, vty: Type) opt[mach.base.intern.StrId]
```

the string the module `val` `d` declares holds where a reference of pointer type `vty` reads it as the address of the
string's bytes: none for a `var`, an `ext` or thread-local binding, or a val of any other value

### store_known

```mach
pub fun store_known(ctx: *mach.lower.context.LowerContext, eid: mach.front.ast.id.ExprId, v: mach.lower.value.Value, borrowed: bool, dst: mach.lower.value.Value, ty: mach.type.kind.TypeId, vol: bool) err[mach.base.fail.Fail]
```

store `v`, the tag place `eid` read in place when `borrowed`, to `dst`: the known case alone where `known_case` finds
one, and the whole value otherwise, as `store_stated` stores it

### try_lower_comptime_intrinsic

```mach
pub fun try_lower_comptime_intrinsic(ctx: *mach.lower.context.LowerContext, eid: mach.front.ast.id.ExprId, e: *mach.front.ast.expr.Expr) opt[res[mach.lower.value.Value, mach.base.fail.Fail]]
```

lower a call of a comptime intrinsic a runtime value stands for: a comptime path,
`$type_name`, `$type_id`, `$zeroize`, or a layout measurement as `$size_of`

| Parameter | |
| :--- | :--- |
| `ctx` | the lowering context |
| `eid` | the call |
| `e` | the call's syntax |
| **returns** | none when the call is no such intrinsic, else the value or the failure lowering it met |

### expr_span_of

```mach
pub fun expr_span_of(ctx: *mach.lower.context.LowerContext, eid: mach.front.ast.id.ExprId) mach.base.source.Span
```

where expression `eid` is written; an expression its syntax lacks records an internal failure
and has the empty span

### lower_into

```mach
pub fun lower_into(ctx: *mach.lower.context.LowerContext, eid: mach.front.ast.id.ExprId, armed: bool, dest: mach.lower.value.Value, in_place: bool, borrowed: *bool, built: *bool) res[mach.lower.value.Value, mach.base.fail.Fail]
```

the value a store into `dest` consumes, as `lower_stored_rvalue` gives it, with `dest` armed as the place an
aggregate value of `eid` is built in when `armed` holds. `built` says it was built there, leaving nothing to store.
whatever destination was armed before is armed again after

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

the truth a branch tests of `v`, the value of the condition `eid`: a comparison, a logical
operator or a runtime `sel` as it is, any other value compared equal to 1

| Parameter | |
| :--- | :--- |
| `ctx` | the lowering context |
| `eid` | the condition |
| `v` | its lowered value |
| **returns** | the truth value, or the builder's failure |

### refuse_mixed

```mach
pub fun refuse_mixed(ctx: *mach.lower.context.LowerContext, v: mach.lower.value.Value, at: mach.lower.value.Value, held: Type, span: mach.base.source.Span) err[mach.base.fail.Fail]
```

refuse the pointer `v` written where `at` holds a pointer of the type `held` names, when one of
the two is physical and the other logical: memory, a result and a parameter no call names a
storage of hold physical pointers only, and a pointer local holds the class its initializer
gave it, so a logical pointer stored there, or a physical one where a logical one is held, is
a register one path makes logical and another physical (types.md, "pointers on spir-v")

### store_stated

```mach
pub fun store_stated(ctx: *mach.lower.context.LowerContext, v: mach.lower.value.Value, ptr: mach.lower.value.Value, ty: mach.type.kind.TypeId, vol: bool) err[mach.base.fail.Fail]
```

write `v` to `ptr`, a place of source type `ty`, stating the memory it writes as secret or public as `ty` is

| Parameter | |
| :--- | :--- |
| `ctx` | the lowering context, positioned where the store goes |
| `v` | the value stored |
| `ptr` | the address of the place |
| `ty` | the place's source type, which decides the secrecy the store states |
| `vol` | whether the store is volatile |
| **returns** | ok, or a pointer of one class written where one of the other is held refused, or the builder's failure |

### case_literal

```mach
pub fun case_literal(ctx: *mach.lower.context.LowerContext, eid: mach.front.ast.id.ExprId) opt[mach.front.ast.id.ExprId]
```

the payload of the tag literal `eid` of one payload, whose construction evaluates that payload and stores it before
it writes anything else of the value (`lower_tag_case_lit`): a tag not zeroed whole first. none for any other

### const_value_of_init

```mach
pub fun const_value_of_init(ctx: *mach.lower.context.LowerContext, eid: mach.front.ast.id.ExprId, v: mach.front.comptime.value.CTValue) res[mach.lower.value.Value, mach.base.fail.Fail]
```

the comptime scalar `v` as a value of the type of `eid`, as a global's initializer names it

| Parameter | |
| :--- | :--- |
| `ctx` | the lowering context |
| `eid` | the expression whose type the value takes |
| `v` | an integer, float or string comptime value |
| **returns** | the constant, a string's being the address of a global holding its bytes, or a failure for any other kind of value |

### type_is_volatile_record

```mach
pub fun type_is_volatile_record(ctx: *mach.lower.context.LowerContext, sem_ty: mach.type.kind.TypeId) bool
```

whether `sem_ty`, its secret qualifier stripped, is a record, union or tag declared volatile

| Parameter | |
| :--- | :--- |
| `ctx` | the lowering context |
| `sem_ty` | the source type |
| **returns** | true for a volatile nominal aggregate, false for any other type |

