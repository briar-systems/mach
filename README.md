MACH
===

![CI](https://github.com/octalide/mach/actions/workflows/ci.yml/badge.svg?branch=dev)
![License](https://img.shields.io/github/license/octalide/mach)
![Code Size](https://img.shields.io/github/languages/code-size/octalide/mach)
![Last Commit](https://img.shields.io/github/last-commit/octalide/mach)
![Issues](https://img.shields.io/github/issues/octalide/mach)

We have an official [Discord](https://discord.com/invite/dfWG9NhGj7)!

# Overview

Mach is a statically-typed, compiled systems language designed to be simple, fast, verbose, and intuitive. Mach is built for projects like compilers, runtimes, operating systems, tooling -- anywhere performance is a requirement and hidden behavior is a liability. The language is deliberately small and explicit: what you read is what executes, every cost is visible in the code that incurs it. Nothing happens by convention or inference.

Mach does not rely on any external dependencies for the compiler or during runtime -- no LLVM, no linking to libc, no system linker or other tools. The entire compiler and all base language features are written in native Mach. 

Memory is managed manually. There is no garbage collector and no hidden allocation. Memory flows through allocators that you create and pass explicitly, and the standard library is built around that style end to end: anything that allocates takes an allocator, and anything that doesn't never will. 

Batteries are not included. Many ways to do the same thing are not provided, and the language will not stop you from doing dangerous things. Safety is a decision made by the programmer, not a restriction imposed upon them.

Use Mach when you want C's reach with one coherent toolchain: a single binary that builds, links (no external linker), tests, vendors dependencies, and cross-compiles.


# Getting Started

Read the [language reference](doc/language/README.md) before installing. The docs are written more like a pamphlet than a bible and assume familiarity with basic programming concepts from other languages.


## Installing Mach

Install the latest release with one line:

```bash
curl -fsSL https://machlang.org/install.sh | sh
```

On Windows (PowerShell):

```powershell
irm https://machlang.org/install.ps1 | iex
```

The scripts verify the download against the release `SHA256SUMS` and install to
`~/.local/bin` (`%LOCALAPPDATA%\mach\bin` on Windows). Precompiled binaries are
also available directly on the [releases](https://github.com/octalide/mach/releases) page.


## Building Mach

Mach builds itself, so building from source needs an existing `mach` installation.

```bash
git clone https://github.com/octalide/mach
cd mach
mach dep pull
mach build .
```

The compiler is written to `out/<target>/bin/mach`, where `<target>` is the selected target name.


# Examples

The following examples require the standard library as a dependency. For a standalone starting point, see the [Mach Sieve](https://github.com/octalide/mach-sieve) project, or run `mach init` to scaffold one.


## Hello World

```mach
use          std.runtime;
use print:   std.print;

`symbol("main")`
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

`symbol("main")`
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

`symbol("main")`
fun main(argc: i64, argv: **u8) i64 {
    print.printf("fact(%d) = %d\n", 10::i64, fact(10));
    ret 0;
}
```


# Documentation

The full language reference is in [`doc/language/`](doc/language/README.md). The
build system is documented in:

- [`doc/manifest.md`](doc/manifest.md): the `mach.toml` manifest reference
- [`doc/cli.md`](doc/cli.md): the `mach` command-line reference


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
