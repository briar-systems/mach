MACH
===

![License](https://img.shields.io/github/license/octalide/mach)
![Code Size](https://img.shields.io/github/languages/code-size/octalide/mach)
![Last Commit](https://img.shields.io/github/last-commit/octalide/mach)
![Issues](https://img.shields.io/github/issues/octalide/mach)

Mach is a statically-typed, compiled programming language designed to be simple, fast, verbose, and intuitive.

> Mach is still alpha quality. Expect breaking changes as the compiler and standard library iterate.

We have an official [Discord](https://discord.com/invite/dfWG9NhGj7)!

# Overview

- [MACH](#mach)
- [Overview](#overview)
  - [Core Philosophy](#core-philosophy)
- [Getting Started](#getting-started)
  - [Building Mach](#building-mach)
  - [Simple Examples](#simple-examples)
    - [Hello World](#hello-world)
    - [Fibonacci](#fibonacci)
    - [Factorial](#factorial)
- [Credit](#credit)
  - [Contributing](#contributing)
- [License](#license)


## Core Philosophy

Mach is designed with the following principles in mind:
- **Simplicity**: Mach is built to be easy to learn, read, write, and maintain.
- **Explicivity**: Mach is explicit and verbose. WYSIWYG, always. Computers are not magic. Your code should not promote this illusion.
- **Maintainability**: Mach's semantics and design principles prioritize long-term maintainability over short-term convenience.

Mach is NOT designed to prioritize:
- **Features**: Batteries are not included. Ever.
- **Flexibility**: Mach is rigid and opinionated. It should not be flexible or allow for many ways to do the same thing.
- **Code Reduction**: Mach is explicit and verbose. More code is not worse code.
- **Hand-holding**: Mach provides tools for safety (like read-only pointers and deferred cleanup), but it will not stop you from doing dangerous things if you explicitly ask to. Safety is a partnership between the language and the programmer.


# Getting Started

We encourage you to not even install Mach until you have read the [language documentation](doc/README.md). The docs are written more like a pamphlet than a bible, and assume that you are familiar with basic programming concepts from other languages.

The reason for this is that Mach may not be for you. If the language does not include some features you hope to use, includes things you despise, or if you just don't like the syntax, then you should look elsewhere. If you read the documentation and have decided that you like the language, then you will have learned the basics and should be capable of diving in.


## Building Mach

Before compiling the toolchain, follow the [getting started checklist](doc/getting-started.md) to ensure that your system is capable of building Mach from source.

Once everything is set up, building a usable mach compiler is as simple as:

```bash
git clone https://github.com/octalide/mach
cd mach
make cmach
```

> NOTE: The above command builds the bootstrap compiler, `cmach`, which is written in C. Mach's fully self-hosted compiler, `mach`, is currently under heavy development and is not yet functional.


## Simple Examples

The following examples are provided to give a sense of the language's syntax and structure.

> The examples are functional, but they expect the standard library to be included as a dependency during compilation. This is NOT default behaviour and must be explicitly specified to the compiler.
>
> For a fully working out-of-the-box example, please refer to the [Mach Sieve](https://github.com/octalide/mach-sieve) project.


### Hello World

```mach
use          std.runtime;
use print:   std.print;

$main.symbol = "main";
fun main(argc: i64, argv: &&u8) i64 {
    print.println("Hello, World!");
    ret 0;
}
```


### Fibonacci

```mach
use          std.runtime;
use print:   std.print;

fun fibr(n: u64) u64 {
    if (n < 2) {
        ret n;
    }

    ret fibr(n - 1) + fibr(n - 2);
}

$main.symbol = "main";
fun main(argc: i64, argv: &&u8) i64 {
    val max: u64 = 10;
    print.print("fib(");
    print.u64(max);
    print.print(") = ");
    print.u64(fibr(max));
    print.println("");
    ret 0;
}
```


### Factorial

```mach
use          std.runtime;
use print:   std.print;

fun fact(n: u64) u64 {
    if (n == 0) {
        ret 1;
    }

    ret n * fact(n - 1);
}

$main.symbol = "main";
fun main(argc: i64, argv: &&u8) i64 {
    val max: u64 = 10;
    print.print("fact(");
    print.u64(max);
    print.print(") = ");
    print.u64(fact(max));
    print.println("");
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


## Contributing

We welcome contributions to Mach! If you would like to contribute, please read our [contributing guidelines](CONTRIBUTING.md) first.


# License

Mach is licensed under the [MIT License](LICENSE).
