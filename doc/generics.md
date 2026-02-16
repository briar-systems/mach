# Generics

Mach supports monomorphized generics for records, unions, and functions. Type parameters are specified in square brackets and must be provided explicitly at every use site.


## Generic Records

```mach
rec Pair[T, U] {
    first: T;
    second: U;
}

val p: Pair[i32, str] = Pair[i32, str]{first: 42, second: "hello"};
```


## Generic Unions

```mach
uni Option[T] {
    some: T;
    none: u8;
}
```


## Generic Functions

```mach
fun identity[T](x: T) T {
    ret x;
}

val n: i32 = identity[i32](42);
```

Multiple type parameters:

```mach
fun make_pair[T, U](a: T, b: U) Pair[T, U] {
    ret Pair[T, U]{first: a, second: b};
}
```


## Monomorphization

The compiler generates a separate, specialized copy of each generic type or function for every unique combination of type arguments used in the program. There is no runtime polymorphism or type erasure.

`Pair[i32, str]` and `Pair[f64, f64]` are distinct types with independent memory layouts.


## Limitations

- No type inference: type arguments must always be specified explicitly
- No type constraints or bounds
- No variance
- No higher-kinded types
- Cross-module generic instantiation requires the compiler to re-process the generic definition for each new set of type arguments
