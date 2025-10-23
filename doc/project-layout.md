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
    - [`[directories]` section](#directories-section)
    - [`[deps]` section](#deps-section)
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
│   ├── obj/           # object files
│   └── bin/           # executables
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

**Minimal project:**

At minimum, a Mach project needs:
- `mach.toml` – project configuration
- `src/` – at least one `.mach` file with an entry point

---

## Project Manifest (`mach.toml`)

Every Mach project requires a `mach.toml` file at the root. This TOML file configures the project, specifies directories, and declares dependencies.

> NOTE: The mach project file is technically NOT required to build simple projects, as the compiler can be invoked with explicit flags. However, using `mach.toml` simplifies builds by providing defaults and reducing command-line complexity.

### Basic structure

```toml
[project]
name = "my_project"
version = "0.1.0"
entrypoint = "main.mach"

[directories]
src = "src"
out = "out"

[deps]
# dependency declarations
```

### `[project]` section

Identifies the project and its entry point:

- **`name`** (required) – Project name. Used for build artifacts and identification.
- **`version`** (required) – Semantic version string (e.g., `"0.1.0"`).
- **`entrypoint`** (required) – Main source file relative to the `src` directory. Must contain the program's entry point.

**Example:**
```toml
[project]
name = "http-server"
version = "1.2.3"
entrypoint = "main.mach"
```

### `[directories]` section

Specifies project directory layout:

- **`src`** (optional, default: `"src"`) – Source directory containing `.mach` files.
- **`out`** (optional, default: `"out"`) – Output directory for build artifacts.

**Example:**
```toml
[directories]
src = "source"
out = "build"
```

### `[deps]` section

Maps dependency names to their locations. Dependencies are effectively mappings for module resolution.

**Simple local dependency:**
```toml
[deps]
std = "std"              # points to ./std/
mylib = "dep/mylib/src"  # points to ./dep/mylib/src
```

Dependencies must point to mach SOURCE directories, not project roots.

> NOTE: This is likely to change in the future to support more complex dependency specifications.

---

## Build Organization

### Build artifacts

The compiler generates several types of artifacts in the `out/` directory:

```
out/
├── obj/               # compiled object files (.o)
├── asm/               # assembly listings (with --emit-asm)
├── ast/               # AST dumps (with --emit-ast)
├── ir/                # LLVM IR (with --emit-ir)
└── bin/               # final executables or libraries
    └── my_project
```

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
[deps]
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
[deps]
std = "path/to/std"
```

Common imports:
```mach
use std.types.string;        # string utilities
use std.types.list;          # dynamic arrays (List<T>)
use std.types.option;        # optional values (Option<T>)
use std.types.result;        # result type (Result<T, E>)
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

#@symbol("main")
fun main(args: []string) i64 {
    # program logic
    ret 0;
}
```

The `#@symbol("main")` directive removes name mangling for the entry point defined in `std.runtime` in this example. This is a requirement for the runtime included in the standard library to link properly. If you are seeing issues related to undefined references to `main` or lack of a `_start` symbol, ensure this directive is present and the runtime is imported without an alias.

---

This layout provides a consistent structure for Mach projects, making them easy to understand and maintain. Projects can deviate from this structure when needed, but following these conventions improves tooling compatibility and developer experience.
