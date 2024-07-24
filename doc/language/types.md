- [Types](#types)
  - [Function Type](#function-type)
  - [Struct Type](#struct-type)
  - [Union Type](#union-type)
  - [Array Type](#array-type)
  - [Pointer Type](#pointer-type)
    - [Pointer Arithmetic](#pointer-arithmetic)
    - [Pointer Dereferencing](#pointer-dereferencing)
    - [Pointer Safety](#pointer-safety)
    - [Pointer Lifetime](#pointer-lifetime)
    - [Pointer Nullability](#pointer-nullability)
  - [Reference Type](#reference-type)
- [Type Casting](#type-casting)

# Types

Mach as an intentionally minimal set of types built in to the base version of the language.
Note that this statement does not include types defined in the standard library, of which there are many.

The Mach compiler understands these base types out of the box:

| type                       | description                  |
| -------------------------- | ---------------------------- |
| `i8`                       | signed 8-bit integer         |
| `i16`                      | signed 16-bit integer        |
| `i32`                      | signed 32-bit integer        |
| `i64`                      | signed 64-bit integer        |
| `u8`                       | unsigned 8-bit integer       |
| `u16`                      | unsigned 16-bit integer      |
| `u32`                      | unsigned 32-bit integer      |
| `u64`                      | unsigned 64-bit integer      |
| `f32`                      | 32-bit floating point number |
| `f64`                      | 64-bit floating point number |
| [function](#function-type) | function signature           |
| [struct](#struct-type)     | user-defined struct          |
| [union](#union-type)       | user-defined union           |
| [array](#array-type)       | fixed-size array of elements |

Here are a few additional types that the compiler recognizes.
These are noted here seperately as they can be assembled from the base types:

| type                         | description          | representation      |
| ---------------------------- | -------------------- | ------------------- |
| `char`                       | character            | `u8`                |
| `string`                     | string               | array of `char`     |
| [`ptr`](#pointer-type)       | pointer to a value   | system pointer size |
| [reference](#reference-type) | reference to a value | `ptr`               |


## Function Type

The function type represents the signature of a function.
It is a special type used throughout multiple compiler stages to represent functions.

The only two userland ways to define a custom function type are as follows:

Using the `fun` keyword:
```mach
fun foo(bar: u32, baz: u32): u32 { ... }
```

Using the `def` keyword. Please see [the section on the `def` keyword](doc/language/keywords.md#def) for more information.


## Struct Type

The struct type is a user-defined type that can contain multiple fields.
Structs are used to group related data together.

Structs are defined using the `str` keyword:
```mach
str foo: {
    bar: u32,
    baz: u32,
}
```


## Union Type

The union type is a user-defined type that can contain multiple fields.
Unions are used to group related data together.

Unions are defined using the `uni` keyword:
```mach
uni foo: {
    bar: u32,
    baz: f32,
}
```

Unions are similar to structs, but they only store one value at a time.
This means that the size of a union is equal to the size of its largest field.

Unions are useful when you need to store different types of data in the same memory location.
For example, you might use a union to represent a value that can be either an integer or a floating-point number.


## Array Type

The array type is a fixed-size array of elements.
Arrays are used to store multiple values of the same type.

Arrays are defined using brackets before the type:
```mach
var array: [2]u32;
```

The above example defines an array of two 32-bit unsigned integers.
Array size must be known at compile time, and the size must be a positive integer.

Note that the compiler can calculate the size argument from variables that are known at compile time:
```mach
val foo:    u32 = 2;
val length: u32 = 2;
val width:  u32 = foo * 2;

var array: [length * width]u32;
```


## Pointer Type

Pointer arithmetic is not possible using references in Mach, however, the `ptr` keyword can be used to create a traditional pointer to a value in memory.

Pointers are a standalone type in Mach and are not used in the same way as references.
They can, however, be created using similar methods as references are technically wrappers around `ptr`:
```mach
var foo: u32 = 0;
var bar: ptr = ?foo; // untyped pointer
```

The above example creates a pointer to a 32-bit unsigned integer.

Pointers are typically used to interact with C code or to perform low-level operations.
They are not recommended for general use in Mach code.


### Pointer Arithmetic

Arithmetic operations can be performed on the `ptr` type using standard arithmetic operators:
```mach
var foo: u32 = 0;
var bar: ptr = ?foo;

bar = bar + 1;
```

In the above example, the value of `bar` is incremented by one.


### Pointer Dereferencing

Pointers can be dereferenced using the dereference operator:
```mach
var foo: u32 = 0;
var bar: ptr = ?foo;

var baz: u32 = @bar;
```

In the above example, the value of `bar` is dereferenced and assigned to `baz`.


### Pointer Safety

Pointers are not type-safe in Mach.

Pointers can be used to access any memory address, and the compiler will not prevent you from doing so.
This can lead to undefined behavior, crashes, and security vulnerabilities.

Pointers should be used with caution and only when necessary.


### Pointer Lifetime

Pointers are not garbage collected in Mach.

Pointers must be manually managed by the programmer.
This means that the programmer must ensure that the memory pointed to by the pointer is not freed before the pointer is dereferenced.


### Pointer Nullability

Similar to the behaviour of languages like C, pointers in Mach are considered null if their value is `0`.

Attempting to dereference a null pointer will result in a runtime error.
```mach
var foo: ptr = 0;
var bar: u32 = @foo; // runtime error
```


## Reference Type

The reference type is a special type that is used to represent a reference to a value in memory.
Technically, the reference type is a pointer to a value, but it is not a pointer in the traditional sense as operations are not performed on the pointer itself in code.

The key differences between a reference and a `ptr` are:
- References preserve type information and are type-safe
- References cannot be used to perform pointer arithmetic

References are used to pass values by reference to functions and to create mutable references to values.

References are defined using the hash symbol as a type prefix, usually in conjunction with a reference operator:
```mach
var foo: u32 = 0;
var bar: #u32 = ?foo;
```

The above example creates a mutable reference to a 32-bit unsigned integer.

References are typically used to pass values by reference to functions:
```mach
fun increment(foo: #u32) {
    foo = foo + 1;
}

fun main() {
    var bar: u32 = 0;
    increment(?bar);
}
```

In the above example, the `increment` function takes a mutable reference to a 32-bit unsigned integer and increments the value by one.
The `main` function creates a 32-bit unsigned integer and passes a reference to it to the `increment` function.
After the `increment` function is called, the value of `bar` is now 1.


# Type Casting

Mach does not support type casting outside of pointer dereferencing, which can be a very unsafe operation to perform.
See [pointer dereferencing](#pointer-dereferencing) and [pointer safety](#pointer-safety).
