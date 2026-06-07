MACH
===

![CI](https://github.com/octalide/mach/actions/workflows/ci.yml/badge.svg?branch=dev)
![License](https://img.shields.io/github/license/octalide/mach)
![Code Size](https://img.shields.io/github/languages/code-size/octalide/mach)
![Last Commit](https://img.shields.io/github/last-commit/octalide/mach)
![Issues](https://img.shields.io/github/issues/octalide/mach)

Mach is a statically-typed, compiled systems language designed to be simple, fast, verbose, and intuitive.

We have an official [Discord](https://discord.com/invite/dfWG9NhGj7)!

# Overview

- [Core Philosophy](#core-philosophy)
- [Getting Started](#getting-started)
- [Usage](#usage)
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
- **Flexibility**: Mach does not allow for many ways to do the same thing.
- **Code Reduction**: Mach is explicit and verbose by design. More code is not worse code.
- **Hand-holding**: Mach will not stop you from doing dangerous things. Safety is a decision made by the programmer, not a restriction to be imposed upon them.


# Getting Started

Read the [language reference](doc/language/README.md) before installing. The docs are written more like a pamphlet than a bible and assume familiarity with basic programming concepts from other languages.


## Building Mach

Mach builds itself, so building from source needs an existing `mach` — install the latest [release](https://github.com/octalide/mach/releases) first.

```bash
git clone --recurse-submodules https://github.com/octalide/mach
cd mach
mach build .
```

The compiler is written to `out/<target>/bin/mach`.


# Usage

```
mach <command> [options]
```

| Command | Description |
|---|---|
| `build` | compile the current project to an executable or object |
| `run`   | build and execute the current project (`-- args...` forward to the program) |
| `test`  | build and run the project's tests |
| `dep`   | manage vendored dependencies (`list`, `add`, `remove`, `sync`, `vendor`) |
| `init`  | scaffold a new project (`--bin`, `--lib`, `--name`) |
| `help`  | show usage; `mach help <command>` for detail |

Run `mach help <command>` for more information about a specific subcommand.


# Examples

The following examples require the standard library as a dependency. For a standalone starting point, see the [Mach Sieve](https://github.com/octalide/mach-sieve) project, or run `mach init` to scaffold one.


## Hello World

```mach
use          std.runtime;
use print:   std.print;

$main.symbol = "main";
fun main(argc: i64, argv: **u8) i64 {
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
fun main(argc: i64, argv: **u8) i64 {
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
fun main(argc: i64, argv: **u8) i64 {
    print.printf("fact(%d) = %d\n", 10::i64, fact(10));
    ret 0;
}
```


# Documentation

The full language reference is in [`doc/language/`](doc/language/README.md).


# Credit

The inspiration for Mach comes from too many languages to count.

Direct inspiration for the compiler itself comes from a few specific sources:

- [Golang](https://golang.org/)
- [Vlang](https://vlang.org/)
- [Zig](https://ziglang.org/)
- [Rust](https://www.rust-lang.org/)

Mach stands on the shoulders of countless giants that have contributed to the development of these languages either directly or by proxy. It is out of respect for their work that Mach will always be fully open source. Thank you all.


## Contributing

We welcome contributions to Mach! If you would like to contribute, please read our [contributing guidelines](CONTRIBUTING.md) first.


# License

Mach is licensed under the [MIT License](LICENSE).
