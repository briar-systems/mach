MACH
===

[![CI](https://github.com/briar-systems/mach/actions/workflows/ci.yml/badge.svg?branch=main)](https://github.com/briar-systems/mach/actions/workflows/ci.yml?query=branch%3Amain)
![License](https://img.shields.io/github/license/briar-systems/mach)
![Code Size](https://img.shields.io/github/languages/code-size/briar-systems/mach)
![Last Commit](https://img.shields.io/github/last-commit/briar-systems/mach/main)
![Issues](https://img.shields.io/github/issues/briar-systems/mach)

We have an official [Discord](https://discord.com/invite/dfWG9NhGj7)!

# Overview

Mach is a self hosted, statically-typed, compiled systems language designed to be simple, fast, verbose, and intuitive. Mach was created for projects like compilers, runtimes, operating systems, tooling, games, and embedded systems -- anywhere performance is a requirement and hidden behavior is a liability. The language is deliberately small and explicit: what you read is what executes, every cost is visible in the code that incurs it.

The compiler, code generators, and linker are written in native Mach, with no external dependencies whatsoever.
The standard library uses platform libraries where required, including libSystem on Darwin and Windows system DLLs.

Memory is managed manually. There is no garbage collector and no hidden allocation.

Batteries are not included. Many ways to do the same thing are not provided, and the language will not stop you from doing dangerous things. Safety is a decision made by the programmer, not a restriction imposed upon them.

Use Mach when you want C's reach with one coherent toolchain: a single binary that builds, links, tests, formats, vendors dependencies, and cross-compiles (it cooks and cleans if you ask nicely too!).


# Getting Started

Read the [language reference](doc/language/README.md) before installing. The docs are written more like a pamphlet than a bible and assume familiarity with basic programming concepts from other languages.


## Installing Mach

Install the latest published release with one line:

```bash
curl -fsSL https://machlang.org/install.sh | sh
```

On Windows (PowerShell):

```powershell
irm https://machlang.org/install.ps1 | iex
```

Precompiled binaries are also available directly on the [releases](https://github.com/briar-systems/mach/releases) page.


## Hello World

Create a new mach project:

```bash
mach init <project_name>
cd <project_name>
```

You will find the source code for a simple "Hello World" program in `src/main.mach`. Build and run it:

```bash
mach build .
mach run .
```

> NOTE: `mach build .` and `mach run .` are *separate commands*. `mach run .` does not build the project first, so you must run `mach build .` before running the program.


# Targets

Mach compiles to a LOT of combinatorial targets. Run `mach info targets` to see the full list of targets your installed compiler version supports.


# Documentation

`doc/` contains language documentation ([`doc/language/`](doc/language/README.md)) as well as generated documentation for the compiler project itself.


## Contributing

We welcome contributions to Mach! If you would like to contribute, please read our [contributing guidelines](CONTRIBUTING.md) first.

# Credit

The inspiration for Mach comes from too many languages to count.

Direct inspiration for the compiler itself comes from a few specific sources:

- [Golang](https://golang.org/)
- [Vlang](https://vlang.org/)
- [Zig](https://ziglang.org/)
- [Rust](https://www.rust-lang.org/)

Mach is licensed under the [MIT License](LICENSE).

Mach stands on the shoulders of countless giants that have contributed to the development of these languages either directly or by proxy. It is out of respect for their work that Mach will always be fully open source. Thank you all.
