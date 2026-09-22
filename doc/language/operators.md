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
dividend (`5.5 % 3.0 == 2.5`, `-5.5 % 3.0 == -2.5`). For finite operands and a
nonzero divisor, this applies across the finite operand range, including
quotients beyond the `i64` range.

```mach
use std.runtime;
use print: std.print;

#[symbol("main")]
fun main(argc: i64, argv: **u8) i64 {
    val r: f64 = 5.5 % 3.0;      # 2.5
    val s: f64 = -5.5 % 3.0;     # -2.5
    val t: i64 = -7 % 3;         # -1
    print.printlnf("{} {} {}", r, s, t);
    ret 0;
}
```

**Widening multiply.** A multiply whose operands are both conversions from
one narrower integer type to a type exactly twice as wide is the full
product of the narrow operands, and compiles to the target's widening
instruction rather than a multiply at the wide width. This is how a 64 x 64
product is written at 128 bits, and the two halves a program selects from
it are single instructions on every 64-bit target:

```mach fragment
val full: u128 = (a::u128) * (b::u128);       # a, b: u64; one widening multiply
val hi:   u64  = (full >> 64)::u64;            # the high-half multiply
val lo:   u64  = full::u64;                    # the plain multiply
```

Both operands must be the same conversion (both zero-extensions or both
sign-extensions); a mixed-sign product is an ordinary multiply at the wide
width. See [types.md](types.md#128-bit-integers).

**`*` on a secret operand.** `^T * T` and `^T * ^T` are the same wrapping,
same-width product with a `^T` result: the operator means the same thing on a
secret, and nothing declassifies. What the operand's secrecy changes is
whether the target may execute it. A secret `/` or `%` is always refused, and a
secret `*` compiles only where the instruction set declares the exact multiply
it emits (the low half, a high half or the widening product, at that operand
width) as data-independent-timing under a condition the build meets: on x86-64
every scalar cell unconditionally, on aarch64 under PSTATE.DIT on linux and
darwin, on riscv64 with `m` and `zkt` selected, and nowhere else. An undeclared
cell is a compile error at lowering, never a slower substitute. The widening
form above carries through: `(a::^u128) * (b::^u128)` over 64-bit secrets is
the 64-bit widening cell, and its halves are `^u64`. The per-instruction-set
table and the conditions are in
[secrecy.md](secrecy.md#constant-time-multiply-by-instruction-set), and
`$mach.build.ct_mul(op, width)` answers the same question at comptime
([comptime-mach.md](comptime-mach.md)).

## Bitwise

`&` `|` `^` `~` `<<` `>>` — work on integer scalars. On integer-lane vectors
`&` `|` `^` `~` apply lane-wise; the shifts `<<` `>>` are not in this increment
(see [SIMD vectors](#simd-vectors)).

```mach fragment
val x: i64    = (a & b) | (c ^ d);
val y: i64    = x << 2;
```

A shift's result has the left operand's type, and its count is any integer
type. `<<` shifts zeros in from the right; `>>` on an unsigned operand shifts
zeros in from the left and on a signed operand copies the sign bit in. A count
at or above the left operand's width **saturates**: `<<` and an unsigned `>>`
answer `0`, a signed `>>` answers the sign fill (`0` or `-1`). A count that is
a compile-time constant at or above the width, or negative, is a compile
error, since a program never means the saturated value by it:

```mach fragment
val a: u32 = x << 31;        # ok
val b: u32 = x << 32;        # error: shift count 32 is at least the width of `u32` (32 bits)
val c: u32 = x >> n;         # n: u8 at run time; 0 when n >= 32
val d: i32 = y >> n;         # -1 or 0 when n >= 32, the sign of y
```

The saturation is branch-free, so a secret count admitted by the constant-time
gates (see [secrecy.md](secrecy.md)) stays admitted. A count already masked
below the width, such as `x << (n & 31)` on a `u32`, needs no saturation and
compiles to the bare shift.

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

- **pointer vs pointer** — every one of the six operators accepts two
  pointer-like operands (a pointer, a `ptr`, a function, or `nil`), whatever
  their pointee types and whatever their pointees' secrecy. Addresses order as
  unsigned values of pointer width, and the result is a public `u8` even when
  both pointees are secret: ordering reveals no more than the `==` beside it,
  and no address comes back out of it. See
  [secrecy.md](secrecy.md#comparing-and-ordering-addresses).
- **pointer vs integer** — a compile error. Ordering relates two addresses; it
  is not a route from an address to an integer.

On the seeded vector types, a comparison produces a
same-shape unsigned **mask** vector (lane-wise) — see [SIMD vectors](#simd-vectors).

`==` / `!=` on an **aggregate value** (a `rec`, `uni`, or whole `tag`) is a compile error.
Comparing representations would silently relate padding bytes and unwritten union
variants, so no whole-value structural equality is provided. Write an explicit field-wise
comparison for records. Comparing pointers to aggregates is unaffected, and the rejection
applies to a generic instantiated at an aggregate type as well as to a concrete one.

For tagged values, `==` and `!=` are rejected entirely: there is no whole-tag
equality, no payload equality and no ordering. Test which case is active with the
`sel place.case` expression instead. See [tag.md](tag.md).

## Logical

`&&` `||` `!` — short-circuiting. Operands are `u8` (`0` is false, nonzero is
true); the result is `u8` (`1` or `0`).

```mach fragment
val ok: u8 = (x > 0) && (y < 100);
```

## Unary

- `-` numeric negation
- `~` bitwise NOT (integer)
- `!` logical NOT (`u8`)

On a float, `-` is the IEEE-754 sign-bit flip and is exact for every operand, so
`-0.0` is negative zero — a constant distinct from `0.0`, whether it is folded at
comptime or negated at run time. The two compare **equal** (`-0.0 == 0.0` is
true), so code that must tell them apart compares bit patterns: `(-0.0):~u64` is
`0x8000000000000000`. Note that `0.0 - x` is subtraction, not negation: it yields
positive zero for either zero.

## Pointer

- `?place` — address-of; produces a pointer to the operand. The operand must be a place: a binding, a field, an element, or a dereference (a field or element reached through a pointer counts). Taking the address of a call result, a literal, a cast, an operator result, or any other temporary is an error naming the operand kind.
- `@ptr` — dereference; reads through the pointer.

```mach
use std.runtime;
use print: std.print;

#[symbol("main")]
fun main(argc: i64, argv: **u8) i64 {
    var x: i64  = 9;
    var p: *i64 = ?x;
    @p = 11;                    # write through
    val v: i64  = @p;           # read through
    print.printlnf("{}", v);
    ret 0;
}
```

```mach error cannot take the address of a call result
fun g() i64 { ret 1; }

fun addresses(x: i64) {
    val b: *i64 = ?g();         # error: cannot take the address of a call result
    val c: *i64 = ?42;          # error: cannot take the address of a literal
    val d: *u64 = ?(x::u64);    # error: cannot take the address of a cast result
    val e: *i64 = ?(x + 1);     # error: cannot take the address of an operator result
}
```

Each of those reads, in full, `cannot take the address of a call result: `?`
applies to a place (a binding, a field, an element, or a dereference)`. Bind
the temporary to a `var` and take that binding's address.

## Cast

Two postfix cast operators, both written `expr OP Type`:

- `expr::Type` — **value conversion**. Resizes integers (sign- or zero-extend,
  truncate), converts between integer and float (a numeric `CVT`), and is the
  identity on a same-type operand. Value-preserving where representable. When
  either type is nonnumeric, equal sizes are required and the bits are reinterpreted.
  Constant expressions follow these rules at every nesting depth, including casts
  through type aliases.
- `expr:~Type` — **bit reinterpret**. Reads the operand's exact bits as the
  target type with no conversion. Legal only when `Type` has the same byte size
  as the operand's type (a size mismatch is a compile error). The `~` recalls
  its bitwise heritage, so `:~` reads as "bit cast".

On two vector types, `::` converts lane by lane: each lane goes through exactly
the scalar `::` above, so `i32x4::f32x4` converts every lane numerically and
`i8x4::i64x4` sign-extends every lane. Both sides need the same lane count
(`i32x4::i64x2` is an error even though the two are the same size), and any
pair of lane types is allowed, including equal-size integer and float lanes
and a signedness change. A lane converts exactly as its scalar would on the
same target, including NaN, the infinities and values outside the
destination type. There is no cast between a vector and a scalar. The raw
bits of a vector are `:~`, which, like every `:~`, needs only equal byte
sizes (`i32x4:~f32x4`, `i32x4:~i64x2`).

The two differ sharply on int<->float. `::` runs a numeric conversion, while
`:~` reinterprets the raw bit pattern:

```mach
use std.runtime;
use print: std.print;

#[symbol("main")]
fun main(argc: i64, argv: **u8) i64 {
    val some_i64: i64 = -1;
    val a: u64 = some_i64::u64;     # value conversion (resize)
    val p: *u8 = argv::*u8;         # pointer value, retyped

    val n: u64 = 1.5::u64;          # 1                  (float -> int conversion)
    val b: u64 = 1.5:~u64;          # 0x3FF8000000000000 (raw IEEE-754 bits)
    val f: f64 = b:~f64;            # 1.5                (bits read back as a float)
    print.printlnf("{} {:x} {}", n, b, f);
    ret 0;
}
```

Neither `::` nor `:~` may add or drop the `^` secret qualifier, and neither can
erase a secret-welded pointer to `ptr`. Representation-changing `::` and `:~` casts
are rejected when either by-value representation contains a tag, including through
records, arrays, or union variants. Transparent aliases preserve the tag type.
The only secrecy downgrade is the `:>T` strip cast, which removes outer secrecy
from `^Tag` without altering the active case or inner payload qualifiers.
See [secrecy.md](secrecy.md) and [tag.md](tag.md).

## SIMD vectors

Vector types (see [types.md](types.md)) carry lane-wise operators at every lane
count, not only the 128-bit shapes. **Which operators are legal is
target-independent.** The table below is the whole surface, and it is identical on
every target and at every width — a `f32x8` add is as legal as a `f32x4` one, and
the two differ only in how they are realized.

| Lane family | `+` `-` | `*` | `/` | `%` | `& \| ^ ~` | `<< >>` | `== != < > <= >=` |
|---|---|---|---|---|---|---|---|
| float — `f32x4`, `f64x2` | yes | yes | yes | no | — | no | → same-shape unsigned mask |
| integer — `i8x16` `i16x8` `i32x4` `i64x2` (+ unsigned) | yes | yes | yes | no | yes | no | → same-shape unsigned mask |

Both operands of a binary operator must be the **same** vector shape: there is no
implicit scalar↔vector mixing and no cross-shape widening. Anything the table
marks `no` is a compile error, not a silent fallback:

- no vector `%` on any lane type;
- bitwise `& | ^ ~` require integer lanes; the shifts `<< >>` are not in this
  increment (a per-lane variable shift is AVX2-only on x86_64, with no 8-bit
  packed form).

Integer division uses each lane's signedness and scalar division behavior, including
truncation toward zero for signed quotients and the scalar behavior for division by
zero or signed overflow. A secret dividend or divisor is rejected because integer
division has variable latency. x86_64 and aarch64 realize integer vector division
as scalar lane operations, as does RISC-V without a vector unit.

### Legality is target-independent; realization is not

A legal operator means the same thing on every target, but not every target has a
packed instruction for every one. The backend picks, in order: the packed form
where the target has one, else a **defined unrolled scalar expansion** with
lane-identical results (see [policy.md](policy.md)). Neither choice changes the
answer, so nothing about the surface depends on it.

Integer `*` is where this is most visible today:

| shape | x86_64 (SSE2) | aarch64 (NEON) | riscv64 (no vector unit) |
|---|---|---|---|
| `i8x16 * i8x16` | scalar expansion | packed `mul .16b` | scalar expansion |
| `i16x8 * i16x8` | packed `pmullw` | packed `mul .8h` | scalar expansion |
| `i32x4 * i32x4` | packed `pmuludq` pair (`pmulld` under `sse41`) | packed `mul .4s` | scalar expansion |
| `i64x2 * i64x2` | packed `pmuludq` triple | scalar expansion (NEON has no `.2d` multiply) | scalar expansion |

Operators never widen implicitly, so a widening multiply is spelled as two lane
casts and a multiply: `a::i32x4 * b::i32x4` for `a, b: i16x4`. When both operands
are extensions of the same narrower vector type, with the same signedness, and
the whole product fits one 128-bit register, the backend emits the target's
widening multiply for that cell:

| operands | x86_64 (SSE2) | aarch64 (NEON) | riscv64 (no vector unit) |
|---|---|---|---|
| `i8x8` / `u8x8` → 16-bit lanes | extend, then multiply | `smull` / `umull .8b` | extend, then multiply |
| `i16x4` / `u16x4` → 32-bit lanes | `pmullw` + `pmulhw` / `pmulhuw` | `smull` / `umull .4h` | extend, then multiply |
| `i32x2` → `i64x2` | extend, then multiply (`pmuldq` is SSE4.1) | `smull .2s` | extend, then multiply |
| `u32x2` → `u64x2` | `pmuludq` | `umull .2s` | extend, then multiply |

A wider product, such as `i16x8` → `i32x8`, is a 256-bit value and keeps the
extend-then-multiply path. Either path gives the same lanes.

A project that cannot afford a scalar expansion sets `simd = "require"` in its
profile (see [manifest.md](manifest.md)), which turns the shortfall into a
build error naming the operation, its lane width, the function and the target.

A comparison produces the same-shape **unsigned mask** vector — one lane per input
lane, all-ones bits (`0xFF…`) for true and all-zeros for false, exactly what the
hardware compare yields. There is no vector-bool type. The mask element is the
unsigned integer of the input's lane width: `f32x4` / `i32x4` / `u32x4` → `u32x4`;
`f64x2` / `i64x2` → `u64x2`; `i16x8` → `u16x8`; `i8x16` → `u8x16`. Select/blend is
not an operator; it is the library idiom `(mask & a) | (~mask & b)` over matching
integer lanes (the tier-3 simd library, #2021).

```mach fragment
val a: f32x4 = f32x4{1.0, 2.0, 3.0, 4.0};
val b: f32x4 = f32x4{4.0, 3.0, 2.0, 1.0};
val sum:  f32x4 = a + b;       # lane-wise -> {5.0, 5.0, 5.0, 5.0}
val mask: u32x4 = a < b;       # -> {0xFFFFFFFF, 0xFFFFFFFF, 0, 0}

val m: i32x4 = i32x4{1, 2, 3, 4};
val n: i32x4 = i32x4{4, 3, 2, 1};
val z: i32x4 = (m & n) ^ n;    # lane-wise bitwise on integer lanes
```

## An operand typed by a type parameter

Inside a generic, an operand whose type is a parameter has no operand class yet:
`T` is not an integer, not a float and not a pointer, and asking would be asking
about a placeholder. Every operator on such an operand is decided at the
instantiation instead, against that instance's concrete type, and the table above
is what it is checked against there. A type that does not support the operator is
refused at the instantiation that asked for it. See
[fun.md](fun.md).

## See also

- [expressions.md](expressions.md) — how operators compose into expressions
- [fun.md](fun.md) — an operator on a generic type parameter
- [types.md](types.md) — which types support which operators
- [secrecy.md](secrecy.md) — the `^` secret qualifier and the `:>T` strip cast
