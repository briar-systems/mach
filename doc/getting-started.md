# Getting Started with Mach

This guide walks you through setting up Mach, building the compiler from source, and running your first Mach program.

---

## Table of Contents

- [Prerequisites](#prerequisites)
- [Building the Compiler](#building-the-compiler)
  - [Quick Start](#quick-start)
  - [Understanding the Build Process](#understanding-the-build-process)
  - [Build Targets](#build-targets)
- [Running Your First Program](#running-your-first-program)
- [Project Structure](#project-structure)
- [Next Steps](#next-steps)

---

## Prerequisites

Before building Mach, ensure you have the following installed:

- **C compiler**: `clang` (C23 support required)
- **LLVM**: Version 14 or later with development headers
  - `llvm-config` must be in your PATH
- **Make**: Standard GNU Make
- **Git**: For cloning the repository

### Installation by Platform

**Linux (Debian/Ubuntu):**
```bash
sudo apt update
sudo apt install clang llvm-dev make git
```

**Linux (Fedora/RHEL):**
```bash
sudo dnf install clang llvm-devel make git
```

**macOS:**
```bash
brew install llvm make
# add llvm to path
export PATH="/opt/homebrew/opt/llvm/bin:$PATH"
```

**Verify installation:**
```bash
clang --version
llvm-config --version
make --version
```

---

## Building the Compiler

### Quick Start

Clone and build the complete compiler chain:

```bash
# clone repository
git clone https://github.com/octalide/mach.git
cd mach

# build entire compiler chain (cmach -> imach -> mach)
make full
```

This builds three compiler stages:
1. **cmach** - Bootstrap C compiler (`boot/` → `out/cmach/bin/cmach`)
2. **imach** - Intermediary Mach compiler (`src/` → `out/imach/bin/imach`)
3. **mach** - Final Mach compiler (`src/` → `out/mach/bin/mach`)

### Understanding the Build Process

Mach uses a bootstrapping approach:

1. **Bootstrap (cmach)**: The C compiler in `boot/` compiles Mach source files to LLVM IR and native code. This is written in C23 and uses LLVM libraries directly.

2. **Intermediary (imach)**: The bootstrap compiler builds the Mach compiler (written in Mach) from `src/`. This validates the compiler can compile itself.

3. **Final (mach)**: The intermediary compiler rebuilds itself, creating the final production compiler.

### Build Targets

The Makefile provides granular control over the build process:

**Bootstrap compiler (cmach):**
```bash
make cmach-build    # build cmach
make cmach-clean    # clean cmach artifacts
make cmach          # clean and build
```

**Intermediary compiler (imach):**
```bash
make imach-build    # build imach
make imach-clean    # clean imach artifacts
make imach          # clean and build
```

**Final compiler (mach):**
```bash
make mach-build     # build mach
make mach-clean     # clean mach artifacts
make mach           # clean and build
```

**Meta targets:**
```bash
make full           # build entire chain
make clean          # clean all artifacts
make help           # show all targets
```

**Build output:**
- Bootstrap compiler: `out/cmach/bin/cmach`
- Intermediary compiler: `out/imach/bin/imach`
- Final compiler: `out/mach/bin/mach`

---

## Running Your First Program

### Hello World

Create a file `hello.mach`:

```mach
use          std.runtime;
use          std.types.string;
use console: std.io.console;

#@symbol("main")
fun main(args: []string) i64 {
    console.print("hello, world!\n");
    ret 0;
}
```

**Compile and run:**

```bash
# using the bootstrap compiler
./out/cmach/bin/cmach build hello.mach -o hello

# or using the final compiler
./out/mach/bin/mach build hello.mach -o hello

# run the program
./hello
```

### Compiler Flags

Common `cmach` flags:

- `-o <file>` - Output executable path
- `-I <dir>` - Add include directory
- `-M <name>=<path>` - Map module to directory
- `--obj-dir=<dir>` - Object file output directory
- `--dep-dir=<dir>` - Dependency file directory
- `--emit-asm` - Emit assembly listings
- `--emit-ast` - Emit abstract syntax tree
- `--emit-ir` - Emit LLVM IR

**Example with flags:**
```bash
./out/cmach/bin/cmach build hello.mach \
    -I std \
    -M std=std \
    --obj-dir=build/obj \
    --emit-ir \
    -o hello
```

---

## Project Structure

Mach projects typically follow this layout:

```
my-project/
├── mach.toml          # project configuration
├── src/
│   ├── main.mach      # entry point
│   └── ...            # other source files
└── dep/               # dependencies (optional)
```

### Project Configuration (`mach.toml`)

Create a `mach.toml` for your project:

```toml
[project]
name = "my-project"
version = "0.1.0"
entrypoint = "main.mach"

[directories]
src-dir = "src"
dep-dir = "dep"
out-dir = "out"

[modules]
# module mappings
```

### Using the Standard Library

The Mach standard library is located in the `std/` directory. For standalone projects, you can configure the standard library path using:

**1. Environment variable (recommended for system-wide installation):**
```bash
export MACH_STD=/path/to/mach/std
```

**2. Command line flag (per-build override):**
```bash
cmach build mach.toml --std /path/to/mach/std
```

**3. In `mach.toml` using environment variable:**
```toml
[deps]
std = "$MACH_STD"
```

**4. In `mach.toml` using absolute or relative path:**
```toml
[deps]
std = "/absolute/path/to/std"
# or
std = "../mach/std"
```

The `--std` flag takes precedence over `MACH_STD`, which takes precedence over the literal path in `mach.toml`.

**Import standard library modules with `use`:**

```mach
use std.types.string;        # string utilities
use std.types.list;          # dynamic arrays
use std.types.option;        # optional values
use std.types.result;        # result type
use std.io.console;          # console i/o
use std.io.fs;               # file system
use std.system.memory;       # memory management
use std.system.time;         # time utilities
use std.system.env;          # environment variables
```

**Aliasing imports:**
```mach
use mem: std.system.memory;  # use as 'mem'
```

---

## Next Steps

Now that you have Mach installed:

1. **Read the language specification**: [`doc/language-spec.md`](language-spec.md) covers all language features in detail.

2. **Explore examples**: Check `src/` and `std/` for real-world Mach code.

3. **Understand project layout**: See [`doc/project-layout.md`](project-layout.md) for organizing larger projects.

4. **Contribute**: Read [`CONTRIBUTING.md`](../CONTRIBUTING.md) to help improve Mach.

5. **Join the community**: Report issues and contribute on [GitHub](https://github.com/octalide/mach).

---

## Troubleshooting

**Build fails with LLVM errors:**
- Ensure `llvm-config` is in your PATH
- Verify LLVM version is 14 or later
- Check that development headers are installed

**Compiler crashes or produces errors:**
- File an issue with minimal reproduction case
- Include compiler version and platform details
- Check existing issues for known problems

**Import errors:**
- Verify module paths in `-M` flags
- Ensure standard library path is correct
- Check `mach.toml` module mappings

For more help, consult the documentation or open an issue on GitHub.
