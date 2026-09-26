# mach.lang.fe.sema.coerce

## def CoerceKind

```mach
pub def CoerceKind: u8
```

## val COERCE_NOT_APPLICABLE

```mach
pub val COERCE_NOT_APPLICABLE: CoerceKind = 0
```

## val COERCE_COERCED

```mach
pub val COERCE_COERCED:        CoerceKind = 1
```

## val COERCE_OUT_OF_RANGE

```mach
pub val COERCE_OUT_OF_RANGE:   CoerceKind = 2
```

## rec CoerceResult

```mach
pub rec CoerceResult;
```

## fun out_of_range

```mach
pub fun out_of_range(to: type.TypeId, value: wide.Wide, negated: bool, bounds: type.IntRange, literal: bool) CoerceResult;
```

## fun try_coerce_literal

```mach
pub fun try_coerce_literal(sc: *context.SemaContext, eid: id.ExprId, to: type.TypeId) CoerceResult;
```

## fun probe_literal_range

```mach
pub fun probe_literal_range(sc: *context.SemaContext, eid: id.ExprId, to: type.TypeId) CoerceResult;
```

ask whether a literal-shaped expression fits the integer range of `to` without
retyping anything: `COERCE_OUT_OF_RANGE` carries the offending leaf's value and
sign, and any other kind means the shape is not a literal or it fits

sc: the semantic context
eid: the expression to probe
to: the integer type whose range is asked about
ret: the probe result; never commits an expression type

## fun int_bounds_of

```mach
pub fun int_bounds_of(sc: *context.SemaContext, to: type.TypeId) opt[type.IntRange];
```

## fun check_float_literal

```mach
pub fun check_float_literal(sc: *context.SemaContext, eid: id.ExprId, ty: type.TypeId);
```

the float literal rule, applied once the literal's type is final. the literal rounds to
nearest; one that rounds to infinity, or whose nonzero value rounds to zero, is refused,
and an inexact one whose written digits are not the shortest spelling of the value stored
warns with that value. an exact literal never warns

sc: the semantic context the literal was typed in
eid: a float literal
ty: its final type; anything but a float type is left alone

## fun is_assignable

```mach
pub fun is_assignable(sc: *context.SemaContext, from: type.TypeId, to: type.TypeId) bool;
```

