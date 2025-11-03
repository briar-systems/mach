# Types

This document describes the Mach type system: primitive numeric types, composite types, pointers, arrays and slices, function types, and type aliases. It focuses on the syntax and semantics you use in programs. For records and unions in depth, see [Records and Unions](records-and-unions.md). For generics, see [Generics](generics.md). For casts, see [Expressions and Operators](expressions-and-operators.md).

- All declarations use explicit types; there is no implicit type inference in `val`/`var` declarations.
- Type names can be qualified with a module alias: `alias.TypeName`.
- Generic types are instantiated with type arguments: `TypeName[Arg1, Arg2, ...]`.

## Overview of categories

- Primitive numeric types
  - Unsigned integers: `u8`, `u16`, `u32`, `u64`
  - Signed integers: `i8`, `i16`, `i32`, `i64`
  - Floating-point: `f16`, `f32`, `f64`
- Pointers
  - Typed pointers: `*T`
- Arrays and slices
  - Fixed-size arrays: `[N]T`
  - Slices (runtime-sized fat pointers): `[]T`
- Composite types
  - Records (struct-like): `rec { ... }` and named `rec Name { ... }`
  - Unions: `uni { ... }` and named `uni Name { ... }`
- Function types
  - `fun(T1, T2, ...) RetType` with optional variadic `...`
- Aliases
  - `def Name: Type;`

Mach uses `u8` as the boolean type in conditions. The null literal is `nil`.

---

## Primitive numeric types

- Unsigned integers: `u8`, `u16`, `u32`, `u64`
- Signed integers: `i8`, `i16`, `i32`, `i64`
- Floating-point: `f16`, `f32`, `f64`

Notes:
- Sizes are the numeric suffix (e.g., `u16` is 16 bits).
- Arithmetic, comparisons, shifts, bitwise operations apply per operator rules in [Expressions and Operators](expressions-and-operators.md).
- Use explicit casts with `::` to convert between numeric types when needed.

Examples:
```
val a: u32 = 10;
val b: i64 = a :: i64;
val x: f64 = 3.14;
```

### Booleans

Conditions in control flow use `u8` as the boolean type. Conventionally, zero is false and nonzero is true.

```
fun ok(flag: u8) {
    if (flag) { /* ... */ }
    or { /* ... */ }
}
```

### Character and string literals

Character literals (e.g., `'a'`) and string literals (e.g., `"hello"`) are literals; their exact types are resolved by context. See [Literals](literals.md).

---

## Pointers

A pointer to a type `T` is written `*T`.

- Address-of: `?expr` yields `*T` when `expr` has type `T` and is an lvalue.
- Dereference: `@expr` yields `T` when `expr` has type `*T`.
- The null literal `nil` can be used for pointer-like values.
- Pointers can be used as function parameters and return values.

Examples:
```
var x: u64 = 42;
var p: *u64 = ?x;
@p = @p + 1;   # write through pointer
```

Casting to or from pointer types uses `::`:
```
var p8: *u8 = p :: *u8;
```

For pointer usage within composite types and arrays, see [Records and Unions](records-and-unions.md) and [Arrays and Slices](arrays-and-slices.md).

---

## Arrays and slices

Mach distinguishes between fixed-size arrays `[N]T` and slices `[]T`.

- Fixed-size array `[N]T`:
  - Compile-time length `N`
  - Contiguous storage of `N` elements of type `T`
  - Indexed with `array[i]`

- Slice `[]T`:
  - Runtime-sized fat pointer to contiguous `T` elements
  - Carries both `.data` (pointer to first element) and `.len` (length) fields
  - Indexed with `slice[i]`
  - Useful for passing views without copying

Type syntax:
```
val a: [4]u32 = [4]u32{ 1, 2, 3, 4 };
var s: []u32;            # slice, may be set at runtime
```

Literal initialization:
```
val x: [3]u8 = [3]u8{ 10, 20, 30 };
val y: []u8  = []u8{ 1, 2, 3, 4, 5 };
```

Slice fields:
```
var len: u64 = y.len;
var ptr: *u8 = y.data;
```

Indexing:
```
val first: u8 = a[0];
var i: u64 = 0;
for (i < y.len) {
    # use y[i]
    i = i + 1;
}
```

Notes:
- Slices are first-class and carry bounds (`.len`). Indexing a slice is subject to bounds rules of the target.
- When passing a slice to a parameter of type `*T`, the data pointer is used. See [Functions and Methods](functions-and-methods.md).

For more on array and slice expressions and semantics, see [Arrays and Slices](arrays-and-slices.md).

---

## Composite types: records and unions

Records and unions are composite types.

- Record (struct-like) type literal: `rec { field: Type; ... }`
- Union type literal: `uni { field: Type; ... }`

Named forms are introduced as top-level declarations (see [Records and Unions](records-and-unions.md)), and can be referenced by name as types. Anonymous type literals can be used directly where a type is required.

Examples (type syntax):
```
val P: rec { x: f64; y: f64; } = rec { x: 0.0, y: 0.0 };

val U: uni { i: i32; f: f32; } = uni { i: 123 };
```

For construction, field access, and layout-related topics (e.g., `$offset_of(T, field)`), see [Records and Unions](records-and-unions.md) and [Compile-time Features](compile-time.md).

---

## Function types

Function types describe callable signatures. Syntax:

```
fun(T1, T2, ..., Tn) RetType
```

- Parameter list is comma-separated. Use `...` as the last parameter to indicate a variadic function type.
- Return type is optional; if omitted, the function type returns no value.

Examples:
```
val f: fun(u64, u64) u64;
val g: fun(*u8, ...) i32;   # variadic function type
```

Function types are used in:
- External declarations (`ext`) to bind foreign symbols
- Variables and fields to store function references
- Casts involving callable values

See [Functions and Methods](functions-and-methods.md) for declaring and calling functions and methods.

---

## Type aliases

A type alias creates a new name bound to an existing type:

```
def Index: u64;
def Bytes: []u8;
```

- Aliases can improve readability and express intent.
- Aliases can refer to primitive, pointer, array/slice, function, or composite types.
- Use the alias name anywhere a type is expected.

---

## Qualified type names and generics

Type names may be qualified with a module alias and may carry generic type arguments.

- Qualified names: `alias.TypeName`
- Generic instantiation: `TypeName[Arg1, Arg2, ...]`

Examples:
```
use net: mylib.network;
val s: net.SocketState;

rec Box[T] { value: T; }
val b: Box[u64];
```

See [Generics](generics.md) for details on declaring and using generic records, unions, and functions.

---

## Casting

Use `expr :: Type` to convert a value to a different type where supported:

```
val a: u32 = 10;
val b: i64 = a :: i64;

var p: *u8 = ?a :: *u8;  # example involving address-of + cast
```

Cast rules follow the language’s type conversion semantics (numeric conversions, pointer conversions, etc.). See [Expressions and Operators](expressions-and-operators.md).

---

## Null

`nil` is the null literal. It is commonly used with pointer-like types and in contexts that accept null values.

```
var p: *u8 = nil;
```

---

## Summary

- Choose among primitive numeric types (`u*`, `i*`, `f*`) for values and arithmetic.
- Use `*T`, `?expr`, and `@expr` for pointer types, address-of, and dereference.
- Model contiguous data with `[N]T` and pass views using `[]T` slices (with `.data` and `.len`).
- Compose data with records (`rec`) and unions (`uni`); employ type literals or named declarations.
- Describe callables with `fun(...) Ret` types; add `...` for variadics.
- Create readable names with `def Name: Type;`.
- Qualify types with module aliases and instantiate generics with `[TypeArgs]`.
- Perform explicit casts via `::` as needed.

Related topics:
- [Expressions and Operators](expressions-and-operators.md)
- [Arrays and Slices](arrays-and-slices.md)
- [Records and Unions](records-and-unions.md)
- [Functions and Methods](functions-and-methods.md)
- [Generics](generics.md)
- [Compile-time Features](compile-time.md)