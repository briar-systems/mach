# Operators

## Arithmetic

`+` `-` `*` `/` `%` — work on integer and floating-point scalars. On the seeded
vector types they apply lane-wise, with the honest per-lane table in
[SIMD vectors](#simd-vectors) below (`+ - * /`, no vector `%`).

```mach
val s: i64    = 10 + 20;
val q: f32    = 1.5 * 2.0;
```

`%` is the remainder. On integers it is the native truncated remainder, taking
the sign of the dividend (`-7 % 3 == -1`). On floats it is the truncated (C
`fmod`) remainder `a - trunc(a / b) * b`, likewise taking the sign of the
dividend (`5.5 % 3.0 == 2.5`, `-5.5 % 3.0 == -2.5`). The exact float result is
defined for `|a / b| < 2^63`.

```mach
val r: f64 = 5.5 % 3.0;      # 2.5
```

## Bitwise

`&` `|` `^` `~` `<<` `>>` — work on integer scalars. On integer-lane vectors
`&` `|` `^` `~` apply lane-wise; the shifts `<<` `>>` are not in this increment
(see [SIMD vectors](#simd-vectors)).

```mach
val x: i64    = (a & b) | (c ^ d);
val y: i64    = x << 2;
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

A pointer-like value — a pointer or a function — may be compared against `nil`
(the null-address literal). On the seeded vector types, a comparison produces a
same-shape unsigned **mask** vector (lane-wise) — see [SIMD vectors](#simd-vectors).

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

Neither `::` nor `:~` may add or drop the `^` secret qualifier, and neither can
erase a secret-welded pointer to `ptr`. The only downgrade is the `:^` strip
cast. See [secrecy.md](secrecy.md).

## SIMD vectors

The seeded 128-bit vector types (see [types.md](types.md)) carry lane-wise
operators. The table below is the honest set — what lowers to one instruction per
operator on the SSE2 (x86_64) and NEON (aarch64) baselines. Everything in it also
lowers to a **defined unrolled scalar expansion** with identical results on a
target without the hardware (see [policy.md](policy.md)), so a vector operator
means the same thing on every target.

| Lane family | `+` `-` | `*` | `/` | `%` | `& \| ^ ~` | `<< >>` | `== != < > <= >=` |
|---|---|---|---|---|---|---|---|
| float — `f32x4`, `f64x2` | yes | yes | yes | no | — | no | → same-shape unsigned mask |
| integer, 16-bit lanes — `i16x8`, `u16x8` | yes | yes | no | no | yes | no | → same-shape unsigned mask |
| integer, other widths — `i8x16` `i32x4` `i64x2` (+ unsigned) | yes | no | no | no | yes | no | → same-shape unsigned mask |

Both operands of a binary operator must be the **same** vector shape: there is no
implicit scalar↔vector mixing and no cross-shape widening. Anything the table
marks `no` is a compile error, not a silent fallback:

- no vector `%` on any lane type;
- no integer vector `/` — division is float lanes only;
- integer vector `*` is 16-bit lanes only (32-bit `pmulld` is SSE4.1, off the SSE2
  baseline);
- bitwise `& | ^ ~` require integer lanes; the shifts `<< >>` are not in this
  increment (a per-lane variable shift is AVX2-only on x86_64, with no 8-bit
  packed form).

A comparison produces the same-shape **unsigned mask** vector — one lane per input
lane, all-ones bits (`0xFF…`) for true and all-zeros for false, exactly what the
hardware compare yields. There is no vector-bool type. The mask element is the
unsigned integer of the input's lane width: `f32x4` / `i32x4` / `u32x4` → `u32x4`;
`f64x2` / `i64x2` → `u64x2`; `i16x8` → `u16x8`; `i8x16` → `u8x16`. Select/blend is
not an operator; it is the library idiom `(mask & a) | (~mask & b)` over matching
integer lanes (the tier-3 simd library, #2021).

```mach
val a: f32x4 = f32x4{1.0, 2.0, 3.0, 4.0};
val b: f32x4 = f32x4{4.0, 3.0, 2.0, 1.0};
val sum:  f32x4 = a + b;       # lane-wise -> {5.0, 5.0, 5.0, 5.0}
val mask: u32x4 = a < b;       # -> {0xFFFFFFFF, 0xFFFFFFFF, 0, 0}

val m: i32x4 = i32x4{1, 2, 3, 4};
val n: i32x4 = i32x4{4, 3, 2, 1};
val z: i32x4 = (m & n) ^ n;    # lane-wise bitwise on integer lanes
```

## See also

- [expressions.md](expressions.md) — how operators compose into expressions
- [types.md](types.md) — which types support which operators
- [secrecy.md](secrecy.md) — the `^` secret qualifier and the `:^` strip cast
