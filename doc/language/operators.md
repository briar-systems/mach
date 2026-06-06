# Operators

SIMD vector operands described below are part of the planned vector
design and are not yet implemented — see [types.md](types.md).

## Arithmetic

`+` `-` `*` `/` `%` — work on integer and floating-point scalars. SIMD
vector types are intended to support the same operators, applied lane-wise.

```mach
val s: i64    = 10 + 20;
val q: f32    = 1.5 * 2.0;
val v: f32x4  = a + b;       # lane-wise add (vectors not yet implemented)
```

## Bitwise

`&` `|` `^` `~` `<<` `>>` — work on integer scalars and (planned) integer
SIMD vectors.

```mach
val x: i64    = (a & b) | (c ^ d);
val y: i64    = x << 2;
val z: i32x4  = (m & n) ^ k;     # vectors not yet implemented
```

## Comparison

`==` `!=` `<` `>` `<=` `>=` — produce `u8` (`1` or `0`). Mach has no compiler
`bool`; `bool` is a stdlib alias for `u8`. Compare values of matching types.
A pointer may be compared against `nil` (the null-address literal). On the
planned SIMD vectors, comparison produces a mask vector (lane-wise).

## Logical

`&&` `||` `!` — short-circuiting. Operands are `u8` (`0` is false, nonzero is
true); the result is `u8` (`1` or `0`).

```mach
val ok: u8 = (x > 0) && (y < 100);
```

## Unary

- `-` numeric negation
- `~` bitwise NOT (integer)
- `!` logical NOT (`u8`)

## Pointer

- `?expr` — address-of; produces a pointer to the operand.
- `@ptr` — dereference; reads through the pointer.

```mach
var x: i64  = 9;
var p: *i64 = ?x;
@p = 11;                    # write through
val v: i64  = @p;           # read through
```

## Cast

`expr::Type` — explicit conversion. Used for numeric width changes,
signedness changes, pointer reinterpretation, etc.

```mach
val a: u64 = some_i64::u64;
val p: *u8 = some_ptr::*u8;
```

## See also

- [expressions.md](expressions.md) — how operators compose into expressions
- [types.md](types.md) — which types support which operators
