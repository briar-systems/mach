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

Example:
```mach
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

| Symbol                 | Description                                               |
| ---------------------- | --------------------------------------------------------- |
| `$OS_LINUX`            | Enum value representing the Linux operating system        |
| `$OS_DARWIN`           | Enum value representing the macOS operating system        |
| `$OS_WINDOWS`          | Enum value representing the Windows operating system      |
| `$OS_BSD`              | Enum value representing the BSD operating system          |
| `$ARCH_X86_64`         | Enum value representing the x86_64 architecture           |
| `$ARCH_ARM64`          | Enum value representing the ARM64 architecture            |
| `$ARCH_ARM`            | Enum value representing the ARM architecture              |
| `$ARCH_RISCV64`        | Enum value representing the RISC-V architecture           |
| `$target.os`           | Enum value of the target operating system                 |
| `$target.arch`         | Enum value of the target architecture                     |
| `$target.pointer_size` | Size of a pointer on the target platform in bytes         |
| `$target.word_size`    | Size of a machine word on the target platform in bytes    |
| `$target.triple`       | Target triple string (e.g., "x86_64-pc-linux-gnu")        |
| `$build.debug`         | Integer exposing the `debug` config option (1 if enabled) |
| `$build.version`       | String exposing the `version` config option               |
| `$mach.version`        | String representing the Mach compiler version             |

> These symbols are subject to change as the Mach language and compiler evolve. They will be documented in future releases and will eventually be stable.

Example:
```mach
$if (target.os == OS_LINUX) {
    # Linux-specific code
}
or (target.os == OS_DARWIN) {
    # macOS-specific code
}
```

Note that at the moment, compiler constants like `$target.os` do not need the leading `$` when used in compile-time expressions. This behaviour is subject to change in future releases.


## Symbol Attributes

Mach allows setting or reading certain attributes on symbols at compile time using special `$<symbol>.<attribute> [= <value>]` syntax.

This is primarily used for specifying information about certain symbols, such as manual (internal) symbol names.

Different symbols will have different supported attributes. Currently, only the `symbol` attribute is supported for functions and global variables.

Most notably, the `symbol` attribute is used in most Mach programs to remove name mangling from the `main` function to provide a predictable entry point for the runtime included with the standard library:

```mach
$main.symbol = "main"
fun main(args: []str) i64 {
    ret 0;
}
```

> This system is highly incomplete and is only guaranteed to function with the `symbol` attribute on functions for now. Future releases will expand this system to support more attributes on all symbol types.
