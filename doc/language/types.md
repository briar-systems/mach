- [Types](#types)
  - [Primitives](#primitives)
  - [`void`](#void)
  - [Functions](#functions)
  - [Structs](#structs)
  - [Unions](#unions)

# Types

Mach as an intentionally minimal set of types built in to the base version of the language.
Note that this statement does not include types defined in the standard library, of which there are many.

## Primitives

The Mach compiler understands these base types out of the box:

| type                       | description                  |
| -------------------------- | ---------------------------- |
| `u8`                       | unsigned 8-bit integer       |
| `u16`                      | unsigned 16-bit integer      |
| `u32`                      | unsigned 32-bit integer      |
| `u64`                      | unsigned 64-bit integer      |
| `i8`                       | signed 8-bit integer         |
| `i16`                      | signed 16-bit integer        |
| `i32`                      | signed 32-bit integer        |
| `i64`                      | signed 64-bit integer        |
| `f32`                      | 32-bit floating point number |
| `f64`                      | 64-bit floating point number |
| `ptr`                      | untyped pointer              |


## `void`

Mach supports the use of the `void` keyword only for semantic clarity as a return type for a function that does not return a value.
It is not available for use in any other context.


## Functions

Functions may be used as values in mach and as such have their own type definition.

Using the [`def`](doc/language/keywords.md#def) keyword allows manual creation of a function signature:
```mach
def foo: fun(u32, u32) u32
```

If a function signature is used as a type, a function with a matching signature may itself be provided in that context:
```mach
def foo: fun(u32, u32) u32

fun bar(f: foo) u32 {
    ret f(1, 2)
}

fun add(a: u32, b: u32) u32 {
    ret a + b
}

fun main(): void {
    bar(add)
}
```


## Structs

The struct type is a user-defined type that can contain multiple fields.

Structs are defined using the `str` keyword:
```mach
str foo {
    bar: u32
    baz: u32
}
```


## Unions

The union type is a user-defined type containing multiple fields that use the same memory space.

Unions are defined using the `uni` keyword:
```mach
uni foo {
    bar: u32
    baz: f32
}
```
