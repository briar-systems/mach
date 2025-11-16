# Pointers and Memory Management

This document covers pointer types, memory operations, and memory management semantics in Mach.

- [Pointers and Memory Management](#pointers-and-memory-management)
  - [Overview](#overview)
  - [Pointer Types](#pointer-types)
  - [Creating Pointers](#creating-pointers)
  - [Dereferencing Pointers](#dereferencing-pointers)
  - [Null Pointers](#null-pointers)
  - [Pointer Arithmetic](#pointer-arithmetic)
  - [Memory Safety](#memory-safety)


## Overview

Mach provides low-level pointer operations for direct memory manipulation. Pointers in Mach are explicit and require manual management - there is no garbage collection or automatic memory management in the language itself.

The language provides two primary operators for working with pointers:
- `?` - address-of operator (creates a pointer to a value)
- `@` - dereference operator (accesses the value a pointer points to)


## Pointer Types

Pointer types are denoted with a `*` prefix followed by the pointed-to type:

```mach
val int_ptr:   *i32 = nil;  # pointer to i32
val float_ptr: *f64 = nil;  # pointer to f64
val ptr_ptr:   **u8 = nil;  # pointer to pointer to u8
```

The special `ptr` type represents an untyped pointer, similar to C's `void*`:

```mach
val raw_ptr: ptr = nil;  # untyped pointer
```

See the [Types](types.md#primitives) documentation for details on primitive types.


## Creating Pointers

The `?` operator takes the address of a variable, creating a pointer to it:

```mach
var x: i32  = 42;
val p: *i32 = ?x;  # p now points to x
```


## Dereferencing Pointers

The `@` operator dereferences a pointer, accessing the value it points to:

```mach
var x: i32  = 42;
var p: *i32 = ?x;

val value: i32 = @p;  # read the value (42)
@p = 100;             # write through the pointer (x is now 100)
```

Dereferencing a null pointer or an invalid pointer results in undefined behavior.


## Null Pointers

The `nil` keyword represents a null pointer value. It can be assigned to any pointer type:

```mach
val p1: *i32 = nil;
val p2: *u8  = nil;
val p3: ptr  = nil;
```

See the [Keywords](keywords.md) documentation for more details on `nil`.


## Pointer Arithmetic

Mach does not provide built-in pointer arithmetic operators. Pointer manipulation must be done through explicit casting and offset calculations when necessary.

For array element access, use array indexing syntax instead:

```mach
val arr:  [5]i32 = [5]i32{1, 2, 3, 4, 5};
val elem: i32    = arr[2]; # preferred over pointer arithmetic
```

See the [Types](types.md#arrays) documentation for details on arrays and indexing.


## Memory Safety

Mach does not enforce memory safety at compile time or runtime.
It is the programmer's responsibility to:

- Ensure pointers are valid before dereferencing
- Avoid use-after-free bugs
- Prevent buffer overflows
- Manage memory allocation and deallocation properly
- Avoid dangling pointers

Patterns such as Zig's allocator model or other stylizations can be employed to help manage memory safely in Mach programs, but this decision is ultimately left to the developer.

The [standard library](https://github.com/octalide/mach-std) provides some utilities for memory management. Here's a simple example of allocating and freeing memory using the standard library's utilities:

```mach
use          std.system.runtime;
use          std.types.string;
use          std.types.option;
use console: std.io.console;

# memory utilities are found in the `std.system.memory` module
use mem: std.system.memory;

$main.symbol = "main"
fun main(args: []str) i64 {
    # allocate memory for 10 i32 values
    var opt_alloc: Option[*i32] = mem.allocate[i32](10);
    if (opt_alloc.is_none()) {
        console.print("Memory allocation failed\n");
        ret 1;
    }

    var p: *i32 = opt_alloc.unwrap();
    # `p` now points to the first byte of the allocated memory

    console.print("Memory allocation succeeded! Location: %p\n", p);

    # use the allocated memory
    var i: i32 = 0;
    for (i < 10) {
        @(p + i) = i * 10;
        i = i + 1;
    }

    # print the values
    i = 0;
    for (i < 10) {
        val value: i32 = @(p + i);
        console.print("Value at index %u: %u\n", i, value);
        i = i + 1;
    }

    # free the allocated memory
    mem.deallocate[i32](p, 10);

    ret 0;
}
```
