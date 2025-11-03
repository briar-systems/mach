# Functions and Methods

This document specifies how to declare and use functions and methods in Mach, including parameters, return types, generics, variadics, and call semantics.

Related topics:
- [Types](types.md)
- [Expressions and Operators](expressions-and-operators.md)
- [Variables and Constants](variables.md)
- [Records and Unions](records-and-unions.md)
- [Arrays and Slices](arrays-and-slices.md)
- [Generics](generics.md)
- [Interoperability](interoperability.md)
- [Inline Assembly](inline-assembly.md)
- [Modules and Visibility](modules-and-visibility.md)

## Overview

- Functions are declared with `fun name(params) ReturnType { ... }`.
- Methods are functions with an explicit receiver: `fun (recv: Type) name(params) ReturnType { ... }`.
- Declarations can be generic: `fun name[T, U](...) ... { ... }`.
- Variadic functions use `...` as the final parameter: `fun printf(fmt: *u8, ...) i32;`.
- A function may omit a return type to indicate “no return value”.
- Statements inside function bodies follow the standard statement rules (`;` terminators, blocks with `{}`).

Example:
```/dev/null/functions-and-methods.mach#L1-28
# a simple function
fun add(a: u64, b: u64) u64 {
    ret a + b;
}

# a procedure (no return value)
fun log_ok() {
    # implement logging here
    ret;
}

# generic function
fun id[T](x: T) T {
    ret x;
}
```

## Syntax

General forms:
- Function:
  - `fun Name [TypeParams]? ( ParamList ) ReturnType? Block`
- Method (receiver comes before the name):
  - `fun (recv: Type) Name [TypeParams]? ( ParamList ) ReturnType? Block`

Where:
- `TypeParams` are zero or more comma-separated type parameters in `[ ... ]`.
- `ParamList` is zero or more comma-separated `name: Type` entries, optionally ending with `...` for variadics.
- `ReturnType` is any type; it may be omitted (no value returned).
- `Block` is a brace-enclosed sequence of statements.

See [Generics](generics.md) for the generic parameter list, and [Types](types.md) for type syntax.

## Parameters

- Each parameter has a name and type: `name: Type`.
- Parameter names are required in function declarations.
- Parameters are evaluated left-to-right at call sites.
- The parameter list may be empty: `fun f() { ... }`.

Examples:
```/dev/null/functions-and-methods.mach#L1-22
fun distance(x0: f64, y0: f64, x1: f64, y1: f64) f64 {
    ret (x1 - x0) * (x1 - x0) + (y1 - y0) * (y1 - y0);
}

fun no_args() {
    ret;
}
```

Notes:
- Use pointer types (`*T`) to allow callee-side mutation of passed data; otherwise values are passed by value. See [Pointers and Memory](pointers-and-memory.md).
- Use slice types (`[]T`) to pass a view of contiguous memory with a length; the callee sees `.data` and `.len`. See [Arrays and Slices](arrays-and-slices.md).

## Return types and `ret`

- If a return type is present, use `ret expression;`.
- If no return type is present, use `ret;`.
- The expression must be type-compatible with the declared return type; use `::` to cast when needed (see [Expressions and Operators](expressions-and-operators.md)).

Examples:
```/dev/null/functions-and-methods.mach#L1-24
fun mul(a: u64, b: u64) u64 {
    ret a * b;
}

fun to_i64(x: u64) i64 {
    ret (x) :: i64;
}

fun proc_only() {
    # work, then return
    ret;
}
```

## Methods

A method is a function declared with an explicit receiver before the method name:
- `fun (recv: Type) name(params) ReturnType? { ... }`
- The receiver can be a value `Type` or a pointer `*Type`.

Call syntax:
- `value.method(args...)`

Automatic address-of/deref:
- If the receiver type is `*T` and you call on `T`, the address of the receiver is taken automatically.
- If the receiver type is `T` and you call on `*T`, the receiver is dereferenced automatically.

Examples:
```/dev/null/functions-and-methods.mach#L1-44
rec Counter {
    value: u64;
}

# pointer receiver: mutates the receiver
fun (c: *Counter) inc() {
    c.value = c.value + 1;
}

# value receiver: reads the receiver
fun (c: Counter) get() u64 {
    ret c.value;
}

fun demo_methods() {
    var c: Counter;
    c.value = 0;

    c.inc();             # auto &c to match *Counter receiver
    val v: u64 = c.get();
    ret;
}
```

Method dispatch:
- Method names are resolved based on the (possibly generic) receiver type.
- Methods coexist with free functions; qualify with a module alias if needed (see [Modules and Visibility](modules-and-visibility.md)).

## Generic functions and methods

Declare type parameters after the function or method name:
- `fun name[T, U](...) ... { ... }`
- Instantiate at call sites by providing type arguments before the argument list:
  - `name[u64](arg)`
  - For methods: `value.method[T](args...)`

Examples:
```/dev/null/functions-and-methods.mach#L1-40
# generic function
fun box[T](x: T) rec { value: T; } {
    ret rec { value: x };
}

# generic method on a generic record
rec Pair[T] {
    a: T;
    b: T;
}

fun (p: Pair[T]) first() T {
    ret p.a;
}

fun use_generics() {
    val p: Pair[u64] = Pair[u64]{ a: 1, b: 2 };
    val x: u64 = p.first();
    val bx     = box[u64](x);
}
```

See [Generics](generics.md) for details on declaring generic records/unions and passing type arguments.

## Variadic functions

- A variadic function places `...` as the last parameter in its parameter list.
- The function type likewise ends its parameter type list with `...`.
- Call sites pass additional arguments positionally after the fixed parameters.
- Use with foreign interop is common (e.g., C’s `printf`).

Examples:
```/dev/null/functions-and-methods.mach#L1-20
# variadic function type in an external declaration
ext "C:printf" printf: fun(*u8, ...) i32;

fun log_value(n: u64) {
    printf("n=%llu\n", n);
    ret;
}
```

Notes:
- A function’s parameter list may contain at most one `...`, and it must appear last.
- When passing a slice `[]T` to a parameter of pointer type in a variadic position, the slice decays to its `.data` pointer when appropriate. See [Arrays and Slices](arrays-and-slices.md) and [Expressions and Operators](expressions-and-operators.md).

## Calls

Call forms:
- Simple: `name(args...)`
- With type arguments: `name[TypeArgs](args...)`
- Method: `value.method(args...)`
- Method with type arguments: `value.method[TypeArgs](args...)`

Semantics:
- Arguments are evaluated left-to-right.
- Postfix operators bind tightly (see precedence in [Expressions and Operators](expressions-and-operators.md)).
- Slice-to-pointer decay:
  - When a parameter is of pointer type `*U` and the argument is a slice `[]T`, the call passes the slice’s `.data` pointer when compatible.
  - For variadic calls, the same decay applies to slice arguments in vararg positions.

Examples:
```/dev/null/functions-and-methods.mach#L1-42
fun add(a: u64, b: u64) u64 { ret a + b; }

fun use_calls() {
    val s: u64 = add(2, 3);

    # generic call
    fun id[T](x: T) T { ret x; }
    val x: u64 = id[u64](s);

    # slice to pointer decay in call
    ext "C:memset" memset: fun(*u8, i32, u64) *u8;
    var buf: []u8 = []u8{ 1, 2, 3, 4 };
    memset(buf.data, 0, buf.len);   # pass pointer + length

    ret;
}
```

## Function types

Function types describe callable signatures:
- Syntax: `fun(T1, T2, ...) RetType` (return type optional)
- Variadics in types: `fun(T1, ...) RetType`
- Used in `ext` declarations, variables, and fields.

Examples:
```/dev/null/functions-and-methods.mach#L1-22
val f: fun(u64, u64) u64;
val g: fun(*u8, ...) i32;

# external binding using a function type
ext "C:puts" puts: fun(*u8) i32;
```

See [Types](types.md) for type syntax and [Interoperability](interoperability.md) for external declarations.

## Visibility and modules

- Prepend `pub` to export a function or method from the current module.
- Import dependencies with `use` (aliased or unaliased). See [Modules and Visibility](modules-and-visibility.md).

Examples:
```/dev/null/functions-and-methods.mach#L1-24
use console: std.io.console;

pub fun greet(name: []u8) {
    console.print("hello, %s\n", name);
    ret;
}
```

## Inline assembly in functions

- Use `asm { ... }` inside functions to embed target-specific instructions. A trailing `;` is allowed after the block.
- See [Inline Assembly](inline-assembly.md).

Example:
```/dev/null/functions-and-methods.mach#L1-16
fun fence() {
    asm {
        # target-specific barrier
    };
    ret;
}
```

## Best practices

- Choose value or pointer receivers deliberately:
  - Use `*T` receivers to mutate the receiver or avoid copying large aggregates.
  - Use `T` receivers for read-only methods on small types.
- Prefer slice parameters (`[]T`) for contiguous buffers; pass `slice.data` to pointer-based APIs and include `slice.len` when required.
- Keep generic APIs minimal and well-constrained; pass explicit type arguments when inference is not possible.
- Use explicit casts `::` at return and call sites to make conversions obvious.

## Summary

- Declare functions with `fun`, methods with a receiver `(recv: Type)` before the name.
- Parameters are `name: Type` entries; end with `...` for variadics.
- Return type is optional; use `ret` to return.
- Generics appear in `[TypeParams]` after the name; call with `[TypeArgs]`.
- Calls evaluate arguments left-to-right; method calls apply auto address-of/deref to match the receiver type.
- Function types use `fun(...) Ret`; commonly appear in external bindings.