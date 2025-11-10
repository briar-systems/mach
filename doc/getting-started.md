# Getting Started

This guide walks through the minimum setup required to build Mach from source and develop projects that depend on the standard library.

---

## Prerequisites

Mach currently depends on a few external tools and headers:

- **LLVM development headers and libraries** that match the version targeted by Mach. On most systems this can be installed via your package manager (e.g., `llvm-config`, `libLLVM`, and associated `clang` development packages).
- A **C toolchain** (compiler, linker, and standard library) used by the bootstrap build and by LLVM. The mach compiler uses `clang` by default. If you chose to use another toolchain, ensure you modify the `Makefile` accordingly.
- **Make** or another build tool capable of invoking the provided `Makefile`.

Verify that `llvm-config` is visible on your `PATH` and that the development headers are installed before attempting to build the compiler.

---

## Environment variables

Many Mach projects, including the mach compiler itself, expect the `MACH_HOME` environment variable to be set. Point it at the root of your Mach checkout before building or running tools:

```bash
export MACH_HOME="/path/to/mach"
```

When authoring a `mach.toml` file, reference the standard library via `MACH_HOME` so that downstream consumers resolve the dependency correctly:

```toml
[dependencies]
std = "${MACH_HOME}/std"
```

Using the environment variable keeps project configuration portable by avoiding hard-coded absolute paths.

---

## Building the compiler

1. Clone the repository and initialise submodules if needed.
2. Ensure the prerequisites above are installed and `MACH_HOME` is exported in your shell.
3. Run the bootstrap build:

```bash
make cmach
```

This produces the bootstrap compiler in `out/bin/`.

Refer to the [language tour](language-tour.md) and the remainder of the documentation in this directory for language details once the toolchain is in place.

### Self-hosted build

The self-hosted compiler (`imach` and `mach`) is currently unfinished. The source for that project is in this repo under `src`. Once the self-hosted compiler is complete, you will be able to build it by running:

```bash
make mach
```

---

## A note about OS support

Mach is currently only being tested on Linux platforms. While it may build and run on other operating systems, there is no guarantee of support or functionality outside of Linux at this time.

With that said, if you would like to help bring support for other operating systems, please reach out on Discord or open an issue on GitHub with a detailed description of problems you run into.