[mach](../../README.md) / [front](../README.md) / [comptime](README.md) / **intrinsic**

# mach.front.comptime.intrinsic

the comptime intrinsic registry: one row per `$name(...)` the compiler answers,
which every phase reads by id

each phase decodes a comptime call's `$name` to the id it spells and decides by
the id and its row, never by the name. a new intrinsic is a row here and the
consumers that answer it

Source: [`src/front/comptime/intrinsic.mach`](../../../../src/front/comptime/intrinsic.mach)

Uses types from [`mach.base.source`](../../base/source.md), [`mach.base.textbuild`](../../base/textbuild.md), [`mach.front.ast`](../ast.md), [`mach.front.ast.expr`](../ast/expr.md), [`mach.front.ast.id`](../ast/id.md).

| Type | | |
| :--- | :--- | :--- |
| [`Id`](#id) | `def` |  |
| [`Class`](#class) | `def` | what an intrinsic answers |
| [`Row`](#row-1) | `rec` | one intrinsic |

| On `Id` | |
| :--- | :--- |
| [`NONE`](#id), [`SIZE_OF`](#id), [`ALIGN_OF`](#id), [`OFFSET_OF`](#id), [`LENGTH_OF`](#id), [`TYPE_ID`](#id), [`TYPE_NAME`](#id), [`IS_RECORD`](#id), [`IS_UNION`](#id), [`IS_TAG`](#id), [`IS_POINTER`](#id), [`IS_SECRET`](#id), [`HOLDS_SECRET`](#id), [`IS_INTEGER`](#id), [`IS_FLOAT`](#id), [`TYPE_OF`](#id), [`POINTEE_OF`](#id), [`DISCRIMINANT_OF`](#id), [`ZEROIZE`](#id) | |
| [`row`](#row) | the row of `id`; nil for NONE and any id no row declares |
| [`id_of`](#id_of) | the intrinsic the name at `name` in `source` spells, without its `$`; NONE when it spells none |
| [`in_class`](#in_class) | whether `id` answers within `class` |
| [`call_intrinsic`](#call_intrinsic) | the intrinsic a comptime call `$name(...)` names; NONE for any other expression |
| [`ident_intrinsic`](#ident_intrinsic) | the intrinsic a `$name` token spells |

| On `Class` | |
| :--- | :--- |
| [`CLASS_LAYOUT`](#class), [`CLASS_QUERY`](#class) | |

| Functions | |
| :--- | :--- |
| [`constructor_refusal`](#constructor_refusal) | the refusal of a `$` in a type that names no type constructor, listing the ones there are |
| [`first_call_arg`](#first_call_arg), [`intrinsic_takes_type_operand`](#intrinsic_takes_type_operand), [`is_type_of_call`](#is_type_of_call), [`type_of_arg`](#type_of_arg), [`is_fields_call`](#is_fields_call), [`is_cases_call`](#is_cases_call), [`is_layout_intrinsic_call`](#is_layout_intrinsic_call), [`is_type_id_call`](#is_type_id_call), [`is_length_of_call`](#is_length_of_call), [`is_offset_of_call`](#is_offset_of_call), [`is_size_of_call`](#is_size_of_call), [`is_align_of_call`](#is_align_of_call), [`layout_intrinsic_type_arg`](#layout_intrinsic_type_arg), [`fields_type_arg`](#fields_type_arg), [`is_error_call`](#is_error_call), [`type_operand_value`](#type_operand_value), [`is_type_comparison`](#is_type_comparison), [`is_type_comparison_binary`](#is_type_comparison_binary), [`is_field_type_member`](#is_field_type_member), [`is_field_descriptor_member`](#is_field_descriptor_member), [`is_type_query_call`](#is_type_query_call) | |
| [`is_type_question_call`](#is_type_question_call) | a call that asks about a type: its layout, its identity, or a predicate over it |
| [`is_path_call`](#is_path_call) | a call whose callee is a rooted comptime path: `$mach.build.ct_mul(low, 64)` |
| [`is_comptime_value`](#is_comptime_value) | a rooted comptime path, or a call on one: both fold to a constant |
| [`is_comptime_path`](#is_comptime_path), [`comptime_ident_name`](#comptime_ident_name) | |

---

## Id

```mach
pub def Id: u8
```

| Name | Value | |
| :--- | :--- | :--- |
| `NONE` | `0` |  |
| `SIZE_OF` | `1` |  |
| `ALIGN_OF` | `2` |  |
| `OFFSET_OF` | `3` |  |
| `LENGTH_OF` | `4` |  |
| `TYPE_ID` | `5` |  |
| `TYPE_NAME` | `6` |  |
| `IS_RECORD` | `7` |  |
| `IS_UNION` | `8` |  |
| `IS_TAG` | `9` |  |
| `IS_POINTER` | `10` |  |
| `IS_SECRET` | `11` |  |
| `HOLDS_SECRET` | `12` |  |
| `IS_INTEGER` | `13` |  |
| `IS_FLOAT` | `14` |  |
| `TYPE_OF` | `15` |  |
| `POINTEE_OF` | `16` |  |
| `DISCRIMINANT_OF` | `17` |  |
| `ZEROIZE` | `21` |  |

### row

```mach
pub fun row(id: Id) *Row
```

the row of `id`; nil for NONE and any id no row declares

### id_of

```mach
pub fun id_of(source: str, name: mach.base.source.Span) Id
```

the intrinsic the name at `name` in `source` spells, without its `$`; NONE when it spells none

### in_class

```mach
pub fun in_class(id: Id, class: Class) bool
```

whether `id` answers within `class`

### call_intrinsic

```mach
pub fun call_intrinsic(a: *mach.front.ast.Ast, source: str, eid: mach.front.ast.id.ExprId) Id
```

the intrinsic a comptime call `$name(...)` names; NONE for any other expression

### ident_intrinsic

```mach
pub fun ident_intrinsic(source: str, full: mach.base.source.Span) Id
```

the intrinsic a `$name` token spells

## Class

```mach
pub def Class: u8
```

what an intrinsic answers

| Field | Type | |
| :--- | :--- | :--- |
| `CLASS_LAYOUT` |  | a constant measured from a type's layout or identity |
| `CLASS_QUERY` |  | a predicate over a type, or its spelling |
| `CLASS_GATE` |  | a type compared inside a `$if` gate, never a value |
| `CLASS_CONSTRUCTOR` |  | a type built from a type, written where a type is expected |
| `CLASS_ITERATION` |  | the members a `$each` walks |
| `CLASS_DIRECTIVE` |  | a compile-time refusal |
| `CLASS_EFFECT` |  | a run-time operation only the compiler can state |

| Name | Value | |
| :--- | :--- | :--- |
| `CLASS_LAYOUT` | `0` |  |
| `CLASS_QUERY` | `1` |  |

## Row

```mach
pub rec Row {
    id:           Id;
    name:         str;
    class:        Class;
    args:         u32;
    type_operand: bool;
}
```

one intrinsic

| Field | Type | |
| :--- | :--- | :--- |
| `args` | `u32` | the argument count it takes |
| `type_operand` | `bool` | its first argument is a type, written in the full type grammar |

## Functions

### constructor_refusal

```mach
pub fun constructor_refusal(a: *std.allocator.Allocator) res[str, mach.base.textbuild.Error]
```

the refusal of a `$` in a type that names no type constructor, listing the ones there are

### first_call_arg

```mach
pub fun first_call_arg(a: *mach.front.ast.Ast, eid: mach.front.ast.id.ExprId) mach.front.ast.id.ExprId
```

### intrinsic_takes_type_operand

```mach
pub fun intrinsic_takes_type_operand(source: str, full: mach.base.source.Span) bool
```

### is_type_of_call

```mach
pub fun is_type_of_call(a: *mach.front.ast.Ast, source: str, eid: mach.front.ast.id.ExprId) bool
```

### type_of_arg

```mach
pub fun type_of_arg(a: *mach.front.ast.Ast, eid: mach.front.ast.id.ExprId) mach.front.ast.id.ExprId
```

### is_fields_call

```mach
pub fun is_fields_call(a: *mach.front.ast.Ast, source: str, eid: mach.front.ast.id.ExprId) bool
```

### is_cases_call

```mach
pub fun is_cases_call(a: *mach.front.ast.Ast, source: str, eid: mach.front.ast.id.ExprId) bool
```

### is_layout_intrinsic_call

```mach
pub fun is_layout_intrinsic_call(a: *mach.front.ast.Ast, source: str, eid: mach.front.ast.id.ExprId) bool
```

### is_type_id_call

```mach
pub fun is_type_id_call(a: *mach.front.ast.Ast, source: str, eid: mach.front.ast.id.ExprId) bool
```

### is_length_of_call

```mach
pub fun is_length_of_call(a: *mach.front.ast.Ast, source: str, eid: mach.front.ast.id.ExprId) bool
```

### is_offset_of_call

```mach
pub fun is_offset_of_call(a: *mach.front.ast.Ast, source: str, eid: mach.front.ast.id.ExprId) bool
```

### is_size_of_call

```mach
pub fun is_size_of_call(a: *mach.front.ast.Ast, source: str, eid: mach.front.ast.id.ExprId) bool
```

### is_align_of_call

```mach
pub fun is_align_of_call(a: *mach.front.ast.Ast, source: str, eid: mach.front.ast.id.ExprId) bool
```

### layout_intrinsic_type_arg

```mach
pub fun layout_intrinsic_type_arg(a: *mach.front.ast.Ast, eid: mach.front.ast.id.ExprId) mach.front.ast.id.ExprId
```

### fields_type_arg

```mach
pub fun fields_type_arg(a: *mach.front.ast.Ast, eid: mach.front.ast.id.ExprId) mach.front.ast.id.ExprId
```

### is_error_call

```mach
pub fun is_error_call(a: *mach.front.ast.Ast, source: str, eid: mach.front.ast.id.ExprId) bool
```

### type_operand_value

```mach
pub fun type_operand_value(a: *mach.front.ast.Ast, source: str, operand: mach.front.ast.id.ExprId) mach.front.ast.id.ExprId
```

### is_type_comparison

```mach
pub fun is_type_comparison(a: *mach.front.ast.Ast, source: str, bin: *mach.front.ast.expr.ExprBinary) bool
```

### is_type_comparison_binary

```mach
pub fun is_type_comparison_binary(a: *mach.front.ast.Ast, source: str, bin: *mach.front.ast.expr.ExprBinary,
    lhs_is_field_type: bool, rhs_is_field_type: bool) bool
```

### is_field_type_member

```mach
pub fun is_field_type_member(a: *mach.front.ast.Ast, source: str, eid: mach.front.ast.id.ExprId) bool
```

### is_field_descriptor_member

```mach
pub fun is_field_descriptor_member(a: *mach.front.ast.Ast, source: str, eid: mach.front.ast.id.ExprId) bool
```

### is_type_query_call

```mach
pub fun is_type_query_call(a: *mach.front.ast.Ast, source: str, eid: mach.front.ast.id.ExprId) bool
```

### is_type_question_call

```mach
pub fun is_type_question_call(a: *mach.front.ast.Ast, source: str, eid: mach.front.ast.id.ExprId) bool
```

a call that asks about a type: its layout, its identity, or a predicate over it

### is_path_call

```mach
pub fun is_path_call(a: *mach.front.ast.Ast, e: mach.front.ast.id.ExprId) bool
```

a call whose callee is a rooted comptime path: `$mach.build.ct_mul(low, 64)`

### is_comptime_value

```mach
pub fun is_comptime_value(a: *mach.front.ast.Ast, e: mach.front.ast.id.ExprId) bool
```

a rooted comptime path, or a call on one: both fold to a constant

### is_comptime_path

```mach
pub fun is_comptime_path(a: *mach.front.ast.Ast, e: mach.front.ast.id.ExprId) bool
```

### comptime_ident_name

```mach
pub fun comptime_ident_name(full: mach.base.source.Span) mach.base.source.Span
```

