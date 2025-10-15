# Getting started

This guide walks through cloning the Mach repositories, compiling the bootstrap toolchain, and running the sample CLI from this repo. Commands assume a Unix-like shell and the following directory layout:

```
~/dev/src/github.com/octalide/
  ├── mach-c     # bootstrap compiler (C)
  ├── mach-std   # standard library modules
  └── mach       # this repository / future self-hosted compiler
```

## Prerequisites

- Clang/LLVM 16 or newer (provides `clang`, `lld`, `llvm-config`).
- POSIX toolchain (`make`, `cc`, `ar`).
- 64-bit Linux (current bootstrap target).

## 1. Clone the repositories

```bash
mkdir -p ~/dev/src/github.com/octalide
cd ~/dev/src/github.com/octalide
git clone https://github.com/octalide/mach-c.git
git clone https://github.com/octalide/mach-std.git
git clone https://github.com/octalide/mach.git
```

## 2. Build the compiler (`mach-c`)

```bash
cd mach-c
make              # produces bin/cmach
```

The build uses Clang and LLD. If they are not on your `PATH`, export `CC`, `CXX`, or adjust the `Makefile` accordingly.

## 3. Build the standard library (`mach-std`)

```bash
cd ../mach-std
make              # produces out/lib/libmachstd.a
```

The archive contains the runtime and modules imported via `use std.*`.

## 4. Build and run the sample CLI (`mach`)

```bash
cd ../mach
make              # compiles src/main.mach -> out/obj/main.o and links out/bin/mach
make run          # executes the binary
```

The `Makefile` invokes `../mach-c/bin/cmach build ... --emit-obj --no-link` and then links with `cc` plus the standard library archive. Use `make clean` to remove outputs.

## 5. Explore

- `./out/bin/mach help` prints the available commands (currently `help` and `build`).
- Modify `src/commands.mach` to add new subcommands or options.
- Review the [language specification](./language-spec.md) for syntactic and semantic rules.

Keep these instructions up to date as the self-hosted compiler comes online; they should always reflect the canonical workflow.
