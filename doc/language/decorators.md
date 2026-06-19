# Backtick decorators

A decorator is a codegen directive attached to a declaration. It expresses
metadata that influences how the compiler emits the symbol: its linker name,
alignment, section placement, inlining, or PE import routing.

Decorators are **codegen-only**. Visibility (`pub` / `ext`) is separate and
unaffected by decorators.

## Grammar

```
`symbol("name")`    # linker name override
`library("dep")`    # PE import routing (ext only)
`inline`            # force inlining (no arguments)
`align(expr)`       # alignment; expr is a comptime integer
`section(".name")`  # place in a named object section
```

Decorators appear **before** the declaration they target, one per line or
space-separated on the same line. They attach to the immediately following
declaration only and do not bleed across declarations.

```mach
`inline`
`symbol("big")`
fun big(a: i64, b: i64) i64 { ... }

`align(64)` `symbol("g_lit64")`
pub var g_lit64: u8 = 7;
```

Each directive is wrapped in its own backtick pair: `` `name` `` for a bare
flag or `` `name(args)` `` for a directive that takes arguments. Arguments are
comptime expressions.

## Directives

### `symbol(str)` — linker name

Overrides the emitted or imported symbol name. Applies to functions and
globals.

```mach
`symbol("main")`
fun entry(argc: i64, argv: **u8) i64 { ... }

`symbol("write")`
ext fun libc_write(fd: i64, buf: *u8, n: i64) i64;
```

Without `symbol`, the compiler mangles the Mach name. `symbol` gives the
exact name the linker sees.

### `library(str)` — PE import routing

Pins an `ext` import to a specific DLL in the link's dependency set. Applies
to `ext` functions only.

```mach
`library("ws2_32.dll")` `symbol("WSAStartup")`
ext fun wsa_startup(ver: u16, data: *u8) i32;
```

- The named DLL must appear in the manifest's `[os.*].libs` for the target OS
  (e.g. `[os.windows].libs = ["kernel32.dll", "ws2_32.dll", ...]`). Pinning
  to an absent library is a link error, never a silent fallback.
- Without `library`, an unattributed PE import binds to the **first** declared
  dependency. Every import that belongs to a different DLL needs an explicit
  `library` pin.
- On ELF (Linux) the loader resolves imports by global search, so `library`
  has no effect on the emitted binary; the value is still validated against
  the link's dependency set.
- `library` composes with `symbol`: the import is emitted under the renamed
  symbol within the named DLL.

### `inline` — force inlining

Marks a function for inlining at every call site, overriding the compiler's
size- and use-count heuristics. Applies to functions only; takes no arguments.

```mach
`inline`
fun fast_path(x: i64) i64 { ret x * 2; }
```

### `align(expr)` — alignment override

Sets the alignment of a global variable or a record/union type. `expr` must
be a comptime integer — either a literal or a comptime expression such as
`$size_of(T)` or `$align_of(T)`.

```mach
`align(64)`
pub var cache_line: u8 = 0;

`align($size_of(Pair))`
pub var g_cmp: u8 = 0;

`align(32)`
rec Over { a: u8; }
```

- On a `var` / `val`, sets the global's section alignment and address
  alignment.
- On a `rec` or `uni`, sets the type's own alignment, which is then inherited
  by any global of that type.
- `align` does not apply to `def` aliases (transparent, no layout of their
  own).

### `section(str)` — object section placement

Places a function or global variable in a named section instead of the
default `.text` / `.data`.

```mach
`section(".hottext")` `symbol("f_hot")`
fun f_hot(x: i64) i64 { ret x + 1; }

`section(".machsec")` `symbol("g_sec")`
pub var g_sec: u64 = 100;
```

The named section is created if absent. Cross-section calls and accesses use
ordinary relocations.

## Applicability

| Directive  | `fun` | `ext fun` | `val` / `var` | `rec` / `uni` |
|------------|:-----:|:---------:|:-------------:|:-------------:|
| `symbol`   |  yes  |    yes    |      yes      |      no       |
| `library`  |  no   |    yes    |      no       |      no       |
| `inline`   |  yes  |    no     |      no       |      no       |
| `align`    |  no   |    no     |      yes      |      yes      |
| `section`  |  yes  |    yes    |      yes      |      no       |

The set is closed. New directives require a compiler change.

## See also

- [ext-fun.md](ext-fun.md) — `ext` imports, `library` and `symbol` use cases
- [visibility.md](visibility.md) — `pub` / `ext` visibility (not decorator-controlled)
- [comptime-intrinsics.md](comptime-intrinsics.md) — `$size_of` / `$align_of` as `align` arguments
