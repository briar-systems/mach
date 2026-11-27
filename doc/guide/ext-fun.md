# Foreign functions

`ext fun` declares a function that C code defines, and `ext var` a C variable. Both use the target's C calling convention and are resolved at link time.

## Declaring a function

```mach
ext fun strlen(s: *u8) u64;
pub ext fun getenv(name: *u8) *u8;
```

The declaration is a signature with no body, called like any other function. `pub` lets other modules call it.

Write each C type as the Mach type of the same size: `int` is `i32`, `double` is `f64`, `char *` is `*u8` and `void *` is `ptr`.

## Declaring a variable

```mach
ext var counter: i32;
ext val version: u32;
```

The storage lives in the C object, so there is no initializer. Only `ext var` can be assigned.

## Symbol names

```mach
#[symbol("write")]
ext fun libc_write(fd: i32, buf: *u8, n: u64) i64;
```

The linker looks for the declared name. `#[symbol]` states the C name when the Mach name differs. Write the plain C name on every target: the compiler adds darwin's leading underscore.

## C-variadic functions

```mach
ext fun printf(fmt: *u8, ...) i32;

fun show(c: u8, ratio: f32) {
    printf("%c %.2f\n", c::i32, ratio::f64);
}
```

A bare `...` after at least one fixed parameter declares a C-variadic function. A call may pass any number of further arguments.

C widens narrow variadic arguments and Mach converts nothing implicitly, so the call writes the cast. A missing cast is refused.

| Argument | Cast |
|---|---|
| An integer narrower than 32 bits, a character literal included | `::i32` or `::u32` |
| `f16` or `f32` | `::f64` |

Wider integers, `f64` and pointers pass as written.

Declare every C-variadic function with `...`, even when every call passes the same arguments: some targets pass variadic arguments differently. Only `ext fun` has this form. A Mach function takes a [variadic pack](functions.md#variadic-packs) instead.

## Linking the library

```mach
#[library("c")]
ext fun strlen(s: *u8) u64;
```

The definition comes from a library or object file named in the [manifest](manifest.md):

- A `[link.c]` entry describes one input: `source = "system"` and a `name` for a shared library found by name, or `source = "local"` and a `path` for an object or archive file. Its `sys`, `arch` and `abi` filters are required.
- An artifact lists the entries it links: `link = ["c"]`.
- `#[library("c")]` names the entry an import comes from, by its table key. Windows and darwin need it on every import. On linux it is checked and changes nothing.
- A linux target finds system libraries only when it states `env = "gnu"`.

`mach build . -l c` links a system library for one build without a manifest entry.

## A worked example

Start from `mach init ffi`, which fetches std, and replace its `mach.toml`:

```toml
[project]
id = "ffi"
version = "0.1.0"
mach = "^7"
src = "src"
work = "out/{target.name}/{profile.name}"

[target.linux-x86_64]
arch = "x86_64"
sys  = "linux"
abi  = "sysv64"
env  = "gnu"

[link.c]
source = "system"
name   = "c"
sys    = ["linux"]
arch   = ["*"]
abi    = ["*"]

[artifact.ffi]
kind = "bin"
entry = "bin/main.mach"
out = "{project.work}/bin/ffi{artifact.suffix}"
targets = ["*"]
link = ["c"]

[profile.debug]
optimize = false
debug = true
simd = "scalarize"

[dep.std]
git = "https://github.com/briar-systems/mach-std"
version = "^10.2.0"
```

And `src/bin/main.mach`:

```mach fragment
use std.runtime;

#[library("c")]
ext fun strlen(s: *u8) u64;

#[library("c")]
ext fun printf(fmt: *u8, ...) i32;

#[library("c")]
#[symbol("fflush")]
ext fun flush(stream: ptr) i32;

#[library("c")]
ext var stdout: ptr;

#[symbol("main")]
fun main(argc: i64, argv: **u8) i64 {
    val msg: *u8 = "hello from c";
    printf("\"%s\" is %lu bytes and starts with %c\n", msg, strlen(msg), msg[0]::i32);
    flush(stdout);
    ret 0;
}
```

```sh
mach build .
mach run .
```

```text
"hello from c" is 12 bytes and starts with h
```

std's runtime ends the process without calling C's `exit`, so the program flushes `stdout` itself.

The full rules are in the reference for [`ext fun`](../language/ext-fun.md) and [the manifest](../language/manifest.md).
