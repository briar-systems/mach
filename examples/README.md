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
| [`11_casts`](11_casts) | casts | the `::` value conversion vs the `:~` bit reinterpret, and how they differ on int<->float |

## Note on `full`

[`full`](full) is a syntax fixture that exercises every grammar form at once.
It is a reference for the language surface, not a runnable program against the
current compiler.

## Output

Examples print through `std.print`: `println` for a line of text and
`printlnf` for formatted output (`%d` signed, `%u` unsigned, `%s` string).
`10_lowlevel` is the exception — it writes its first line with a hand-issued
`write` syscall, since the raw syscall is the point of that example.
