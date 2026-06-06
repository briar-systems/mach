MACH
===

![CI](https://github.com/octalide/mach/actions/workflows/ci.yml/badge.svg?branch=dev)
![License](https://img.shields.io/github/license/octalide/mach)
![Code Size](https://img.shields.io/github/languages/code-size/octalide/mach)
![Last Commit](https://img.shields.io/github/last-commit/octalide/mach)
![Issues](https://img.shields.io/github/issues/octalide/mach)

Mach is a statically-typed, compiled systems language designed to be simple, fast, verbose, and intuitive. The compiler is written in Mach and is **self-hosting**: it compiles its own source and reproduces itself bit-for-bit.

> Mach is still alpha quality. Expect breaking changes as the compiler and standard library iterate.

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
- **Flexibility**: Mach is rigid and opinionated. It should not be flexible or allow for many ways to do the same thing.
- **Code Reduction**: Mach is explicit and verbose. More code is not worse code.
- **Hand-holding**: Mach provides tools for safety (like read-only pointers and deferred cleanup), but it will not stop you from doing dangerous things if you explicitly ask to. Safety is a partnership between the language and the programmer.


# Getting Started

Read the [language reference](doc/language/README.md) before installing. The docs are written more like a pamphlet than a bible and assume familiarity with basic programming concepts from other languages.


## Building Mach

Prerequisites: git, make, curl.

```bash
git clone https://github.com/octalide/mach
cd mach
make
```

`make` runs the 4-stage bootstrap, each stage compiling the same Mach source with the previous compiler:

1. **cmach** — seed compiler (the pinned version is auto-downloaded from [mach-boot](https://github.com/octalide/mach-boot))
2. **imach** — intermediate compiler (`cmach` compiles the source)
3. **smach** — self-hosted compiler (`imach` compiles the source)
4. **mach** — final compiler (`smach` compiles the source)

The four binaries land in `out/bin/`; the final compiler is `out/bin/mach`. Because the language is self-hosting, the bootstrap reaches a byte-identical fixpoint — recompiling the source with `mach` reproduces `mach` exactly.

`make clean` wipes `out/`. To bootstrap from a custom seed: `CMACH=/path/to/cmach make`.


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

Common flags: `--target <name>` selects a `[targets.<name>]` entry, `--release` enables the optimisation pipeline, `--emit obj` stops at object files, `-o <path>` sets the output, and `--verbose`/`--quiet` adjust output. Run `mach help <command>` for the full set.


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
