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
pub fun out_of_range(to: type.TypeId, value: u64, negated: bool, bounds: type.IntRange, literal: bool) CoerceResult;
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

## fun is_assignable

```mach
pub fun is_assignable(sc: *context.SemaContext, from: type.TypeId, to: type.TypeId) bool;
```

