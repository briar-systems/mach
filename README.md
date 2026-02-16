MACH
===

![CI](https://github.com/octalide/mach/actions/workflows/ci.yml/badge.svg?branch=dev)
![License](https://img.shields.io/github/license/octalide/mach)
![Code Size](https://img.shields.io/github/languages/code-size/octalide/mach)
![Last Commit](https://img.shields.io/github/last-commit/octalide/mach)
![Issues](https://img.shields.io/github/issues/octalide/mach)

Mach is a statically-typed, compiled programming language designed to be simple, fast, verbose, and intuitive.

> Mach is still alpha quality. Expect breaking changes as the compiler and standard library iterate.

We have an official [Discord](https://discord.com/invite/dfWG9NhGj7)!

# Overview

- [Core Philosophy](#core-philosophy)
- [Getting Started](#getting-started)
- [Examples](#examples)
- [Documentation](#documentation)
- [Credit](#credit)
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

Read the [language documentation](doc/README.md) before installing. The docs are written more like a pamphlet than a bible and assume familiarity with basic programming concepts from other languages.


## Building Mach

Prerequisites: git, make, clang (or gcc with C23 support). See [getting started](doc/getting-started.md) for details.

```bash
git clone https://github.com/octalide/mach
cd mach
make full
```

This runs the 4-stage bootstrap:
1. **cmach** -- C bootstrap compiler (compiles from `boot/src/`)
2. **imach** -- intermediate compiler (cmach compiles the Mach source)
3. **smach** -- self-hosted compiler (imach compiles the Mach source)
4. **mach** -- final compiler (smach compiles the Mach source)

The final binary is at `out/linux/bin/mach`. To build only the bootstrap compiler: `make cmach`.


# Examples

The following examples require the standard library as a dependency. For a standalone project, see the [getting started guide](doc/getting-started.md) or the [Mach Sieve](https://github.com/octalide/mach-sieve) project.


## Hello World

```mach
use          std.runtime;
use print:   std.print;

$main.symbol = "main";
fun main(argc: i64, argv: &&u8) i64 {
    print.println("Hello, World!");
    ret 0;
}
```


## Fibonacci

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
    print.printf("fib(%d) = %d\n", 10::i64, fibr(10));
    ret 0;
}
```


## Factorial

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
    print.printf("fact(%d) = %d\n", 10::i64, fact(10));
    ret 0;
}
```


# Documentation

The full language and tooling documentation is in [`doc/`](doc/README.md).


# Credit

The inspiration for Mach comes from too many languages to count. Almost every language has problems that Mach attempts to elegantly resolve (most often by the process of reductive simplification).

Direct inspiration for the compiler comes from a few specific sources:

- [Golang](https://golang.org/)
- [Vlang](https://vlang.org/)
- [Zig](https://ziglang.org/)
- [Rust](https://www.rust-lang.org/)

Mach, at its core, stands on the shoulders of countless giants that have contributed to the development of these languages either directly or by proxy. It is out of respect for their work that Mach will always be fully open source. Thank you all.


## Contributing

We welcome contributions to Mach! If you would like to contribute, please read our [contributing guidelines](CONTRIBUTING.md) first.


# License

Mach is licensed under the [MIT License](LICENSE).
