# Types

Mach is statically typed with no type inference. Every binding requires an explicit type annotation.


## Primitives

| Type    | Size    | Description |
|---------|---------|-------------|
| `u8`    | 1 byte  | Unsigned 8-bit integer |
| `u16`   | 2 bytes | Unsigned 16-bit integer |
| `u32`   | 4 bytes | Unsigned 32-bit integer |
| `u64`   | 8 bytes | Unsigned 64-bit integer |
| `i8`    | 1 byte  | Signed 8-bit integer |
| `i16`   | 2 bytes | Signed 16-bit integer |
| `i32`   | 4 bytes | Signed 32-bit integer |
| `i64`   | 8 bytes | Signed 64-bit integer |
| `f32`   | 4 bytes | 32-bit floating point |
| `f64`   | 8 bytes | 64-bit floating point |
| `ptr`   | ptr     | Untyped raw pointer (equivalent to C `void*`) |


## Pointer Types

- `*T` -- mutable pointer to T
- `&T` -- read-only pointer to T
- `**T` -- pointer to pointer to T

See [memory.md](memory.md) for pointer operations.


## Arrays

Fixed-size collections with the size specified at compile time:

```mach
val a: [3]i32;                       # uninitialized array of 3 i32
val b: [4]u8 = [4]u8{1, 2, 3, 4};   # initialized array
```

Array size is part of the type. Arrays are contiguous in memory with no hidden metadata.

Elements are accessed by index:

```mach
val x: i32 = b[0];   # first element
```

Pointers to array elements also support indexing:

```mach
val p: &u8 = ?b[0];
val y: u8  = p[2];   # third element
```


## Records

Records group named fields into a single type:

```mach
rec Point {
    x: f64;
    y: f64;
}

val p: Point = Point{x: 1.0, y: 2.0};
val x: f64 = p.x;
```

Records can be nested and used as field types in other records.


## Unions

Unions hold one of several named fields, sharing the same memory. The size of a union equals its largest field:

```mach
uni Value {
    int_val:   i64;
    float_val: f64;
    ptr_val:   ptr;
}
```

Only one field is valid at a time. There is no built-in tag -- tracking which field is active is the programmer's responsibility.


## Anonymous Records and Unions

Records and unions can be defined inline without a name:

```mach
rec Container {
    id: u32;
    data: uni {
        int_data:   i32;
        float_data: f64;
    };
}
```


## Type Aliases

The `def` keyword creates a type alias:

```mach
def Byte: u8;
def Offset: i32;
```

Aliases are interchangeable with their underlying type. Two aliases with the same underlying type are also interchangeable:

```mach
def Age = u8;
def Level = u8;
val a: Age = 30;
val l: Level = a;   # allowed, both are u8
```


## Generics

Records, unions, and functions can be parameterized with type parameters in square brackets:

```mach
rec Pair[T, U] {
    first: T;
    second: U;
}

fun swap[T](a: T, b: T) Pair[T, T] {
    ret Pair[T, T]{first: b, second: a};
}
```

Type parameters must be specified explicitly at every use site:

```mach
val p: Pair[i32, f64] = Pair[i32, f64]{first: 42, second: 3.14};
val s: Pair[i32, i32] = swap[i32](1, 2);
```

Generics are monomorphized: the compiler generates a separate copy of the type or function for each unique combination of type arguments. There are no type constraints, variance, or higher-kinded types.

See [generics.md](generics.md) for more detail.


## Methods

Functions can be associated with a record, union, or aliased type using a receiver parameter:

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

Methods are called with dot notation:

```mach
var p: Point = Point{x: 0.0, y: 0.0};
p.move(5.0, 10.0);
```

The receiver can be by value or by pointer. The compiler automatically takes the address or dereferences as needed to match the receiver type.


## Type Casting

The `::` operator casts a value to a different type. For same-sized types, this is a bit reinterpretation. For different-sized integer types, the compiler generates appropriate widening (zero-extend or sign-extend) or truncation instructions:

```mach
val x: f32 = 3.14;
val y: i32 = x::i32;   # reinterpret f32 bits as i32 (NOT a numeric conversion)
```

Reinterpreting between same-sized types:

```mach
rec Color { r: u8; g: u8; b: u8; a: u8; }

val c: Color = Color{r: 255, g: 0, b: 128, a: 255};
val packed: u32 = c::u32;
```

Widening and truncation between integer types:

```mach
val a: u8 = 42;
val b: u64 = a::u64;   # zero-extend u8 to u64
val c: i16 = -5;
val d: i64 = c::i64;   # sign-extend i16 to i64
val e: i64 = 1000;
val f: u8  = e::u8;    # truncate i64 to u8
```


## Literal Coercion

Integer and float literals are coerced to the declared type at the point of declaration:

```mach
val x: u8 = 42;       # 42 coerced to u8
val y: f32 = 3.14;    # 3.14 coerced to f32
```

Coercion only happens for literals. Variables of different types require an explicit cast:

```mach
val a: u8 = 42;
val b: u16 = a::u16;   # explicit cast required (u8 to u16)
```
