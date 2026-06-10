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
`bool`; `bool` is a stdlib alias for `u8`. Comparisons relate **mathematical
values**, so the result is identical in either operand order:

- **integer vs integer** — any signedness and width mix is legal and compares
  the true values (e.g. a negative `i64` is never equal to, and always less
  than, any `u64`). Width aliases (`usize`, `isize`) follow their backing type.
- **float vs float** — any width mix is legal; the narrower operand widens
  exactly (`f32` -> `f64`).
- **integer vs float** — a compile error; cast one operand explicitly with
  `::`. An implicit widening would hide `f64` rounding above `2^53`.

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

Two postfix cast operators, both written `expr OP Type`:

- `expr::Type` — **value conversion**. Resizes integers (sign- or zero-extend,
  truncate), converts between integer and float (a numeric `CVT`), and is the
  identity on a same-type operand. Value-preserving where representable.
- `expr:~Type` — **bit reinterpret**. Reads the operand's exact bits as the
  target type with no conversion. Legal only when `Type` has the same byte size
  as the operand's type (a size mismatch is a compile error). The `~` recalls
  its bitwise heritage, so `:~` reads as "bit cast".

The two differ sharply on int<->float. `::` runs a numeric conversion, while
`:~` reinterprets the raw bit pattern:

```mach
val a: u64 = some_i64::u64;     # value conversion (resize)
val p: *u8 = some_ptr::*u8;     # pointer value, retyped

val n: u64 = 1.5::u64;          # 1                  (float -> int conversion)
val b: u64 = 1.5:~u64;          # 0x3FF8000000000000 (raw IEEE-754 bits)
val f: f64 = b:~f64;            # 1.5                (bits read back as a float)
```

## See also

- [expressions.md](expressions.md) — how operators compose into expressions
- [types.md](types.md) — which types support which operators
