---
name: mach-comptime
description: Use when writing or reviewing Mach code that touches the comptime channel or decorators: the $ namespace ($mach.*, $project.*, $bin.*), conditional compilation with $if/$or, #[...] codegen decorators (symbol/library/inline/align/section), intrinsics ($size_of/$align_of/$offset_of/$type_of/$fields/$error/$assert), $each unrolls with field projection v.[f], comptime function parameters ($name: T), and variadic packs.
---

# Mach comptime channel

`$` opens the compiler-owned comptime channel — read-only: it *selects and
expands*, never executes or mutates. The parser disambiguates by shape:

| Shape | Meaning |
|---|---|
| `$mach.*` / `$project.*` / `$bin.*` | read a compiler-owned tree (roots are reserved) |
| `$sym(args)` | comptime call — the closed intrinsic set |
| `$if` / `$or` | comptime control flow |
| `$each x in SEQ { }` | comptime unroll over `$fields(T)` or a variadic pack |

A bare `$ident` (`$mode`, `$foo`) is none of these and is rejected: comptime
*parameters* are referenced without `$`; comptime *paths* are rooted.
Per-declaration codegen directives are `#[...]` decorators (below), not `$`
shapes — the `$sym.attr = value;` setters were removed in v2.0.0 and the
backtick decorators in v2.4.0.

## `$mach.*` — compiler and build state

All reads, all comptime constants, closed tree:

```mach
$mach.build.os / .arch / .abi / .mode   # live; compare against the tag tables
$mach.build.pointer_width               # live; integer byte count (8 on 64-bit)
$mach.build.pie                         # live; 1 when building position-independent
$mach.build.<NAME>                      # live; manifest `defines` lookup (see below)
$mach.version / .major / .minor / .patch    # live; compiler version
$mach.compiler.name / .version              # live

$mach.os.linux    .darwin   .windows  .freestanding
$mach.arch.x86_64 .aarch64  .riscv64
$mach.abi.sysv    .win64    .aapcs64  .lp64
$mach.mode.debug  .release
```

`$mach.build.{timestamp,host,git.*}`, `$mach.project.*`, and `$mach.source.*`
are reserved stubs — reading one is a compile error. The tag tables are closed;
an unrecognized tag is a compile error, never a silent fold.

Tag comparison is path-value — plain `==`, no `.id` suffix, no unwrap:

```mach
$if ($mach.build.os == $mach.os.linux) { ... }
$if ($mach.build.arch == $mach.arch.riscv64) { ... }
```

A `$mach.build.<NAME>` that names none of the reserved facts resolves to the
manifest's per-target comptime `defines` (`defines = ["TRACING", "DEPTH=4"]`
in a `[target.*]` stanza); an undeclared name is a loud error.

A `$mach.*` read can fold into a runtime binding — the binding still declares
its type:

```mach
pub val IS_LINUX: u8  = $mach.build.os == $mach.os.linux;
pub val COMPILER: *u8 = $mach.compiler.name;
```

## `$project.*` / `$bin.*` — manifest state

```mach
$project.id / .version / .name / .description   # [project] metadata
$project.version.major / .minor / .patch        # folded integer components
$project.target.os / .arch / .abi               # the selected target's declared *strings*
$bin.name                                       # the artifact being built
```

`$project.target.*` carries the manifest's string spellings (`"linux"`,
`"x86_64"`) — distinct from `$mach.build.*`'s numeric tags. A field the
manifest does not declare is reported unavailable, not folded to `""`.

## Decorators — `#[...]`

Codegen directives on the line(s) above a declaration (after the docstring).
One clause each, stackable on one line or several; they attach only to the
immediately following declaration. Closed set:

| Decorator | Applies to | Argument | Purpose |
|---|---|---|---|
| `#[symbol("name")]` | fun, ext fun, val/var | string | linker name override |
| `#[library("x.dll")]` | ext fun | string | PE import DLL pin |
| `#[inline]` | fun | none | force inlining |
| `#[align(expr)]` | val/var, rec/uni | comptime int | alignment override |
| `#[section(".name")]` | fun, ext fun, val/var | string | object section placement |

```mach
#[symbol("read")]
pub ext fun sys_read(fd: i32, buf: *u8, n: u64) i64;

#[align(64)] #[section(".hot")]
pub var cache_line: u8 = 0;
```

`align` takes any comptime expression (`#[align($align_of(T))]`). A `library`
pin must name a DLL in the target's `libs` set — pinning to an absent library
is a link error; on ELF the pin is validated but has no binary effect. Beware:
a line comment starting `#[` with no space parses as a decorator — write
`# [...]` in prose comments.

## Intrinsics — `$name(args)`

Closed compiler-shipped set. New ones require a compiler change; runtime
instruction emitters (`trap`, `fence`, `pause`) are **not** intrinsics — they
are stdlib functions with per-arch `asm` bodies.

### Layout values

```mach
$size_of(T)   $align_of(T)   $offset_of(T, field)
```

Comptime unsigned integers; the binding declares the storage type
(`pub val POINT_SIZE: i64 = $size_of(Point);`).

### `$type_of(expr)` — type dispatch

Produces a comptime type value, comparable with `==`/`!=` against a type name
inside `$if`. Dead arms are **pruned before type-checking**, so each arm can
use the value at its own concrete type with no cast:

```mach
$if ($type_of(arg) == str)  { write_str(w, arg); }
$or ($type_of(arg) == i64)  { write_i64(w, arg); }
$or { $error("no writer for this argument type"); }
```

This is the idiom inside `$each` bodies for per-element dispatch (stdlib
`vformat` is built exactly this way).

### `$fields(T)` and projection

`$fields(T)` yields a comptime sequence of field descriptors for a `rec`/`uni`,
consumed by `$each`. Each descriptor `f` carries `f.name` (`*u8`), `f.type`
(type value), `f.offset` (integer); `v.[f]` projects the concrete field off an
instance — an lvalue, re-typed per iteration:

```mach
fun sum_fields(p: Pair) i64 {
    var t: i64 = 0;
    $each f in $fields(Pair) { t = t + p.[f]; }
    ret t;
}
```

### `$each` — comptime unroll

Statement-scope only. The body is spliced once per element — not a runtime
loop. Two sequence forms: `$each f in $fields(T)` and `$each a in va` (packs —
see below). Enclosing runtime variables thread across the unrolled copies;
nesting is allowed.

### Diagnostics

`$error("msg")` fails the build when **reached** — unconditional position or a
selected `$if`/`$or` arm. A `$error` in a discarded arm never fires, making it
the natural exhaustiveness fallback for target and type dispatch. Valid at
declaration and statement scope. **`$assert` parses but is not yet evaluated**
— write `$if (!cond) { $error("msg"); }` instead.

## `$if` / `$or` — conditional compilation

```mach
$if (cond) { ... }
$or (cond) { ... }
$or { ... }             # comptime else — no condition; there is no `$or $if`
```

Only the taken branch compiles: discarded branches are not resolved,
type-checked, or emitted — names inside them are never looked up. This is what
makes per-arch `asm` and per-OS `use` safe. Valid at declaration scope
(selecting `use`/`fwd`/declarations) and statement scope.

Conditions must be comptime: `$mach.*`/`$project.*` reads, comptime constants
(`pub val`), comptime parameters, `$type_of` comparisons. Comptime comparison
follows the runtime rules — mathematical values, mixed signedness fine;
overflow in comptime arithmetic is a compile error, not a wrap.

## Comptime function parameters — `$name: T`

A `$`-marked parameter in the regular list must receive a comptime-evaluable
argument (literal, `pub val` constant — including imported ones — or another
comptime parameter). The compiler **monomorphizes the body per distinct
value**; each instance compiles only the arms its value selects. Referenced
bare inside the body:

```mach
val MODE_DOUBLE: u8 = 0;
val MODE_SQUARE: u8 = 1;

pub fun apply($mode: u8, n: i64) i64 {
    $if (mode == MODE_DOUBLE) { ret n + n; }
    $or (mode == MODE_SQUARE) { ret n * n; }
    ret 0;
}
```

Rules that bite:

- Unlike target-gated `$if`, **all** arms of a parameter-gated `$if` are
  resolved and type-checked structurally; only the selected arm is emitted per
  instance. Each arm must be independently valid.
- No storage: `?mode` is rejected; the parameter is stripped from the lowered
  signature and ABI.
- A comptime-parameter function is a template, not a value — it cannot be
  assigned, passed, or compared, only called.
- Not yet combinable with generic type params (`fun f[T]($m: u8, ...)` is a
  clear diagnostic).
- Function parameters only — never record fields.

## Variadic packs (comptime-adjacent)

`va: ...` (trailing parameter) collects call-site arguments into a comptime
sequence; monomorphized per distinct type-list. `$each a in va` consumes it
(heterogeneous packs work — dispatch with `$type_of` per element), `va.len`
folds to the count, `g(va...)` forwards the whole pack (sole trailing argument
only; no partial forward). No runtime `va_list` exists. Pack functions have no
stable ABI symbol: not `ext`, not addressable. See `doc/language/variadics.md`.

## Pitfalls

- **Backticks and `$sym.attr =` setters are removed syntax.** Decorators are
  `#[...]` only.
- **No `$or $if`** — the else arm is bare `$or { }`.
- **Closed sets everywhere.** Intrinsics, decorators, `$mach.*` tags. Don't
  invent members.
- **`$assert` doesn't fire yet**; `$error` does.
- **Comptime params are referenced bare** (`order`, not `$order`) and their
  `$if` arms all must type-check.
- **`$if` vs `if`.** `$if` selects at compile time and discards the rest;
  `if` emits a runtime branch. Never use `$if` to fake reflection over code
  that must exist at runtime.
- **Not in the channel:** no `$<Type>.*` reflection, no comptime function
  definitions, no comptime loops beyond `$each` over fields/packs.

## Reference

Authoritative docs in the Mach repository under
[`doc/language/`](https://github.com/briar-systems/mach/tree/dev/doc/language):
`comptime.md`, `comptime-mach.md`, `comptime-intrinsics.md`,
`comptime-control.md`, `decorators.md`, `variadics.md`. The reference wins on
any disagreement.
