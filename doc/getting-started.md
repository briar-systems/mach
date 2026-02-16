# Getting Started

This guide covers the minimum setup required to build Mach from source and create a project.

> Mach is currently only tested on Linux (x86_64). It may build on other platforms but there is no guarantee.


## Prerequisites

### Git

Required for cloning the repository and managing dependencies.

```bash
git --version
```

### Make

GNU Make or a compatible tool for driving the build.

### C Compiler

The bootstrap compiler is written in C and requires a C23-capable compiler. Clang is preferred:

```bash
sudo apt install clang
clang --version
```

GCC also works if it supports `-std=c23`.


## Building the Compiler

```bash
git clone https://github.com/octalide/mach.git
cd mach
make full
```

This runs the 4-stage bootstrap:

| Stage | Command | Description |
|-------|---------|-------------|
| 1 | `make cmach` | C bootstrap compiler (`boot/src/` -> `out/bin/cmach`) |
| 2 | `make imach` | Intermediate compiler (cmach compiles `src/` -> `out/bin/imach`) |
| 3 | `make smach` | Self-hosted compiler (imach compiles `src/` -> `out/bin/smach`) |
| 4 | `make mach`  | Final compiler (smach compiles `src/` -> `out/linux/bin/mach`) |

`make full` runs all four stages. `make cmach` builds only the bootstrap if you need a quick start.

To run the test suite (492 tests):

```bash
make test
```


## Creating a Project

Use `mach init` to scaffold a new project:

```bash
mach init my-project
cd my-project
```

This creates:
- `mach.toml` -- project configuration
- `src/main.mach` -- entrypoint with a hello world program
- `dep/` -- dependency directory
- `.gitignore` -- standard ignores

The standard library is automatically added as a git submodule dependency.

Build and run:

```bash
mach build .
mach run .
```


## Hello World

The generated `src/main.mach`:

```mach
use          std.runtime;
use print:   std.print;

$main.symbol = "main";
fun main(argc: i64, argv: &&u8) i64 {
    print.println("Hello, World!");
    ret 0;
}
```

Key points:
- `use std.runtime;` imports the runtime startup code (required for executables).
- `$main.symbol = "main";` sets the linker symbol name for the function below it.
- `fun main(argc: i64, argv: &&u8) i64` is the program entrypoint with C-compatible signature.
- `ret 0;` returns the exit code.

See [modules.md](modules.md) for how `use` statements work and [config.md](config.md) for project configuration.
