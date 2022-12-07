# Pointers in Mach

Mach handles pointers in a very no-nonsense way. There is no abstraction layer between the programmer and the pointer, and the programmer is responsible for managing the memory that the pointer points to. This is similar to C, but with a few semantic differences.

Note that while Mach does allow for unrestricted pointer access by default, Mach is designed in a way that makes the arduous task of memory management significantly easier than its low-level friends (looking at you, C).

## Pointer Types

A pointer type is declared by placing an asterisk (`*`) before the type that the pointer points to. For example, `*u32` is a pointer to an unsigned, 32-bit integer. Pointers can point to any type, including other pointers, meaning the syntax `**u32` is valid.

Additionally, the pointer operators described below can be chained to reference or dereference a pointer value as many times as necessary. For example, `@@x` is a valid expression that, if `x` is of type `**u32`, would return the `u32` value that `x` points to. An example of this is shown below:

```
val x:   i32 =  0   # x is a 32-bit, signed integer variable
val y:  *i32 = ?x   # y is a pointer to a 32-bit, signed integer variable
val z: **i32 = ?y   # z is a pointer to a pointer to a 32-bit, signed integer variable
val w:   i32 = @@z  # w is a 32-bit, signed integer variable (equal to x)
```

## Pointer Operators

Mach has two operators that are used to manipulate pointers: `@` and `?`. The `@` operator is used to dereference a pointer, and the `?` operator is used to get the address of a variable. It's that simple.

This example shows how to use the `@` and `?` operators:
```
val x:  i32 =  0    # x is a 32-bit, signed integer variable
val y: *i32 = ?x    # y is a pointer to a 32-bit, signed integer variable
val z:  i32 = @y    # z is a 32-bit, signed integer variable (equal to x)
```

Pointers can be declared using `var` or `val` depending on whether or not the stored pointer should be mutable or immutable. Note that the pointer cannot change type, and must be set using another pointer of the same type, just like other variables.
