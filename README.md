MACH
===

Mach is a statically-typed, compiled programming language designed to be simple, fast, and easy to use.
It is intended to be a systems programming language, but can be used for a wide variety of applications.

> Mach is still alpha quality. Expect breaking changes as the compiler and standard library iterate.

# Overview

- [MACH](#mach)
- [Overview](#overview)
  - [Philosophy](#philosophy)
  - [Key Features](#key-features)
- [Getting Started](#getting-started)
  - [Simple Examples](#simple-examples)
    - [Hello World](#hello-world)
    - [Fibonacci](#fibonacci)
    - [Factorial](#factorial)
- [Credit](#credit)
- [License](#license)


## Philosophy

Mach is designed with the following principles in mind:
- **Simplicity**: Mach is easy to learn and use.
- **Readability**: Mach is easy to read and understand.
- **Explicivity**: Mach is explicit and verbose. WYSIWYG, always. Computers are not magic. Your code should not promote this illusion.
- **Performance**: Mach is fast and efficient.
- **Predictability**: Mach is predictable and consistent. There are no hidden behaviors or side effects.

Mach is NOT designed to prioritize:
- **Features**: Batteries are not included.
- **Flexibility**: Mach is rigid and opinionated. It should not be flexible or allow for many ways to do the same thing.
- **Code Reduction**: Mach is explicit and verbose. More code is not bad code.
- **Safety**: Safety is the responsibility of the programmer and is often project-specific. Mach does not hold your hand or put you on a leash.


## Key Features

- No magic. No side effects. No bullshit. Code written in Mach follows the WYSIWYG principle down to the metal.
- Small, clearly defined, and well documented feature set.
- Familiar and easy to read syntax. You do not need to take a class to use mach.
- Barebones standard library with enough to do the basics, but not enough to overload the language.
- Compatibility with C at the ABI level.


# Getting Started

We encourage you to not even install Mach until you have read the [language documentation](doc/README.md). The docs are written more like a pamphlet then a bible, and assumes that you are familiar with basic programming concepts from other languages.

The reason for this is that Mach may not be for you. If the language does not include some features you hope to use, includes things you despise, or if you just don't like the syntax, then you should look elsewhere. If you read the documentation and have decided that you like the language, then you will have learned the basics and should be capable of diving in.

The [documentation](doc/README.md) includes instructions for how to [get started](doc/getting_started.md) with Mach.

If you are new to programming in general, then Mach may not be for you. There are lots of other languages that are better suited for beginners (mostly because of the level of documentation), and we encourage you to look into those instead. Some good "first" programming languages are:
- [Python](https://www.python.org/)
- [Lua](https://www.lua.org/)
- [Javascript](https://www.javascript.com/)


## Simple Examples

The following examples are provided to give a sense of the language's syntax and structure.


### Hello World

```mach
use          std.runtime;
use          std.types.string;
use console: std.io.console;

#@symbol("main")
fun main(args: []string): i64 {
    console.print("Hello, World!");
    ret 0;
}
```


### Fibonacci

```mach
use          std.runtime;
use          std.types.string;
use console: std.io.console;

fun fibr(n: i64): i64 {
    if (n < 2) {
        ret n;
    }

    ret fibr(n - 1) + fibr(n - 2);
}

#@symbol("main")
fun main(args: []string): i64 {
    var max: i64 = 10;
    console.print("%i", fibr(max));
    ret 0;
}
```


### Factorial

```mach
use          std.runtime;
use          std.types.string;
use console: std.io.console;

fun fact(n: i64): i64 {
    if (n == 0) {
        ret 1;
    }

    ret n * fact(n - 1);
}

#@symbol("main")
fun main(args: []string): i64 {
    var max: i64 = 10;
    console.print("%i", fact(max));
    ret 0;
}
```

# Credit

The inspiration for Mach comes from too many languages to count. Almost every language has problems that Mach attempts to elegantly resolve (most often by the process of reductive simplification).

Direct inspiration for the compiler, however, comes from a few more specific sources, the most notable of which are listed below:

- [Golang](https://golang.org/)
- [Vlang](https://vlang.org/)
- [Zig](https://ziglang.org/)
- [Rust](https://www.rust-lang.org/)

The original compiler would not have been written without the ability to reference the source code of these languages.

Mach, at its core, stands on the shoulders of countles giants that have contributed to the development of these languages either directly or by proxy. It is out of respect for their work that Mach will always be fully open source. Thank you all.

# License

Mach currently operates under the [Unlicense](https://unlicense.org/).

> Take it. Use it. Hate it. Break it. Fix it. Love it. It's yours.
