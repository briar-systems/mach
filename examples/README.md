# Examples

A progression of small, standalone Mach programs, each focused on a single
language or standard-library concept. Every example is its own project with a
`mach.toml` and a `main.mach`, and each program prints what it demonstrates
when run.

## Building and running

From inside an example directory:

```sh
mach build       # compile to out/linux/bin/<name>
mach run         # build and execute
```

Each example vendors the standard library through `dep/mach-std`, a symlink to
the repository's `dep/mach-std`. The build resolves it as a local dependency.

## Index

| Example | Concept | Demonstrates |
| --- | --- | --- |
| [`01_hello`](01_hello) | hello world | the entrypoint contract (`use std.runtime;`, `$main.symbol`, `main`) and writing a line to stdout |
| [`02_control_flow`](02_control_flow) | control flow | the `if {} or {}` chain, the `for (cond)` loop, and `brk` / `cnt` |
| [`03_functions`](03_functions) | functions and recursion | plain functions, direct recursion, mutual recursion, a module constant |
| [`04_structs_unions`](04_structs_unions) | aggregates | records (`rec`), overlapping unions (`uni`), and a tagged variant |
| [`05_generics`](05_generics) | generics | generic functions `fun f[T](...)` and a generic record `rec Pair[A, B]` |
| [`06_comptime`](06_comptime) | compile-time | `$if` / `$or` target dispatch, `$size_of` / `$align_of`, a `$mode` parameter |
| [`07_errors`](07_errors) | error handling | `Result` (ok / err) and `Option` (some / none) with unwrap helpers |
| [`08_strings`](08_strings) | strings | the `str` type, `str_len` and friends, and byte-by-byte manipulation |
| [`09_collections`](09_collections) | collections | the standard hash `Map` and `Set` over an allocator |
| [`10_lowlevel`](10_lowlevel) | low level | an `asm x86_64` block and a hand-issued Linux `write` syscall |

## Note on `full`

[`full`](full) is a syntax fixture that exercises every grammar form at once.
It is a reference for the language surface, not a runnable program against the
current compiler.

## Output helpers

Several examples include a small local `out` / `out_num` pair that writes to
stdout via `std.system.os.write`. They are written out in full in each file so
every example reads top to bottom without cross-references.
