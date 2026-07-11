# Types

Mach has a small set of compiler-shipped concrete types plus a uniform
type-construction grammar for pointers, arrays, and function types. There
are no compiler-known type aliases — names like `bool`, `usize`, and `str`
are stdlib `def`s.

## Primitive scalars

| Family | Members |
|---|---|
| Unsigned int | `u8`, `u16`, `u32`, `u64` |
| Signed int | `i8`, `i16`, `i32`, `i64` |
| Float | `f32`, `f64` |
| Untyped pointer | `ptr` |

These eleven names are the complete set of compiler-seeded primitive types.

There is no compiler `bool`. `bool` is a stdlib `def bool: u8;` with `true` /
`false` as stdlib `val`s (`1` / `0`) — see [def.md](def.md).

## SIMD vectors

Ten 128-bit vector types are compiler-seeded, each a fixed number of lanes of a
primitive numeric element:

```mach
f32x4  f64x2                    # float lanes
i8x16  i16x8  i32x4  i64x2      # signed integer lanes
u8x16  u16x8  u32x4  u64x2      # unsigned integer lanes
```

These ten are the complete set. The spelling is `<u|i|f><width>x<count>` with a
**single** `x` — there are no other vector types and no multi-`x` shapes. A name
like `f32x4x4` is not a vector type (it resolves as an ordinary identifier); a
matrix is an algorithm over vectors and belongs in a library over vector
elements, not the language. A vector spelling is recognized only in type
position, so a value may still be named `f32x4` without colliding with the type.

Every seeded vector is exactly 128 bits: `$size_of` and `$align_of` are both
`16`. Arrays of vectors (`[4]f32x4`) and pointers to vectors (`*f32x4`) are
ordinary composite types over a vector element.

**Literals** are full-arity — one initializer per lane, mirroring array literals.
The lane count must match exactly; too few or too many lanes is a compile error.

```mach
val v: f32x4 = f32x4{1.0, 2.0, 3.0, 4.0};
val w: i32x4 = i32x4{1, 2, 3, 4};
```

An uninitialized vector local default-initializes to all-zero lanes:

```mach
var z: i32x4;                   # every lane is 0
```

**Lane access** `v[i]` reads or writes a single lane. The index must be a
comptime constant in `[0, lanes)` and is bounds-checked at compile time; a
dynamic (runtime) lane index is not supported in this increment.

```mach
var v: f32x4 = f32x4{1.0, 2.0, 3.0, 4.0};
val x: f32 = v[0];              # read lane 0
v[3] = 9.0;                     # write lane 3
```

There are no scalar↔vector casts in this increment: neither an implicit
scalar-to-vector conversion nor a `1.0::f32x4` reinterpret is legal. The
lane-wise operators and the comparison-to-mask rule are in
[operators.md](operators.md); what a target without hardware SIMD does with a
vector operator is the `simd` profile lever ([manifest.md](manifest.md),
[policy.md](policy.md)).

## Pointer

`*T` — pointer to a value of type `T`.

```mach
var x: i64;
var p: *i64 = ?x;       # address-of yields a pointer
val v: i64  = @p;       # dereference reads through it
```

## Array

`[N]T` — array of exactly `N` values of type `T`. Nested: `[N][M]T`.

```mach
val a: [4]i64    = [4]i64{1, 2, 3, 4};
val g: [2][2]i64 = [2][2]i64{ [2]i64{1, 2}, [2]i64{3, 4} };
```

## Function type

`fun(T1, T2) R` — first-class function-pointer type.

```mach
def BinOp: fun(i64, i64) i64;
val op: BinOp = add;
val r:  i64   = op(2, 3);
```

## Record and union types

`rec` and `uni` declarations produce named types. See [rec.md](rec.md) and
[uni.md](uni.md).

## Type aliases

`def NAME: TYPE;` introduces an alias. See [def.md](def.md).

## See also

- [secrecy.md](secrecy.md) — the `^` secret qualifier over any of these types
- [operators.md](operators.md) — what operations work on each type
- [comptime-intrinsics.md](comptime-intrinsics.md) — `$size_of`, `$align_of`
