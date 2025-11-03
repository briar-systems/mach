# Interoperability

This document describes how to declare and use external symbols with `ext`, including calling conventions, symbol naming, function type signatures (with variadic support), and external data bindings.

Related topics:
- [Types](types.md)
- [Functions and Methods](functions-and-methods.md)
- [Pointers and Memory](pointers-and-memory.md)
- [Literals](literals.md)
- [Compile-time Features](compile-time.md)

## Overview

External declarations bind Mach names to symbols provided by foreign code (such as C libraries). An `ext` declaration specifies:
- An optional calling convention and/or explicit foreign symbol name
- A Mach identifier (the name you use in Mach code)
- A type, commonly a function type, but any type is allowed (for data externs)

External items have no bodies in Mach; you supply their type and link against implementations in external objects or libraries.

## Syntax

General form:
```/dev/null/interop.mach#L1-1
ext ["Convention[:symbol]"] Name: Type;
```

- The string literal is optional:
  - If present and of the form `"Convention"`, it sets the calling convention (e.g., `"C"`).
  - If present and of the form `"Convention:symbol"`, it also sets the foreign symbol name to `symbol`.
- If the string is omitted, the default calling convention is `"C"`.
- If `symbol` is omitted in the string, the foreign symbol name defaults to `Name`.
- The declaration ends with `;`.
- You may prepend `pub` to export the binding from the current module.

Examples:
```/dev/null/interop.mach#L1-12
# Function externs
ext "C"         puts:   fun(*u8) i32;         # convention only
ext "C:puts"    printc: fun(*u8) i32;         # convention + explicit symbol
ext             abort:  fun() -> ;            # default "C", no return value

# Variadic function extern
ext "C:printf"  printf: fun(*u8, ...) i32;    # format string + varargs

# Data externs (see “External data”)
ext "C:errno"   errno:  *i32;                 # pointer to an external int
```

Notes
- The convention string is a literal; `"C"` is the default and commonly used. Other values are target-dependent.
- `Name` is the Mach identifier bound in the current module; `symbol` is the foreign symbol as seen by the linker/loader.

## Function types for externs

The right-hand side of an `ext` declaration is a type. For callable externs, use a function type:

```/dev/null/interop.mach#L1-10
# fun(ParamTypes...) ReturnType
ext "C" atan2: fun(f64, f64) f64;

# No-return functions can omit the return type
ext "C" abort: fun();

# Variadic functions place ... as the last parameter
ext "C:printf" printf: fun(*u8, ...) i32;
```

Calling conventions at a glance
- Call sites in Mach use normal function call syntax.
- Arguments are evaluated left-to-right.
- When a parameter expects a pointer, passing a slice `[]T` uses its data pointer as appropriate; pass the length separately if needed.
- String literals can be used where `*u8` is expected; they provide a pointer to the underlying bytes.

Examples:
```/dev/null/interop.mach#L1-22
ext "C:puts"   puts:   fun(*u8) i32;
ext "C:printf" printf: fun(*u8, ...) i32;

fun demo() {
    puts("hello\n");

    # print an integer
    printf("n=%d\n", 42);

    # slices and pointers
    val bytes: []u8 = []u8{ 'O', 'K', 0 };
    puts(bytes.data);        # pass data pointer
}
```

See [Functions and Methods](functions-and-methods.md) and [Expressions and Operators](expressions-and-operators.md) for call semantics and argument conversions.

## External data

You can bind external data using any type. In practice, foreign globals are often exposed as pointers.

```/dev/null/interop.mach#L1-14
# External integer pointer (e.g., C's errno-like global)
ext "C:errno" errno: *i32;

fun clear_errno() {
    @errno = 0;      # write through the pointer
}

fun get_errno() i32 {
    ret @errno;      # read the current value
}
```

Recommendations
- Prefer pointer types (`*T`) for mutable external data:
  - Read: `val v: T = @ptr;`
  - Write: `@ptr = value;`
- If a foreign API exposes a constant buffer or structure, model it with a pointer or slice type that best matches usage on the foreign side.

## Symbol names and visibility

Choosing the foreign symbol
- Without a string literal, the foreign symbol name defaults to the Mach identifier.
- With a string literal `"Convention:symbol"`, the foreign symbol name is exactly `symbol`.

Exporting to other Mach modules
- Use `pub` to make an external binding visible to importers:
```/dev/null/interop.mach#L1-4
pub ext "C:puts" puts: fun(*u8) i32;
```

Overriding emitted names (for Mach-defined functions)
- For Mach functions you define (not `ext`), you can override the emitted symbol name via a compile-time attribute:
```/dev/null/interop.mach#L1-6
$main.symbol = "main";

fun main() i64 {
    ret 0;
}
```
See [Compile-time Features](compile-time.md) for symbol attributes.

## End-to-end examples

Binding a subset of a C runtime:
```/dev/null/interop.mach#L1-18
# I/O
ext "C:puts"   puts:   fun(*u8) i32;
ext "C:printf" printf: fun(*u8, ...) i32;

# Memory
ext "C:memcpy" memcpy: fun(*u8, *u8, u64) *u8;
ext "C:memset" memset: fun(*u8, i32, u64) *u8;

fun demo() {
    printf("Hello %s!\n", "world");

    var buf: []u8 = []u8{ 'A', 'B', 'C', 0 };
    memset(buf.data, 0, buf.len);
    puts(buf.data);  # prints an empty line (buffer now zeroed)
}
```

Working with external globals:
```/dev/null/interop.mach#L1-18
# Example global pointer
ext "C:errno" errno: *i32;

fun read_and_reset_errno() i32 {
    val v: i32 = @errno;
    @errno = 0;
    ret v;
}

# Example constant data (read-only pointer)
ext "C:program_name" program_name: *u8;

fun show_prog() {
    puts(program_name);
}
```

## Best practices

- Be explicit about calling conventions; use `"C"` unless you have a specific need for another.
- Provide an explicit symbol name in the declaration string when integrating with preexisting foreign APIs that use particular names.
- Prefer pointer and slice types that mirror the foreign API’s expectations; pass `slice.data` and `slice.len` as separate parameters where appropriate.
- For variadic externs, ensure the format string and arguments match the foreign function’s requirements.
- Keep external data bindings simple and explicit; favor `*T` types for globals so reads and writes are unambiguous.
