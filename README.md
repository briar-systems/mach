MACH
===

![CI](https://github.com/briar-systems/mach/actions/workflows/ci.yml/badge.svg?branch=dev)
![License](https://img.shields.io/github/license/briar-systems/mach)
![Code Size](https://img.shields.io/github/languages/code-size/briar-systems/mach)
![Last Commit](https://img.shields.io/github/last-commit/briar-systems/mach)
![Issues](https://img.shields.io/github/issues/briar-systems/mach)

We have an official [Discord](https://discord.com/invite/dfWG9NhGj7)!

# Overview

Mach is a self hosted, statically-typed, compiled systems language designed to be simple, fast, verbose, and intuitive. Mach was created for projects like compilers, runtimes, operating systems, tooling, and embedded systems -- anywhere performance is a requirement and hidden behavior is a liability. The language is deliberately small and explicit: what you read is what executes, every cost is visible in the code that incurs it.

The compiler, code generators, and linker are written in native Mach, with no
LLVM or external assembler or linker. Ordinary Linux programs using the
standard library need no libc: the standard library talks to the kernel
directly. Where a platform requires its own libraries the standard library
links them, which today means libSystem on Darwin and `kernel32`, `ws2_32`,
`bcrypt` and the synchronization API set on Windows.

Memory is managed manually. There is no garbage collector and no hidden allocation. Memory flows through allocators that you create and pass explicitly, and the standard library is built around that style end to end: anything that allocates takes an allocator, and anything that doesn't never will. 

Batteries are not included. Many ways to do the same thing are not provided, and the language will not stop you from doing dangerous things. Safety is a decision made by the programmer, not a restriction imposed upon them.

Use Mach when you want C's reach with one coherent toolchain: a single binary that builds, links (no external linker), tests, formats, vendors dependencies, and cross-compiles.

Mach 5 adds tagged values with a declared discriminator (`tag`), `sel` as the
case test with lexical payload guards, the standard library's `res`, `opt` and
`err` failure tags, the `:>T` declassification cast for secret values, `mach
fmt` with one canonical layout, and an opt-in persistent object cache that
reuses generated objects across compiler processes. The compiler targets the
x86_64, aarch64, riscv64, riscv32 and SPIR-V instruction sets, the linux,
darwin, windows and freestanding operating systems, and writes ELF, COFF,
Mach-O, raw and SPIR-V images; `mach info` prints the full list for any
build.


# Getting Started

Read the [language reference](doc/language/README.md) before installing. The docs are written more like a pamphlet than a bible and assume familiarity with basic programming concepts from other languages.


## Installing Mach

Install the latest published release with one line. Until 5.0.0 is published
that release is a 4.x compiler, which reads the 4.x dialect and cannot build
this repository's source; see [Building Mach](#building-mach).

```bash
curl -fsSL https://machlang.org/install.sh | sh
```

On Windows (PowerShell):

```powershell
irm https://machlang.org/install.ps1 | iex
```

Precompiled binaries are also available directly on the [releases](https://github.com/briar-systems/mach/releases) page.


## Building Mach

Mach builds itself. The development source is Mach 5 source and pins std
2.0.0, so it needs a 5.0 compiler: a 4.x release cannot read it. Install a
5.0 release, or build one from the published 4.26.5 seed through the pinned
source bootstrap chain, `.github/actions/setup-mach/bootstrap.py`, which is
how CI builds its compiler on every native host; [CONTRIBUTING.md](CONTRIBUTING.md)
describes it. A project moving from 4.x follows the `CHANGELOG.md` entry for
5.0.0, which names every removed form and its replacement.

```bash
git clone https://github.com/briar-systems/mach
cd mach
mach dep pull .
mach build .
```

The compiler is written to `out/<target>/<profile>/bin/mach`, or `bin/mach.exe`
on Windows. A default Linux x86_64 build writes
`out/linux-x86_64/debug/bin/mach`. The compiler you build is generation A;
A builds B, B builds C, and B and C are byte-identical. See
[CONTRIBUTING.md](CONTRIBUTING.md) for the checks.


# Examples

The following examples require the standard library as a dependency. For a standalone starting point, see the [Mach Sieve](https://github.com/octalide/mach-sieve) project, or run `mach init` to scaffold one.


## Hello World

```mach
use          std.runtime;
use print:   std.print;

#[symbol("main")]
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

#[symbol("main")]
fun main(argc: i64, argv: **u8) i64 {
    print.printf("fib({}) = {}\n", 10::i64, fibr(10));
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

#[symbol("main")]
fun main(argc: i64, argv: **u8) i64 {
    print.printf("fact({}) = {}\n", 10::i64, fact(10));
    ret 0;
}
```


# Documentation

The full language reference is in [`doc/language/`](doc/language/README.md),
including [`doc/language/manifest.md`](doc/language/manifest.md), the
`mach.toml` reference. The command line is documented by the compiler itself:
`mach --help` lists the commands and `mach help <command>` describes each one.
`mach doc .` renders a project's docstrings to `doc/api/`.


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
