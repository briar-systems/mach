# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [1.1.1] - 2026-06-09

Patch release for project scaffolding correctness.

### Fixed

- `mach init` now scaffolds fully buildable projects with all required manifest
  entries and `mach-std` dependency wiring.

## [1.1.0] - 2026-06-08

Tooling and cross-compilation release. Mach can now emit Windows executables,
link dynamically, manage dependencies, and answer editor queries — while the
Linux self-host continues to build to a byte-identical fixpoint.

### Added

- **Windows cross-compilation** for `x86_64-windows`: the Microsoft x64 calling
  convention (win64 ABI), a COFF/PE object and executable writer, and kernel32
  import linking. Mach builds runnable Windows `.exe`s. (Running the compiler
  itself natively on Windows is in progress for a later release.)
- **ELF dynamic linking**: link against shared libraries via `-l`/`-L` and
  `[targets.*].libs`, with a real PLT/GOT and `DT_NEEDED`/`PT_INTERP`.
- **External linking and static archives**: link prebuilt `.o`/`.a` inputs; a
  Unix `ar` archive reader.
- **`mach dep`**: git-based dependency management (`add`/`remove`/`sync`/`vendor`)
  with a `mach.lock`.
- **`mach check`**: single-file diagnostics with no project or link step.
- **Per-target optimization levels** via the manifest, overridable on the CLI.
- **Editor query surface** (`mach.lang.editor`): single-file/unsaved-buffer
  open, parse, resolve, and diagnostics for tooling and language servers.

### Fixed

- `fwd` re-exports now resolve against the dependency set correctly.
- x86-64 `imul` by a constant outside signed-imm32 range no longer truncates to
  the low 32 bits (silent miscompile of large-constant multiplies).
- A global `val` initialized from a constant cast no longer silently lowers to
  zero; a non-foldable global initializer is now a hard error.
- Several win64 codegen fixes (shadow space, variadic definitions, callee-saved
  register preservation) and a COFF weak-symbol round-trip.

## [1.0.0] - 2026-06-06

First stable release of Mach: a self-hosting, dependency-free native compiler
for the Mach systems programming language. The compiler builds its own source
to a byte-identical fixpoint and emits statically-linked x86-64 ELF directly,
with no external backend, assembler, or linker.

### Added

#### Compiler

- Self-hosting compiler that builds its own source to a byte-identical fixpoint.
- Direct x86-64 (Linux, SysV ABI) native code generation: lexer, parser,
  resolver, semantic analysis, an SSA mid-end, instruction selection,
  linear-scan register allocation, and ELF object/executable emission — with no
  LLVM and no external assembler or linker.
- Optimization pipeline: `mem2reg` (stack-to-SSA promotion), constant folding,
  dead-code elimination, function inlining, algebraic simplification, and local
  common-subexpression elimination. `-O0` runs the always-on subset
  (`mem2reg` / constant folding / DCE); `-O1` and `-O2` run the full pipeline.

#### Language

- Records (`rec`) and overlapping-layout unions (`uni`).
- Generics with bracket syntax (`T[U]`) and monomorphization.
- Compile-time evaluation: `$if` / `$or` branch selection, `$mach.*` target
  parameters, comptime value-parameter monomorphization, and value/layout
  intrinsics (`$size_of`, `$align_of`, `$offset_of`, …).
- Two cast operators: `::` (value conversion) and `:~` (same-size bit
  reinterpret).
- Pointers (including `nil`), slices, and fixed-size arrays.
- Error handling with `Result` and `Option`.
- Modules with `use`, module aliases, and `pub` visibility.
- Inline assembly (`asm`) and variadic functions.

#### Standard library

- `mach-std`: runtime, allocators, strings, collections, I/O, filesystem,
  formatting, OS/syscall bindings, and the core `Option` / `Result` types.

#### Tooling

- `mach` CLI: `build`, `test`, and `init`.
- Differential test harness (optimization-level and cross-compiler miscompile
  detection), a crash fuzzer, and a compiler compile-time benchmark harness.
