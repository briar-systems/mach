[mach](../../README.md) / [front](../README.md) / [sema](README.md) / **coerce**

# mach.front.sema.coerce

literal coercion, and the rule for when a value of one type may be assigned to another

an unsuffixed integer literal, or an expression built from such literals under the operators a
literal coerces through, takes the type it is checked against when its value fits that type's
range, as a comptime integer value must. a float literal's rounding is checked once its type is
final. assignment may add secrecy, spreads `^[N]T` to `[N]^T`, and converts `ptr` to and from
a typed pointer that reaches no secret.

<sub>source: [`src/front/sema/coerce.mach`](../../../../src/front/sema/coerce.mach) · imported by 4 modules</sub>

<sub>`CoerceKind` is private · `type_kind` is [`mach.type.kind`](../../type/kind.md) · `wide` is [`mach.base.wide`](../../base/wide.md) · `type_prim` is [`mach.type.prim`](../../type/prim.md) · `sema_context` is [`mach.front.sema.context`](context.md) · `ast_id` is [`mach.front.ast.id`](../ast/id.md) · `fail` is [`mach.base.fail`](../../base/fail.md) · `comptime_value` is [`mach.front.comptime.value`](../comptime/value.md)</sub>

| Type | | |
| :--- | :--- | :--- |
| [`CoerceResult`](#coerceresult) | `rec` |  |

| On `CoerceResult` | |
| :--- | :--- |
| [`out_of_range`](#out_of_range) · [`try_coerce_literal`](#try_coerce_literal) | |
| [`probe_literal_range`](#probe_literal_range) | ask whether a literal-shaped expression fits the integer range of `to` without retyping anything: `COERCE_OUT_OF_RANGE` carries the offending leaf's value and sign, and any other kind means the shape is not a literal or it fits |
| [`fit_ct_int`](#fit_ct_int) | whether a comptime integer value fits integer type `to`, retyping nothing: not applicable unless both are integers |

| Functions | |
| :--- | :--- |
| [`is_untyped_int_literal`](#is_untyped_int_literal) | whether an expression is built only from unsuffixed integer literals under the operators a literal coerces through, so its exact value is still untyped |
| [`int_bounds_of`](#int_bounds_of) | |
| [`check_float_literal`](#check_float_literal) | the float literal rule, applied once the literal's type is final |
| [`is_assignable`](#is_assignable) | |

| Constants | |
| :--- | :--- |
| [`COERCE_COERCED`](#constants) · [`COERCE_OUT_OF_RANGE`](#constants) | |

---

## CoerceResult

```mach
pub rec CoerceResult {
    kind:    CoerceKind;
    ty:      type_kind.TypeId;
    value:   wide.Wide;
    negated: bool;
    bounds:  type_prim.IntRange;

    literal: bool;
}
```

### out_of_range

```mach
pub fun out_of_range(to: type_kind.TypeId, value: wide.Wide, negated: bool, bounds: type_prim.IntRange, literal: bool) CoerceResult
```

### try_coerce_literal

```mach
pub fun try_coerce_literal(sc: *sema_context.SemaContext, eid: ast_id.ExprId, to: type_kind.TypeId) res[CoerceResult, fail.Fail]
```

### probe_literal_range

```mach
pub fun probe_literal_range(sc: *sema_context.SemaContext, eid: ast_id.ExprId, to: type_kind.TypeId) res[CoerceResult, fail.Fail]
```

ask whether a literal-shaped expression fits the integer range of `to` without
retyping anything: `COERCE_OUT_OF_RANGE` carries the offending leaf's value and
sign, and any other kind means the shape is not a literal or it fits

| Parameter | |
| :--- | :--- |
| `sc` | the semantic context |
| `eid` | the expression to probe |
| `to` | the integer type whose range is asked about |
| **returns** | the probe result, or the failure of a refused read; never commits an expression type |

### fit_ct_int

```mach
pub fun fit_ct_int(sc: *sema_context.SemaContext, ctval: comptime_value.CTValue, to: type_kind.TypeId) CoerceResult
```

whether a comptime integer value fits integer type `to`, retyping nothing:
not applicable unless both are integers

## Functions

### is_untyped_int_literal

```mach
pub fun is_untyped_int_literal(sc: *sema_context.SemaContext, eid: ast_id.ExprId) res[bool, fail.Fail]
```

whether an expression is built only from unsuffixed integer literals under the
operators a literal coerces through, so its exact value is still untyped

| Parameter | |
| :--- | :--- |
| `sc` | the semantic context |
| `eid` | the expression to ask about |
| **returns** | true when every leaf is an unsuffixed integer literal, or the failure of a refused read |

### int_bounds_of

```mach
pub fun int_bounds_of(sc: *sema_context.SemaContext, to: type_kind.TypeId) opt[type_prim.IntRange]
```

### check_float_literal

```mach
pub fun check_float_literal(sc: *sema_context.SemaContext, eid: ast_id.ExprId, ty: type_kind.TypeId)
```

the float literal rule, applied once the literal's type is final. the literal rounds to
nearest; one that rounds to infinity, or whose nonzero value rounds to zero, is refused,
and an inexact one whose written digits are not the shortest spelling of the value stored
warns with that value. an exact literal never warns

| Parameter | |
| :--- | :--- |
| `sc` | the semantic context the literal was typed in |
| `eid` | a float literal |
| `ty` | its final type; anything but a float type is left alone |

### is_assignable

```mach
pub fun is_assignable(sc: *sema_context.SemaContext, from: type_kind.TypeId, to: type_kind.TypeId) bool
```

## Constants

| Name | Type | Value | |
| :--- | :--- | :--- | :--- |
| `COERCE_COERCED` | `CoerceKind` | `1` |  |
| `COERCE_OUT_OF_RANGE` | `CoerceKind` | `2` |  |

