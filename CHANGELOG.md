# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

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
