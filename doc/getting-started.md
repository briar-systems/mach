# Getting Started

This guide walks through the minimum setup required to build Mach from source.

> Mach is currently only being tested on Linux platforms. While it may build and run on other operating systems, there is no guarantee of support or functionality outside of Linux at this time.

- [Getting Started](#getting-started)
  - [Prerequisites](#prerequisites)
    - [Git](#git)
    - [Make](#make)
    - [LLVM Development Libraries (v19+)](#llvm-development-libraries-v19)
    - [C Compiler](#c-compiler)
  - [Building the Bootstrap Compiler](#building-the-bootstrap-compiler)
  - [Building the Self-hosted Compiler](#building-the-self-hosted-compiler)
  - [Making a New Project](#making-a-new-project)

---

## Prerequisites

### Git

Git is required for cloning the Mach repository as well as management of dependencies through the mach toolchain.

You can ensure git is installed with:

```bash
git --version
```

Please install the appropriate package for your system if git is not found.

### Make

The simplest way to build Mach is via the provided `Makefile`. This requires GNU Make or a compatible tool.

### LLVM Development Libraries (v19+)

Mach requires the LLVM development headers and libraries (version 19 or later). Ensure that `llvm-config` is visible on your `PATH` and that the development headers are installed before attempting to build the compiler.

On Debian-based systems, you can install the required package with:

```bash
sudo apt install llvm-dev
```

Use the following command to verify that `llvm-config` is available and is the correct version:

```bash
llvm-config --version
```

### C Compiler

Mach's bootstrap compiler is written in C and requires a C compiler to build. The existing build system has a preference for `clang`, but similar tools like `gcc` may also work.

To ensure `clang` is installed on a Debian-based system, run:

```bash
sudo apt install clang
clang --version
```

## Building the Bootstrap Compiler

```bash
git clone https://github.com/octalide/mach.git
cd mach
make cmach
```

This produces the bootstrap compiler at `<repo_root>/out/bin/cmach`.

## Building the Self-hosted Compiler

The commands `make imach` and `make mach` build the intermediate and final self-hosted compilers, respectively.

These stages of the toolchain are incomplete. Failure to compile these stages, especially `make mach`, is expected at this time.

Once complete, the production compiler can be built like so:

```bash
git clone https://github.com/octalide/mach.git
cd mach
make mach
```

## Making a New Project

To create a new Mach project, the `mach init` tool is suggested. This tool will scaffold a new project structure with sensible defaults:

```bash
cmach init mach-project
cd mach-project
```

Do note that this command will automatically add the standard library (located at https://github.com/octalide/mach-std) as a vendored dependency.

From here, you can build your Mach project with:

```bash
cmach build .
```

This will compile your new Mach project according to the configuration specified in the `mach.toml` file located in the project root.

You can execute the compiled binary with either:

```bash
cmach run .
```

or directly with:

```bash
./out/bin/mach-project
```
