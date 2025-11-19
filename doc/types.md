# Types

This section describes Mach's type system.

- [Types](#types)
  - [Primitives](#primitives)
  - [Type Aliases](#type-aliases)
  - [Compound Types](#compound-types)
    - [Records](#records)
    - [Unions](#unions)
  - [Methods](#methods)
  - [Anonymous `rec` and `uni`](#anonymous-rec-and-uni)
  - [Arrays](#arrays)
  - [String Primitive](#string-primitive)
  - [Generics](#generics)
  - [Type Casting](#type-casting)
    - [Inference](#inference)
    - [Coercion](#coercion)


## Primitives

Mach includes the following primitive types:

| Type  | Description                                                      |
| ----- | ---------------------------------------------------------------- |
| `u8`  | 8-bit unsigned integer                                           |
| `u16` | 16-bit unsigned integer                                          |
| `u32` | 32-bit unsigned integer                                          |
| `u64` | 64-bit unsigned integer                                          |
| `i8`  | 8-bit signed integer                                             |
| `i16` | 16-bit signed integer                                            |
| `i32` | 32-bit signed integer                                            |
| `i64` | 64-bit signed integer                                            |
| `f32` | 32-bit floating-point number                                     |
| `f64` | 64-bit floating-point number                                     |
| `ptr` | Untyped pointer (equivalent to C's `void*`)                      |
| `str` | String literal record `{ data: &u8, len: <pointer-sized-uint> }` |

## Type Aliases

Type aliases can be created using the `def` keyword.

Aliases provide a way to create more meaningful names for existing types, improving code readability.

Aliased types do NOT inherit any special properties, methods, or behaviors from their original types. They are simply alternative names.

See the [Keywords](keywords.md#def) documentation for details.


## Compound Types

Mach supports four primary compound types:
- [Records](#records): Used to group related data fields together.
- [Unions](#unions): Used to define a type that can hold one of several different types.
- [Arrays](#arrays): Fixed-size collections of elements of the same type.


### Records

Records are defined using the `rec` keyword. See the [`rec`](keywords.md#rec) documentation for syntax details.

Records contain named fields of any other type:

```mach
rec Point {
    x: f64;
    y: f64;
}
```

Records can have methods associated with them, defined using a Go-style method syntax. See the [Method Syntax](keywords.md#method-syntax) section for syntax details and the [Methods](#methods) section for more information.


### Unions

Unions are defined using the `uni` keyword. See the [`uni`](keywords.md#uni) documentation for syntax details.

Unions contain named fields of any other type, but only one field can hold a value at a time:

```mach
uni Value {
    int_value:   i32;
    float_value: f64;
}
```

The size of a union is determined by its largest variant. In the above example, `Value` would occupy enough space to hold a `f64`, since `float_value` is its largest field.

Union types can also have methods associated with them, defined using the same Go-style method syntax as records. See the [Method Syntax](keywords.md#method-syntax) section for syntax details and the [Methods](#methods) section for more information.


## Methods

Methods are functions associated with a record, union, or aliased type, allowing them to operate on instances of that type.
They are defined using the same `fun` keyword, but include a special first parameter that represents the instance the method is called on.

The instance itself can be specified as either a named value or a pointer to a named value.
The instance parameter is conventionally named `this`, but any valid identifier can be used.

```mach
rec Point {
    x: f32;
    y: f32;
}

fun (this: *Point) move(dx: f32, dy: f32) {
    this.x = this.x + dx;
    this.y = this.y + dy;
}
```

Methods may be called on instances of the type they are associated with using dot notation:

```mach
val p: Point = Point {
  x: 0.0,
  y: 0.0,
};
p.move(5.0, 10.0);
```

Accessing the fields of the instance within the method is done using the instance parameter (`this` in the example above).
This parameter behaves like any other parameter, so it can be named differently or passed by value instead of by pointer, depending on the desired semantics.

If the instance parameter is passed by value, modifications to its fields will not affect the original instance outside the method.

This behaviour is particularly similar to how methods work in Go, where the receiver can be either a value or a pointer.

The compiler automatically converts between value and pointer types when calling methods.
If a method has a pointer receiver but is called on a value, the address is taken automatically.
If a method has a value receiver but is called on a pointer, the pointer is dereferenced automatically.
This means `.` is used uniformly for both field access and method calls, regardless of whether the instance is a value or pointer.


## Anonymous `rec` and `uni`

Mach supports anonymous records and unions in certain contexts.
The most common application of this feature is in nested data structures, where a record or union is defined inline without a name:

```mach
rec Container {
    id: u32;
    data: uni {
        int_data: i32;
        float_data: f64;
    };
}
```


## Arrays

Arrays are fixed-size collections of elements of the same type. The size of an array is specified in square brackets (`[<n>]`) before the element type where `n` is a positive integer literal or a constant expression that evaluates to a positive integer.

```mach
val a: [3]u8;
```

Arrays have a fixed size that is determined at compile time and cannot be changed at runtime.

On the backend, arrays are represented as a contiguous block of memory containing the elements in sequence.

Arrays expose no intrinsic fields. They behave like POD aggregates with a fixed number of elements known at compile time. If you need runtime-sized views into memory, use a record type from the standard library such as `std.types.Slice[T]` or `std.types.String`.

## String Primitive

`str` is the only compiler known composite type.
It represents string literals embedded in the binary and is exposed as a simple record:

```mach
rec str {
    data: &u8; # readonly pointer to UTF-8 data start
    len:  u64; # pointer-sized unsigned integer (u32 on 32-bit targets)
}
```

The `len` field uses the pointer-sized unsigned integer for the current target (e.g., `u64` on 64-bit platforms).
All string literals have type `str`, are UTF-8 encoded, and live in read-only memory.
Access the data and length through the exposed fields just like any other record.
For owned or mutable strings, prefer the standard library's `std.types.String` type.

## Generics

Mach supports a limited form of generics for records, unions, and functions.
This comes in the form of monomorphized templating, where type parameters are specified in square brackets (`[ ]`) after the type or function name and are replaced with concrete types at compile time.

```mach
rec Pair[T, U] {
    first: T;
    second: U;
}

fun make_pair[T, U](first: T, second: U) Pair[T, U] {
    return Pair[T, U] {
        first:  first,
        second: second,
    };
}
```

In the above example, both `T` and `U` are type parameters that can be replaced with any concrete types when creating instances of `Pair` or calling `make_pair`. They can both be used in any context that requires a type.

When using generic types or functions, the type parameters must be specified explicitly:

```mach
val p: Pair[i32, f64] = make_pair[i32, f64](42, 3.14);
```

Mach does not support advanced generic features such as type constraints, variance, or higher-kinded types.

## Type Casting

Mach requires explicit type casting between different types using the `::` operator:

```mach
val a: i32 = 42;
val b: i64 = a::i64;
```

This cast operator is extremely literal and does not perform any implicit conversions or coercions. This is important to keep in mind when working with different types, as you must ensure that the cast is valid and makes sense in the context of your program and the underlying data representation.

For example, this example will produce unusable results, as the bit patterns of `f32` and `i32` are not directly compatible:

```mach
val x: f32 = 3.14;
val y: i32 = x::i32; # This is NOT a valid conversion of the underlying bit pattern
```

The above example WILL compile and execute successfully, but the resulting value of `y` will not represent the integer equivalent of `3.14`. Instead, it will represent the raw bit pattern of the `f32` value interpreted as an `i32`, which is likely not what you want.

This cast operator is extremely powerful, but it comes with the responsibility of ensuring that the conversions you perform are valid and meaningful.

The compiler will warn you if you attempt to cast between incompatible types. Incompatible types are those that do not have identical sizes. This means that the cast operator DOES allow for casting between types of the same size but different representations. Here is a special example that demonstrates the power of such a system:

```mach
rec Color {
    r: u8;
    g: u8;
    b: u8;
    a: u8;
}

fun convert_color_to_u32(color: Color) u32 {
    ret color::u32;
}

fun convert_u32_to_color(value: u32) Color {
    ret value::Color;
}
```

### Inference

Mach does not support type inference under any circumstances.


### Coercion

Mach only supports type coercion in the context of literal declarations themselves.

```mach
val x:  u8 = 42;    # `42` is coerced to `u8`
val y:  u8 = x % 2; # `2` and the result of `x % 2` are coerced to `u8`
```

For example:
```mach
rec Byte {
    value: u8;
}

val b: Byte = Byte{ value: 255 }; # `255` is coerced to `u8`
val n: u8   = b::u8; # `b` requires an explicit cast to `u8` even though they have the same underlying data representation
```

The only case in which automatic coercion is allowed is when two aliased with equal underlying types are involved. These can be used interchangeably without explicit casting.

```mach
def Age:    u8;
def Height: u8;

val my_age:    Age    = 30;     # `30` is coerced to `u8`, which is the underlying type of `Age`
val my_height: Height = my_age; # no explicit cast needed, both have underlying type `u8`
```
