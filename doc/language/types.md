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

A vector type is a **form**, not a fixed list: any primitive numeric element
followed by `x` and a lane count.

On a 128-bit target the spellings this currently accepts are:

```mach
f32x4  f64x2                    # float lanes
i8x16  i16x8  i32x4  i64x2      # signed integer lanes
u8x16  u16x8  u32x4  u64x2      # unsigned integer lanes
```

The spelling is `<u|i|f><width>x<count>` with a **single** `x`. A name like
`f32x4x4` is not a vector type (it resolves as an ordinary identifier); a matrix
is an algorithm over vectors and belongs in a library over vector elements, not
the language.

A vector spelling is recognized only in **type position**, so a *value* may still
be named `f32x4` without colliding with the type:

```mach
val f32x4: i64 = 7;             # fine: values are a different position
```

A **type** may not. `rec`, `uni`, and `def` reject a name spelled as a vector
form, because a type declared with a vector's name would be silently unreachable
— every use in type position resolves to the vector instead:

```mach
rec f32x3 { x: f32; }           # error: `f32x3` is spelled as a vector type
```

This holds for any well-formed spelling, including ones this target does not
currently admit (`f32x3`, `f32x7`), so the name cannot be claimed and then
collide when the bound moves.

Three rules bound the form:

- **At least 2 lanes.** `f32x1` is refused; a one-lane vector is just its scalar.
- **No wider than the target's vector register.** Every current target is 128
  bits, so `f32x7` (224 bits) is refused by name. The error names the width you
  asked for and the width the target has, rather than reporting an unknown type.
- **Currently, exactly as wide as the register.** A vector *narrower* than the
  vector register (`f32x3`, `f32x2`) is read and laid out correctly but refused
  by name for now, because codegen cannot yet carry a value whose register width
  and memory footprint differ. This restriction is temporary and is the only
  thing between the form and `vec3`.

`ptr` is not a lane element: its width is target-defined rather than a scalar bit
count, so `ptrx2` is not a vector spelling.

### Size and alignment

`$size_of` is **lane-derived**: `lanes × element size`, packed, with no padding
up to the register width. (The sub-register rows below are the rule the layout
already implements; those spellings are not yet accepted — see above.)

| type | `$size_of` | `$align_of` |
|---|---|---|
| `f32x4` | 16 | 16 |
| `f32x3` | 12 | 4 |
| `f32x2` | 8 | 4 |
| `i16x4` | 8 | 2 |

`$align_of` is deliberately **discontinuous at the register width**. A vector
that *fills* the vector register aligns to its whole size, because the machine's
vector load requires it. A narrower one is a packed aggregate that scalarizes or
loads piecewise, so it aligns to a single lane. This is what makes `[N]f32x3` a
usable packed vertex buffer: padding `f32x3` to 16 bytes would make it
indistinguishable from `f32x4` in memory.

Arrays of vectors (`[4]f32x4`) and pointers to vectors (`*f32x4`) are ordinary
composite types over a vector element.

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
comptime constant in `[0, lanes)`; a dynamic (runtime) lane index is not
supported in this increment. The bound is the same compile-time rule an array
gets, reported the same way — `v[4]` on a `u32x4` is `index 4 is out of bounds
for `u32x4` of length 4`.

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

## Image and sampler handles

A shader reads a texture through a **handle**: a name for a descriptor the pipeline
binds, not a value with storage. Like a vector, a handle type is a **form** rather
than a fixed list:

```
[i|u] ("sampler" | "texture" | "image") <dim> ["ms"] ["array"] ["shadow"]
"sampler"
```

with `<dim>` one of `1d`, `2d`, `3d`, `cube`, `rect`, `buffer`, `subpass`. The
leading `i` / `u` picks the scalar texels are sampled as (`f32` by default); the
suffixes read in GLSL's own order, so `isampler2DMSArray` is `isampler2dmsarray`.

| spelling | what it is |
|---|---|
| `sampler2d` | a combined sampled image — one descriptor |
| `texture2d` | an image with no sampler, bound separately |
| `sampler` | a sampler with no image, bound separately |
| `isampler2d` / `usampler2d` | integer-sampled images |
| `sampler2darray` | array-layer image |
| `sampler3d`, `sampler1d`, `samplercube`, `samplercubearray` | the other dimensionalities |

Handles are **declared with `#[sampler(set, binding)]`** and nothing else — see
[decorators.md](decorators.md) for how one is bound and sampled. A handle cannot
sit behind a pointer, inside an array, or in a local binding: it names a
descriptor rather than an object with an address, and SPIR-V forbids storing an
image, sampler or sampled-image object at all.

A handle spelling is recognized only in **type position**, and reserves the name
against a `rec` / `uni` / `def`, on exactly the terms a vector spelling does.

Several well-formed spellings are **read and refused by name**, so they cannot be
half-supported and cannot be claimed later by an unrelated declaration. Each error
names the member it refused:

- `...shadow` — depth-comparison sampling, which needs `OpImageSampleDref*` and a
  reference value.
- `...ms` — a multisampled image is fetched per sample, not sampled.
- `rect`, `buffer`, `subpass` — dimensionalities needing further capabilities and
  a different read path.
- `image...` — a read-write storage image, which carries an image format and is
  read and written rather than sampled.

`$size_of` a handle is the target's pointer size: it is a name for a resource,
and a pointer is the shape every target already has for that.

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

**Constant indices are bounds-checked at compile time.** `N` is part of the
type, so an index the compiler can fold must land in `[0, N)`:

```mach
var xs: [4]i32;
val a: i32 = xs[3];             # ok
val b: i32 = xs[4];             # error: index 4 is out of bounds for `[4]i32` of length 4
val c: i32 = xs[-1];            # error: index -1 is out of bounds ...
```

The rule is keyed on the length the type carries, not on how the array was
spelled, so a `def` alias, an array field of a generic instance, an array nested
in a record or in another array, and a `^`-qualified array are all checked the
same way. It is exactly the length `$length_of` reports
([comptime-intrinsics.md](comptime-intrinsics.md)), and a vector's lane count
takes the identical rule.

Only a **constant** index is checked. A runtime index is not, and a pointer is
not indexed against any length at all — `*T` carries none.

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
