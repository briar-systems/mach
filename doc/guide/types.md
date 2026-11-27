# Types

Every value has a written type, and nothing converts without a cast. Pointers, arrays and function types are built from a few built-in types.

## At a glance

| Family | Spelling |
|---|---|
| Truth value | `u1` |
| Unsigned integers | `u8` `u16` `u32` `u64` `u128` `u256` `u512` |
| Signed integers | `i8` `i16` `i32` `i64` `i128` `i256` `i512` |
| Floats | `f16` `f32` `f64` |
| Pointers | `*T` and the untyped `ptr` |
| Arrays | `[N]T` |
| SIMD vectors | `f32x4`, `u8x16`, any element, `x` and a lane count |
| Function types | `fun(i64, i64) i64` |
| Records, unions and tags | `rec`, `uni` and `tag`, on [their own page](records-unions.md) |
| Secret data | `^T`, under [Secrecy](secrecy.md) |

## Scalars

- Integers of different types never mix, not even to widen. Convert with a [cast](values.md#casts), as in `n::i64`.
- `u1` is one byte holding 0 or 1. Comparisons, `&&`, `||` and `!` produce it, `if` and `for` accept nothing else, and it has no arithmetic. `n::u1` asks whether `n` is non-zero.
- Floats are IEEE 754 on every target.

`bool` and `usize` are not built in. std declares them as aliases, imported like any name.

| Alias | Type | Module |
|---|---|---|
| `bool` | `u1`, with the values `true` and `false` | `std.types.bool` |
| `usize`, `isize` | Integers as wide as a pointer | `std.types.size` |
| `char` | `u8` | `std.types.char` |
| `str` | `*char` | `std.types.string` |

```mach
use std.types.bool.bool;
use std.types.bool.true;

val ready: bool = true;
```

## Pointers

```mach
fun bump() i64 {
    var x: i64  = 5;
    val p: *i64 = ?x;
    @p = @p + 1;
    ret x;
}
```

| Form | Meaning |
|---|---|
| `*T` | A pointer to a `T` |
| `ptr` | An address with no pointee type. It converts to and from any `*T`, and nothing is read through it |
| `nil` | The null address, for any pointer or function value |
| `?place` | The address of a binding, field or element |
| `@p` | The value `p` points at |
| `p.field` | A field through a pointer to a record |
| `p[i]` | The element `i` places past `p` |

A pointer carries no length and no `const`. There is no pointer arithmetic: a later element's address is `?p[i]`.

## Arrays

```mach
val a: [4]i64 = [4]i64{1, 2, 3, 4};
val b: [_]i64 = [_]i64{1, 2, 3};
```

- `[_]T` takes its length from the literal that initializes it, and is legal only there.
- Arrays are values. Assigning, passing or returning one copies every element. To share one, pass a `*[N]T`, or a pointer and a length.
- `N` is part of the type, so a constant index is checked at compile time. A runtime index is not.
- `a[start, n]` is `n` elements from `start` as an `[n]T`, where `n` is a constant.

## Strings

There is no string type. A string literal is a `*u8` to its bytes in read-only data, with a 0 byte after them.

```mach
val msg:  *u8   = "hello\n";
val code: [3]u8 = "abc";
val name: [_]u8 = "mach";
```

- A `*u8` has no length, and `==` compares addresses. `std.types.string` has `str_len`, `str_equals` and the rest.
- A literal that initializes a `[N]u8` is exactly its `N` bytes, with a terminator only when `\0` is written.

## SIMD vectors

A vector type is an element type, `x` and a lane count. A lane is an integer of 8 to 64 bits or a float, and there are at least 2. Every shape compiles on every target.

```mach
fun lanes() f32 {
    val v: f32x4 = f32x4{1.0, 2.0, 3.0, 4.0};
    val w: f32x4 = v * v + v;
    val m: u32x4 = v < w;
    ret w[3];
}
```

- A literal gives every lane.
- Operators work lane by lane on two vectors of one shape. A scalar never mixes in.
- A comparison gives an unsigned mask vector, each lane all ones or all zeros.
- `v[i]` is one lane, where `i` is a constant.
- `p[i, 4]::f32x4` loads four elements through a pointer.

## Function types

```mach
fun add(a: i64, b: i64) i64 {
    ret a + b;
}

val op: fun(i64, i64) i64 = add;
```

`fun(T1, T2) R` is the type of a function value, and `op(2, 3)` calls through it. See [Functions](functions.md).

## Aliases

```mach
def Age:   u32;
def Bytes: [16]u8;
```

`def` gives a type another name, at module level only. It makes no new type: an `Age` is a `u32`.

The full rules are in the reference for [types](../language/types.md) and [`def`](../language/def.md).
