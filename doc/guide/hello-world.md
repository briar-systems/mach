# Hello world

A first program takes three commands: `mach init` writes a project, `mach build` compiles it and `mach run` runs it.

## Creating the project

```sh
mach init hello
cd hello
```

```text
created hello
  files  3
  path   hello/
realized std
  version  ^10
  commit   378ad6bb40a8
  path     hello/dep/std
```

`mach init` writes a manifest and an entry file, and checks out the standard library under `dep/std`.

## The program

This is `src/bin/main.mach` as `mach init` writes it:

```mach
use std.print;
use std.runtime;

#[symbol("main")]
fun main(argc: i64, argv: **u8) i64 {
    print.println("Hello, World!");
    ret 0;
}
```

- `use std.print;` binds the module as `print`, so its functions are called as `print.println`.
- `use std.runtime;` links the code that starts the process and calls your entry function. The program does not link without it.
- `#[symbol("main")]` is what makes this function the entry. The runtime calls the symbol `main`, and the function's own name can be anything.
- The entry takes the argument count and the argument strings, and its result is the process exit code.

## Building and running

Every command takes the project directory, here `.`:

```sh
mach build .
mach run .
```

```text
built hello
  target  linux-x86_64 debug
  time    365ms
  output  out/linux-x86_64/debug/bin/hello
Hello, World!
```

`mach run` runs what the last build wrote and never builds, so build again after an edit. Arguments for the program follow `--`, as in `mach run . -- a b`.

## Printing

```mach fragment
print.println("Hello, World!");
print.printf("{} arguments\n", argc);
print.printlnf("{} + {} = {}", 2, 3, 2 + 3);
```

| Function | Writes |
|---|---|
| `println(s)` | The string and a newline |
| `printf(fmt, ...)` | The format, each `{}` filled by the next argument |
| `printlnf(fmt, ...)` | The same, and a newline |

`eprintln`, `eprintf` and `eprintlnf` write to standard error. A bare number is printed as an `i64` or `f64`, so give a literal of any other type its suffix, as in `255u8`.

Next is [Project layout](project-layout.md). The full rules for an entry are in the reference for [files](../language/files.md).
