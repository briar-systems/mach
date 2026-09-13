MACH
===

[![CI](https://github.com/briar-systems/mach/actions/workflows/ci.yml/badge.svg?branch=main)](https://github.com/briar-systems/mach/actions/workflows/ci.yml?query=branch%3Amain)
![License](https://img.shields.io/github/license/briar-systems/mach)
![Code Size](https://img.shields.io/github/languages/code-size/briar-systems/mach)
![Last Commit](https://img.shields.io/github/last-commit/briar-systems/mach/main)
![Issues](https://img.shields.io/github/issues/briar-systems/mach)

We have an official [Discord](https://discord.com/invite/dfWG9NhGj7)!

# Overview

Mach is a self hosted, statically-typed, compiled systems language designed to be simple, fast, verbose, and intuitive. Mach was created for projects like compilers, runtimes, operating systems, tooling, and embedded systems -- anywhere performance is a requirement and hidden behavior is a liability. The language is deliberately small and explicit: what you read is what executes, every cost is visible in the code that incurs it.

The compiler, code generators, and linker are written in native Mach, with no
LLVM or external assembler or linker. Ordinary Linux programs using the
standard library need no libc. The standard library uses platform libraries
where required, including libSystem on Darwin and Windows system DLLs.

Memory is managed manually. There is no garbage collector and no hidden allocation. Memory flows through allocators that you create and pass explicitly, and the standard library is built around that style end to end: anything that allocates takes an allocator, and anything that doesn't never will.

Batteries are not included. Many ways to do the same thing are not provided, and the language will not stop you from doing dangerous things. Safety is a decision made by the programmer, not a restriction imposed upon them.

Use Mach when you want C's reach with one coherent toolchain: a single binary that builds, links (no external linker), tests, formats, vendors dependencies, and cross-compiles.


# Getting Started

Read the [language reference](doc/language/README.md) before installing. The docs are written more like a pamphlet than a bible and assume familiarity with basic programming concepts from other languages.


## Installing Mach

Install the latest published release with one line. Until 5.0.0 is published
that release is a 4.x compiler.

```bash
curl -fsSL https://machlang.org/install.sh | sh
```

On Windows (PowerShell):

```powershell
irm https://machlang.org/install.ps1 | iex
```

Precompiled binaries are also available directly on the [releases](https://github.com/briar-systems/mach/releases) page.


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

`mach init` scaffolds a project around a file like this, with the standard
library as a dependency; `mach build .` builds it and `mach run .` runs it.


# Targets

Mach compiles for the x86_64, aarch64, riscv64, riscv32, and SPIR-V
instruction sets, for linux, darwin, windows, and freestanding, and writes
ELF, COFF, Mach-O, raw, and SPIR-V images. `mach info` prints the full list.


# Documentation

The language reference, including the `mach.toml` manifest, is in
[`doc/language/`](doc/language/README.md). `mach --help` and
`mach help <command>` document the command line. [`doc/api/`](doc/api/README.md)
is the rendered standard library and compiler API reference.


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
