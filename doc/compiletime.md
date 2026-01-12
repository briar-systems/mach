# Compiletime Features

- [Compiletime Features](#compiletime-features)
  - [Overview](#overview)
  - [Type Introspection](#type-introspection)
  - [Conditional Compilation](#conditional-compilation)
  - [Compiler-provided Symbols](#compiler-provided-symbols)
  - [Symbol Attributes](#symbol-attributes)


## Overview

The compiletime system in Mach facilitates a two-way connection between the developer and the compiler.

It allows the developer to write code that is executed during compilation, enabling dynamic code generation, conditional compilation, and introspection of types and symbols.

This system is built around special compile-time expressions, directives, and intrinsics that provide powerful capabilities to manipulate and query the program's structure before it is run.


## Type Introspection

Compiletime type introspection works through a set of intrinsics that allow you to query properties of types at compile time.

Available type introspection intrinsics include:

| Intrinsic                     | Description                                                             |
| ----------------------------- | ----------------------------------------------------------------------- |
| `$size_of(<type>)`            | Evaluates to the size in bytes of the specified `<type>`                |
| `$align_of(<type>)`           | Evaluates to the alignment in bytes of the specified `<type>`           |
| `$offset_of(<type>, <field>)` | Evaluates to the byte offset of the specified `<field>` within `<type>` |
| `$type_of(<expr>)`            | Evaluates to a type value equivalent to the type of `<expr>`            |

Examples:

```mach
rec Foo {
    a: u32,
    b: u64,
}

var size:   u64 = $size_of(Foo);      # `size` will be 16
var align:  u64 = $align_of(Foo);     # `align` will be 8
var offset: u64 = $offset_of(Foo, b); # `offset` will be 8
var t: $type_of(size);                # `t` will be `u64`
```


## Conditional Compilation

Mach supports conditional compilation through the use of compile-time `if` statements. This allows you to include or exclude code based on compile-time conditions.

The example below pulls in `std.types.bool`, which defines the `bool`, `true`, and `false` symbols used for the flag.

Example:
```mach
use std.types.bool;

$if ($size_of(u64) == 8) {
    var is_64_bit: bool = true;
}
or {
    var is_64_bit: bool = false;
}
```

Practical uses for this system include primarily platform-specific code generation. Good examples of this in use exist in the [standard library](https://github.com/octalide/mach-std/tree/main/src/system) under the `src/system` directory, where different implementations are provided based on the target architecture and operating system.

Code that does not meet the compiletime conditions is completely omitted from the final compiled binary, allowing for efficient and tailored builds.

> NOTE: If a `use` statement is inside a compiletime conditional block that does not pass its condition, the import itself will not be processed. This means modules that are conditionally imported will not go through any form of lexing, parsing, semantic analysis, or code generation. Bugs in such modules (including lexical errors) may be transparent until the condition is met (such as building with a different target platform).


## Compiler-provided Symbols

The Mach compiler provides several built-in symbols that can be used for compile-time introspection and conditional compilation.

| Symbol                         | Description                                                                  |
| ------------------------------ | ---------------------------------------------------------------------------- |
| `$mach.build.target.os`        | Target operating system info (see `std.compiler.target.OSInfo`)             |
| `$mach.build.target.arch`      | Target architecture info (`std.compiler.target.ArchitectureInfo`)           |
| `$mach.build.target.abi`       | Target ABI descriptor (`std.compiler.target.ABIInfo`)                       |
| `$mach.build.target.vendor`    | Target vendor descriptor (`std.compiler.target.VendorInfo`)                 |
| `$mach.build.target.environment` | Target environment descriptor (`std.compiler.target.EnvironmentInfo`)      |
| `$mach.build.target.object_format` | Target object format descriptor (`std.compiler.target.ObjectFormatInfo`) |
| `$mach.build.target.endianness` | Target endianness descriptor (`std.compiler.target.EndiannessInfo`)        |
| `$mach.build.target.backend`   | Target backend descriptor (`std.compiler.target.BackendKindInfo`)           |
| `$mach.build.target.pointer_width` | Pointer width of the compilation target in bytes                          |
| `$mach.build.target.triple`    | Target triple string used for code generation                               |
| `$mach.build.debug`            | Non-zero when the current build is a debug build                            |
| `$mach.build.version`          | User-provided build version string                                           |
| `$mach.version`                | Compiler version string                                                      |
| `$mach.os`                     | Namespace containing all supported operating system descriptors              |
| `$mach.arch`                   | Namespace containing architecture descriptors                                |
| `$mach.abi`                    | Namespace containing ABI descriptors                                         |
| `$mach.vendor`                 | Namespace containing vendor descriptors                                      |
| `$mach.environment`            | Namespace containing environment descriptors                                 |
| `$mach.endianness`             | Namespace containing endianness descriptors                                  |
| `$mach.object_format`          | Namespace containing object-format descriptors                               |
| `$mach.backend`                | Namespace containing backend descriptors                                     |
| `$mach.feature`                | Namespace containing architecture feature descriptors                        |

All compile-time identifiers must be explicitly qualified with `$`. Even inside `$if` blocks you must prefix every access, for example:

```mach
$if ($mach.build.target.os.id == $mach.os.linux.id) {
    # linux-specific code
}
```

> These symbols are subject to change as the Mach language and compiler evolve. They will be documented in future releases and will eventually be stable.

Example:
```mach
$if ($mach.build.target.os.id == $mach.os.linux.id) {
    # Linux-specific code
}
or ($mach.build.target.os.id == $mach.os.darwin.id) {
    # macOS-specific code
}
```


## Symbol Attributes

Mach allows setting or reading certain attributes on symbols at compile time using special `$<symbol>.<attribute> [= <value>]` syntax.

This is primarily used for specifying information about certain symbols, such as manual (internal) symbol names.

Different symbols will have different supported attributes. Currently, only the `symbol` attribute is supported for functions and global variables.

Most notably, the `symbol` attribute is used in most Mach programs to remove name mangling from the `main` function to provide a predictable entry point for the runtime included with the standard library:

```mach
$main.symbol = "main"
fun main(argc: i64, argv: &&u8) i64 {
    ret 0;
}
```

> This system is highly incomplete and is only guaranteed to function with the `symbol` attribute on functions for now. Future releases will expand this system to support more attributes on all symbol types.
