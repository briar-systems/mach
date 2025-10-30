# Project Layout

This document defines the standard layout for Mach projects, including the project manifest (`mach.toml`), directory structure conventions, and how modules are organized and referenced.

It is meant to serve as a "best practices" guide for Mach projects to ensure consistency and compatibility with tooling. It is not strictly enforced; projects may deviate as needed, but following these conventions will improve developer experience and tool support.

---

## Table of Contents

- [Project Layout](#project-layout)
  - [Table of Contents](#table-of-contents)
  - [Standard Directory Structure](#standard-directory-structure)
  - [Project Manifest (`mach.toml`)](#project-manifest-machtoml)
    - [Basic structure](#basic-structure)
    - [`[project]` section](#project-section)
    - [`[targets.<name>]` section](#targetsname-section)
    - [`[dependencies]` section (formerly `[directories]` and `[deps]`)](#dependencies-section-formerly-directories-and-deps)
  - [Build Organization](#build-organization)
    - [Build artifacts](#build-artifacts)
  - [Dependencies](#dependencies)
    - [Local dependency example](#local-dependency-example)
    - [Standard library](#standard-library)
  - [Entry points](#entry-points)

---

## Standard Directory Structure

A typical Mach project follows this layout:

```
my-project/
├── dep/               # external dependencies (optional)
│   └── some-lib/
├── doc/               # documentation (optional)
├── out/               # build artifacts (generated)
│   ├── bin/           # final executables (all stages)
│   │   ├── cmach      # bootstrap compiler
│   │   ├── imach      # intermediate compiler
│   │   └── mach       # final compiler
│   ├── cmach/         # bootstrap compiler artifacts
│   │   └── obj/       # object files
│   ├── imach/         # intermediate compiler artifacts
│   │   └── <target>/  # per-target artifacts
│   │       ├── ast/   # AST files
│   │       ├── ir/    # LLVM IR
│   │       ├── asm/   # assembly
│   │       └── obj/   # object files
│   └── mach/          # final compiler artifacts
│       └── <target>/  # per-target artifacts
├── src/               # source files
│   ├── main.mach      # entry point
│   └── ...            # other source files specific to the project
├── mach.toml          # project manifest
└── Makefile           # build system (optional)
```

**Directory conventions:**

- **`src/`** – Primary source directory. Contains `.mach` files for your project.
- **`dep/`** – External dependencies. Each dependency is a subdirectory containing its own Mach source.
- **`doc/`** – Project documentation.
- **`out/`** – Generated build artifacts. Should be excluded from version control.
  - **`out/bin/`** – Final executables from all build stages
  - **`out/<stage>/<target>/`** – Per-stage, per-target build artifacts

**Minimal project:**

At minimum, a Mach project needs:
- `mach.toml` – project configuration
- `src/` – at least one `.mach` file with an entry point

---

## Project Manifest (`mach.toml`)

Every Mach project requires a `mach.toml` file at the root. This TOML file configures the project, specifies build targets, and declares dependencies.

> NOTE: The mach project file is technically NOT required to build simple programs or to customize the build process, as the compiler can be invoked with explicit flags. However, using `mach.toml` simplifies builds by providing defaults and reducing command-line complexity.

### Basic structure

```toml
[project]
name = "my_project"
version = "0.1.0"
src = "src"
target = "native"  # or "all", or a specific target name

[dependencies]
std = "std"

[targets.linux]
triple = "x86_64-pc-linux-gnu"
entrypoint = "main.mach"
artifacts = "out/mach/linux"
out = "bin/mach"
opt-level = 2
emit-ast = true
emit-ir = true
emit-asm = true
emit-object = true
build-library = false
no-pie = false
```

### `[project]` section

Identifies the project and build configuration:

- **`name`** (required) – Project name. Used for build artifacts and module namespace.
- **`version`** (required) – Semantic version string (e.g., `"0.1.0"`).
- **`src`** (required) – Source directory containing `.mach` files (default: `"src"`).
- **`target`** (required) – Default target to build:
  - `"native"` – Build for the host platform (auto-detected via LLVM)
  - `"all"` – Build for all defined targets
  - `"<target-name>"` – Build a specific target (e.g., `"linux"`, `"darwin"`, `"windows"`)

**Example:**
```toml
[project]
name = "http-server"
version = "1.2.3"
src = "src"
target = "native"
```

### `[targets.<name>]` section

Each target defines a build configuration for a specific platform:

- **`triple`** (required) – LLVM target triple (e.g., `"x86_64-pc-linux-gnu"`)
- **`entrypoint`** (required) – Main source file relative to the `src` directory
- **`artifacts`** (required) – Directory for build artifacts (AST, IR, ASM, OBJ) relative to project root
- **`out`** (required) – Final executable/library path (relative to project root or absolute)
- **`opt-level`** (required) – Optimization level (0-3)
- **`emit-ast`** (required) – Emit AST files (true/false)
- **`emit-ir`** (required) – Emit LLVM IR files (true/false)
- **`emit-asm`** (required) – Emit assembly files (true/false)
- **`emit-object`** (required) – Emit object files (true/false)
- **`build-library`** (required) – Build as library instead of executable (true/false)
- **`shared`** (optional) – Build shared library if build-library=true (true/false)
- **`no-pie`** (optional) – Disable position-independent executable (true/false)
- **`link`** (optional, repeatable) – External libraries to link (e.g., `link = "/usr/lib/libglfw.so"`)

**Example:**
```toml
[targets.linux]
triple = "x86_64-pc-linux-gnu"
entrypoint = "main.mach"
artifacts = "out/mach/linux"
out = "bin/mach"
opt-level = 2
emit-ast = true
emit-ir = true
emit-asm = true
emit-object = true
build-library = false
no-pie = false

[targets.darwin]
triple = "x86_64-apple-darwin"
entrypoint = "main.mach"
artifacts = "out/mach/darwin"
out = "bin/mach"
opt-level = 2
emit-ast = false
emit-ir = false
emit-asm = false
emit-object = true
build-library = false
no-pie = false
```

### `[dependencies]` section (formerly `[directories]` and `[deps]`)

Maps dependency names to their locations. Dependencies are effectively mappings for module resolution.

**Simple local dependency:**
```toml
[dependencies]
std = "std"              # points to ./std/
mylib = "dep/mylib/src"  # points to ./dep/mylib/src
```

Dependencies must point to mach SOURCE directories, not project roots.

> NOTE: This is likely to change in the future to support more complex dependency specifications.

---

## Build Organization

### Build artifacts

The compiler generates several types of artifacts organized by build stage and target:

```
out/
├── bin/                      # final executables from all build stages
│   ├── cmach                 # bootstrap compiler (C-based)
│   ├── imach                 # intermediate compiler (Mach-based)
│   └── mach                  # final compiler (Mach-based)
├── cmach/                    # bootstrap compiler artifacts
│   └── obj/                  # object files (.o)
├── imach/                    # intermediate compiler artifacts
│   └── <target>/             # per-target (e.g., linux, darwin, windows)
│       ├── ast/              # AST files (.ast)
│       │   └── <namespace>/  # module namespace (e.g., mach/)
│       ├── ir/               # LLVM IR files (.ll)
│       │   └── <namespace>/
│       ├── asm/              # assembly files (.s)
│       │   └── <namespace>/
│       └── obj/              # object files (.o)
│           └── <namespace>/
└── mach/                     # final compiler artifacts
    └── <target>/             # per-target organization
        ├── ast/
        ├── ir/
        ├── asm/
        └── obj/
```

**Key points:**

- All final executables go in `out/bin/` regardless of build stage
- Artifacts are organized by stage (`cmach`, `imach`, `mach`)
- Multi-stage builds use per-target subdirectories for cross-compilation
- Modules are namespaced within artifact directories (e.g., `mach/main.ll`)

**Exclude from version control:**

Add to `.gitignore`:
```
out/
*.o
*.a
```

---

## Dependencies

### Local dependency example

Place dependencies in the `dep/` directory:

```
my-project/
├── mach.toml
├── src/
│   └── main.mach
└── dep/
    ├── json-parser/
    │   ├── parser.mach
    │   └── types.mach
    └── http-client/
        └── client.mach
```

Configure in `mach.toml`:
```toml
[dependencies]
json = "dep/json-parser"
http = "dep/http-client"
```

Import in code:
```mach
use json.parser;
use http.client;
```

### Standard library

The Mach standard library is typically a dependency:

```toml
[dependencies]
std = "path/to/std"
```

Common imports:
```mach
use std.types.string;        # string utilities
use std.types.list;          # dynamic arrays (List[T])
use std.types.option;        # optional values (Option[T])
use std.types.result;        # result type (Result[T, E])
use std.io.console;          # console i/o
use std.io.fs;               # file system
use std.system.memory;       # memory allocation
use std.system.time;         # time utilities
use std.system.env;          # environment variables
```

> NOTE: The standard library path will be eventually resolved through environment variables or global configuration rather than hardcoding in each project. This option will still be required, but the syntax and usage may change to support this feature in the future.

---

## Entry points

The entry point must contain a main function:

```mach
use std.runtime;
use std.types.string;

$main.symbol = "main";
fun main(args: []string) i64 {
    # program logic
    ret 0;
}
```

The `$main.symbol = "main"` attribute assignment ensures the function is exported with an unmangled symbol name. This is a requirement for the runtime included in the standard library to link properly. If you are seeing issues related to undefined references to `main` or lack of a `_start` symbol, ensure this attribute is set and the runtime is imported without an alias.

---

This layout provides a consistent structure for Mach projects, making them easy to understand and maintain. Projects can deviate from this structure when needed, but following these conventions improves tooling compatibility and developer experience.
