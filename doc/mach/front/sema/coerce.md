[mach](../../README.md) / [front](../README.md) / [sema](README.md) / **coerce**

# mach.front.sema.coerce

literal coercion, and the rule for when a value of one type may be assigned to another

an unsuffixed integer literal, or an expression built from such literals under the operators a
literal coerces through, takes the type it is checked against when its value fits that type's
range, as a comptime integer value must. a float literal's rounding is checked once its type is
final. assignment may add secrecy, spreads `^[N]T` to `[N]^T`, and converts `ptr` to and from
a typed pointer that reaches no secret.

Source: [`src/front/sema/coerce.mach`](../../../../src/front/sema/coerce.mach)

Uses types from [`mach.base.fail`](../../base/fail.md), [`mach.base.wide`](../../base/wide.md), [`mach.front.ast.id`](../ast/id.md), [`mach.front.comptime.value`](../comptime/value.md), [`mach.front.sema.context`](context.md), [`mach.type.kind`](../../type/kind.md), [`mach.type.prim`](../../type/prim.md).

| Type | | |
| :--- | :--- | :--- |
| [`CoerceResult`](#coerceresult) | `rec` |  |

| On `CoerceResult` | |
| :--- | :--- |
| [`out_of_range`](#out_of_range), [`try_coerce_literal`](#try_coerce_literal) | |
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
| [`COERCE_COERCED`](#constants), [`COERCE_OUT_OF_RANGE`](#constants) | |

---

## CoerceResult

```mach
pub rec CoerceResult {
    kind:    CoerceKind;
    ty:      mach.type.kind.TypeId;
    value:   mach.base.wide.Wide;
    negated: bool;
    bounds:  mach.type.prim.IntRange;

    literal: bool;
}
```

### out_of_range

```mach
pub fun out_of_range(to: mach.type.kind.TypeId, value: mach.base.wide.Wide, negated: bool, bounds: mach.type.prim.IntRange, literal: bool) CoerceResult
```

### try_coerce_literal

```mach
pub fun try_coerce_literal(sc: *mach.front.sema.context.SemaContext, eid: mach.front.ast.id.ExprId, to: mach.type.kind.TypeId) res[CoerceResult, mach.base.fail.Fail]
```

### probe_literal_range

```mach
pub fun probe_literal_range(sc: *mach.front.sema.context.SemaContext, eid: mach.front.ast.id.ExprId, to: mach.type.kind.TypeId) res[CoerceResult, mach.base.fail.Fail]
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
pub fun fit_ct_int(sc: *mach.front.sema.context.SemaContext, ctval: mach.front.comptime.value.CTValue, to: mach.type.kind.TypeId) CoerceResult
```

whether a comptime integer value fits integer type `to`, retyping nothing:
not applicable unless both are integers

## Functions

### is_untyped_int_literal

```mach
pub fun is_untyped_int_literal(sc: *mach.front.sema.context.SemaContext, eid: mach.front.ast.id.ExprId) res[bool, mach.base.fail.Fail]
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
pub fun int_bounds_of(sc: *mach.front.sema.context.SemaContext, to: mach.type.kind.TypeId) opt[mach.type.prim.IntRange]
```

### check_float_literal

```mach
pub fun check_float_literal(sc: *mach.front.sema.context.SemaContext, eid: mach.front.ast.id.ExprId, ty: mach.type.kind.TypeId)
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
pub fun is_assignable(sc: *mach.front.sema.context.SemaContext, from: mach.type.kind.TypeId, to: mach.type.kind.TypeId) bool
```

## Constants

| Name | Type | Value | |
| :--- | :--- | :--- | :--- |
| `COERCE_COERCED` | `CoerceKind` | `1` |  |
| `COERCE_OUT_OF_RANGE` | `CoerceKind` | `2` |  |

