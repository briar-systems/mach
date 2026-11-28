# Decorators

A decorator is `#[name]` or `#[name(args)]` written before a declaration. It states how that one declaration is linked, compiled, laid out or diagnosed.

## Syntax

```mach
#[inline("always")]
#[symbol("app_checksum")]
fun checksum(a: u32, b: u32) u32 {
    ret a ^ b;
}

#[align(64)]
#[section(".counters")]
var hits: u64 = 0;
```

- A decorator applies to the next declaration only, and appears at most once on it. Several may share a line.
- An argument is a constant: an integer, or a string for a name, a mode or a path. A `val` works, so a `$if` can choose a symbol per target.
- An unknown name is refused, and so is a decorator its declaration or its target cannot take, as `naked` on wasm.
- A comment that begins `#[` is read as a decorator. Write `# [`.

## The set

| Decorator | On | Effect |
|---|---|---|
| **Linking and symbols** | | |
| `symbol("name")` | `fun`, `val`, `var`, `ext` | The linker name, in place of the mangled module path |
| `export` | `pub` `fun`, `val`, `var` | Exports from a shared library. `pub` alone never does |
| `library("key")` | `ext` | Names the `[link.key]` entry the import comes from. Required on Windows and macOS |
| `weak("name")` | `fun` | A default that a strong definition of the name replaces at link |
| `section(".name")` | `fun`, `val`, `var` | Places it in a named object section |
| `at(offset)` | module `val`, `var` | Places it at a fixed offset in a freestanding image |
| **Code generation** | | |
| `inline("always")`, `inline("never")` | `fun` | Forces or forbids inlining |
| `vectorize("never")` | `fun` | Keeps its loops scalar |
| `naked` | `fun` | No prologue and no return. The body is `asm` only |
| `noreturn` | `fun`, `ext fun`, an `asm` statement | Control never comes back |
| `extension("avx2")` | `fun` | Admits instruction-set extensions the target does not select. The caller checks the processor |
| `constant_time("soft")`, `constant_time("hard")` | `fun`, a `def` of a function type | A constant-time boundary. See [secrecy](secrecy.md) |
| `intrinsic("mirl", "ir", "clz")` | bodyless `fun` | A call is that operation: `popcnt`, `clz`, `ctz`, `bswap` |
| `gpu.*`, `intrinsic`, `handle` | Shader declarations | The shader interface and the target's instructions and types. See [shaders](shaders.md) |
| **Layout and data** | | |
| `align(N)` | `fun`, `rec`, `uni`, `tag`, `val`, `var` | Raises the alignment to `N` bytes |
| `packed` | `rec`, `uni`, `tag` | No padding between fields or at the end |
| `volatile` | `rec`, `uni`, `tag` | Every read and write of the type is kept, in order |
| `thread` | module `var` | One copy per thread |
| `embed("path")` | `val` of `[_]u8` or `[N]u8` | The file's bytes are the initializer |
| `abi_type("va_list")` | `def` with no type | C's `va_list`. See [ext fun](ext-fun.md) |
| **Testing and diagnostics** | | |
| `testing` | `fun`, `val`, `var`, types, `use`, `fwd` | Exists only for tests, and only a `test` or another `testing` declaration may name it |
| `expect("key")` | `fun`, `val`, `var`, `rec`, `uni`, `tag`, `test` | Acknowledges a warning raised inside the declaration |
| `deprecated`, `deprecated("msg")` | `fun`, `val`, `var`, types, `use`, `fwd`, a tag case | Warns at each use from another module |

## Symbols and exports

```mach
#[symbol("write")]
ext fun c_write(fd: i32, buf: *u8, n: u64) i64;

#[export]
#[symbol("shapes_hello")]
pub fun hello() i64 {
    ret c_write(1, "hello\n", 6);
}
```

`symbol` gives the name the linker sees, on an import and on a definition alike. Mach code calls `c_write` and `hello`, and a C program linked against this library calls `shapes_hello`. An executable's entry is the function with `#[symbol("main")]`.

## Inlining

```mach
#[inline("always")]
fun twice(n: i64) i64 {
    ret n * 2;
}

#[inline("never")]
fun fail(code: i64) i64 {
    ret code * 100;
}
```

An optimizing build inlines small functions on its own. `"always"` expands the function at each direct call whatever its size, and `"never"` keeps every call a call. A build that does not optimize inlines nothing.

## Layout

```mach
#[packed]
rec Header {
    magic:  u8;
    length: u32;
    flags:  u16;
}

#[align(64)]
var counter: u64 = 0;
```

`Header` is 7 bytes with `length` at offset 1. Without `packed` it is 12 bytes with `length` at offset 4.

- Taking the address of a packed field is refused, because the pointer's type would promise an alignment the field does not have. Copy the field into a local, or take the address of the whole record.
- `align` only raises. On a type it holds for every value of that type, locals included.
- The two combine: `packed` removes the padding, and `align` rounds the record's size up to its alignment.

## Embedded files

```mach fragment
#[embed("assets/banner.txt")]
val BANNER: [_]u8;

#[embed("boot/sector.bin")]
val SECTOR: [512]u8;
```

The file's bytes are the `val`, placed in read-only data at build time. The declaration takes no initializer.

- The path is relative to the directory of the source file that declares it, and must stay inside the project.
- `[_]u8` takes its length from the file. `$length_of(BANNER)` reads it back.
- `[N]u8` pins the size, and a file of any other size fails the build.

The full rules are in the reference for [decorators](../language/decorators.md).
