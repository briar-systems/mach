# Arrays and Slices

This document describes contiguous sequence types in Mach: fixed-size arrays and runtime-sized slices. It covers type syntax, construction with literals, fields, indexing semantics, interactions with calls and casts, and best practices.

Related topics:
- [Types](types.md)
- [Literals](literals.md)
- [Expressions and Operators](expressions-and-operators.md)
- [Variables and Constants](variables.md)
- [Functions and Methods](functions-and-methods.md)
- [Records and Unions](records-and-unions.md)
- [Generics](generics.md)
- [Compile-time Features](compile-time.md)

## Overview

- Fixed-size array: `[N]T` holds exactly `N` elements of type `T` in contiguous memory.
- Slice (runtime-sized): `[]T` is a fat pointer to contiguous `T` elements with separate length metadata.
- Both arrays and slices support indexing with `[i]`.
- Slices expose `.data` (pointer to first element) and `.len` (length as `u64`).

## Type syntax

- Fixed-size array type:
  - `[N]T` — `N` is a compile-time integer length expression, `T` is any type
- Slice type:
  - `[]T` — canonical slice type

Examples (types in declarations and fields):
```
val a: [3]u8;
var b: []u8;
rec Row { pixels: [640]u8; }
rec Buffer { data: []u8; }
```

Multidimensional forms are composed by nesting:
```
val m: [3][4]u64;     # 3 rows, each a [4]u64
var grid: [][]u8;     # slice of slice
```

## Literals

Mach supports typed literals for both arrays and slices.

- Fixed array literal:
  - `[N]T{ e0, e1, ..., e(N-1) }`
  - The number of elements must be `N`, each convertible to `T`.

- Slice literal:
  - `[]T{ e0, e1, ..., ek }`
  - Length is determined by the number of elements.

Examples:
```
val a: [3]u8 = [3]u8{ 1, 2, 3 };
val s: []u8  = []u8{ 10, 20, 30, 40 };
```

Literals may appear wherever a value is expected (initializers, arguments, etc.). Use explicit type on the left or as the literal’s prefix to disambiguate element type.

Empty literals:
```
val z1: [0]u8 = [0]u8{ };
val z2: []u8  = []u8{ };
```

String literals interoperate with byte sequences:
- In a context expecting `[]u8`, a string literal provides both data and length to produce a slice value.
- In a context expecting `*u8`, a string literal supplies a pointer to its first byte.

```
ext "C:puts" puts: fun(*u8) i32;
puts("Hello, world!\n");   # string literal as *u8
fun use_bytes(b: []u8) { /* ... */ }
use_bytes("hi\0");           # string literal as []u8
```

Notes:
- Mach's string literals are NOT null-terminated; they carry explicit length. Use escape sequences for embedded nulls as needed.

See [Literals](literals.md) for escape sequences and details.

## Fields (slices)

Slices expose two fields via dot access:
- `data: *T` — pointer to the first element
- `len: u64` — number of elements

Examples:
```
var s: []u8 = []u8{ 10, 20, 30 };
var n: u64  = s.len;
var p: *u8  = s.data;
```

Notes:
- Fixed-size arrays do not expose `.data`/`.len` as fields; they are plain contiguous values whose length is known at compile time.

## Indexing

Use `seq[i]` for both arrays and slices.

- Fixed array `[N]T`:
  - Indexing computes an element address; the valid range is `0..N-1`.
  - The index expression typically must be integer-typed and within range.

- Slice `[]T`:
  - Indexing is bounds-checked at runtime against `0..len-1`.
  - Out-of-bounds indexing triggers a runtime failure.

Examples:

```
val a: [3]u8 = [3]u8{ 1, 2, 3 };
val x: u8 = a[0];

var s: []u8 = []u8{ 10, 20, 30, 40 };
var y: u8   = s[2];          # ok
# s[10];                     # runtime bounds failure
```

Assignment through indexing is allowed when the base is assignable (a `var`, a dereferenced pointer, a field of an assignable object, etc.):
```
var b: [3]u8 = [3]u8{ 1, 2, 3 };
b[0] = 7;

var t: []u8 = []u8{ 10, 20, 30 };
t[1] = 25;
```

## Address-of, dereference, and pointers

Combine address-of `?` and dereference `@` with arrays and slices as needed:

- First element pointer:
```
var a: [3]u8 = [3]u8{ 1, 2, 3 };
var p0: *u8 = ?a[0];
```

- Slice data pointer:
```
var s: []u8 = []u8{ 10, 20 };
var p: *u8  = s.data;
```

- Pointer to array or slice variables:
```
var a: [4]u64;
var pa: *[4]u64 = ?a;

var s: []u8 = []u8{ 1, 2, 3 };
var ps: *[]u8 = ?s;
```

Dereferencing follows `@ptr` rules (see [Expressions and Operators](expressions-and-operators.md)).

## Calls and parameter matching

Function calls support ergonomic use of slices with pointer parameters:

- When a parameter is of pointer type `*U` and an argument is a slice `[]T`, the call can pass the slice’s data pointer when compatible (e.g., `T` convertible to `U`’s base type).
- For variadic calls, a slice argument `[]T` similarly decays to its data pointer for the corresponding position when compatible.

Examples:
```
ext "C:memcpy" memcpy: fun(*u8, *u8, u64) *u8;

var dst: [4]u8 = [4]u8{ 0, 0, 0, 0 };
var src: []u8  = []u8{ 1, 2, 3, 4 };

# pass &dst[0] (pointer) and src.data (slice decays to pointer)
memcpy(?dst[0], src.data, src.len);

ext "C:printf" printf: fun(*u8, ...) i32;
printf("val=%d\n", 42);        # normal varargs
printf("s=%s\n", src.data);    # pass data pointer
```

Notes:
- Slices are values with two fields; passing a slice where a slice is expected transmits both fields.
- Passing a slice to a pointer parameter provides the `data` pointer; the callee does not receive the `len` unless explicitly passed.

## Casts (`::`)

Use `::` to convert between compatible array/slice and pointer forms when explicit conversion is required:

- Slice to pointer (data pointer):
```
var s: []u8 = []u8{ 10, 20, 30 };
var p: *u8  = (s) :: *u8;   # use s.data; equivalent to s.data in calls
```

- Pointer to element from array/slice indexing:
```
var a: [3]u8 = [3]u8{ 1, 2, 3 };
var p0: *u8  = ?a[0];
```

Numeric casts of indices or lengths use the same `::` operator (see [Expressions and Operators](expressions-and-operators.md)).

## Multidimensional arrays and slices

Nest the type constructors:

- Fixed-size:
```
val m: [2][3]u64 = [2][3]u64{
    [3]u64{ 1, 2, 3 },
    [3]u64{ 4, 5, 6 },
};
```

- Mixed slice-of-array or array-of-slice:
```
var rows: [][]u8;         # a slice of row-slices
val fixed: [2][]u8 = [2][]u8{
    []u8{ 1, 2 },
    []u8{ 3, 4, 5 },
};
```

Indexing applies one dimension at a time: `m[i][j]`.

## Size, alignment, and layout

Use compile-time intrinsics to query type information:

- `$size_of([N]T)` — total byte size of the array
- `$size_of([]T)` — size of the slice structure (fat pointer)
- `$align_of([N]T)` / `$align_of([]T)` — alignment of the type

Examples:
```
val sz_arr: u64 = $size_of([4]u64);
val sz_slc: u64 = $size_of([]u8);
```

See [Compile-time Features](compile-time.md) for these intrinsics.

## Common patterns

- Iteration by index:
```
fun sum(xs: []u64) u64 {
    var i: u64 = 0;
    var acc: u64 = 0;
    for (i < xs.len) {
        acc = acc + xs[i];
        i = i + 1;
    }
    ret acc;
}
```

- Slicing as views:
  - A slice can refer to data owned elsewhere (e.g., a fixed array, a global, or foreign memory).
  - Passing slices avoids copying large buffers.

- First element and length:
```
fun head_or_zero(xs: []u64) u64 {
    if (xs.len == 0) {
        ret 0;
    }
    or {
        ret xs[0];
    }
}
```

## Best practices

- Use slices for function parameters to avoid copying entire arrays and to carry a length alongside the data pointer.
- Pass `slice.data` explicitly to APIs that expect `*T` and separately pass `slice.len` where needed.
- Keep indexing within bounds; prefer explicit checks when logic depends on lengths.
- Favor clear element types—e.g., prefer `[]u8` for raw byte buffers and distinct typedefs (via `def`) for semantic clarity.

## Summary

- `[N]T` is a fixed-size, contiguous array known at compile time.
- `[]T` is a slice: a value carrying a data pointer and a `len` field.
- Both support indexing; slice indexing is bounds-checked at runtime.
- Use typed literals to construct arrays and slices; use `::` casts and methodical field access (`.data`, `.len`) to interoperate with pointers and foreign APIs.

Continue with:
- [Expressions and Operators](expressions-and-operators.md) for indexing, assignment, and casts
- [Literals](literals.md) for construction forms and string interoperability
- [Functions and Methods](functions-and-methods.md) for argument passing rules
