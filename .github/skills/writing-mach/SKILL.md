---
name: writing-mach
description: Use when writing, editing, or reviewing Mach (.mach) source files. Covers project/module structure, use/fwd imports and re-exports, the shadow-module pattern, all declaration forms (pub/ext, def, rec, uni, fun, ext fun, val/var, test), #[...] decorators, variadic packs, the type grammar (primitives, pointers, arrays, function types, the ^ secret qualifier), literals, operators and casts, statements (if/or, for, ret, brk/cnt, fin, blocks), docstring conventions, and the entrypoint / std.print idioms.
---

# Writing Mach

Mach is a low-level, explicitly-typed language: no type inference, no garbage
collection, no hidden control flow. Files use `.mach`. This skill is the fast
path for authoring correct Mach; `doc/language/` in the Mach repository is the
authoritative reference and wins on any disagreement.

## Hard rules — get these right

- **No type inference.** Every binding declares its type. `val x = 42;` is an
  error; write `val x: i64 = 42;`.
- **No compiler-known type aliases.** `bool`, `usize`, `str`, `char` are stdlib
  `def`s, not built-ins. Import them before use (`use std.types.bool.bool;`).
  `true`/`false` are stdlib `val`s (`1`/`0`); comparisons and logical operators
  produce `u8`.
- **Decorators are `#[...]` attributes** on the line(s) above a declaration:
  `#[symbol("main")]`, `#[inline]`, `#[align(64)]`. The backtick form was
  removed in v2.4.0 and is a migration error — never emit backticks.
- **Variadics are comptime packs.** A trailing `va: ...` parameter, consumed by
  `$each a in va`. There is no `va_list`/`va_start`/`va_arg` and the C-style
  bare `...` parameter is a removed-syntax error.
- **Strings are `*u8`, single-line.** `"hello"` is a pointer to null-terminated
  bytes. No fat-pointer string type (`str` is `def str: *char;`); no multi-line
  string literal — use `\n` escapes.
- **No tagged unions, no `match`.** A discriminated value is a `rec` carrying a
  discriminator plus a payload `uni`; consumers branch with `if`/`or`. This is
  exactly how stdlib `Result[T, E]` is built.
- **No compound assignment.** `+=` etc. do not exist; write `x = x + 1;`.
- **`fwd` is bare and always public** (no `pub fwd`). `ext fun` is the only
  body-less function form.

## Project and module structure

A project has a `mach.toml` at its root; `[project] id` roots every module
path. A file at `src/foo/bar.mach` in project `id = "myproj"` is the module
`myproj.foo.bar`. There is no `this.` self-prefix — always use the full
project-rooted path, including for sibling modules. A one-segment `use <id>;`
resolves only when that project declares a `[project] module` surface file
(e.g. a library `glfw` imported as `use glfw;`); `std` does not — always
import full `std.*` paths.

A file reads top-down: module docstring, `use`/`fwd` lines, declarations.

### `use` — private import

```mach
use std.types.size;             # binds module `size`; use as size.usize
use sz: std.types.size;         # module under alias; use as sz.usize
use std.types.size.usize;       # binds the symbol; use bare as usize
```

The resolver binds whatever the path ends at: a **module** (members reached
qualified) or a **symbol** (used bare). Importing a module does not pull its
members in unqualified. No splat, no `use foo.{a,b}` — one name per line. A
module `use`s every dependency it directly names, even ones reachable through
a re-export: the dependency graph is visible at the top of every file.

### `fwd` — public re-export

```mach
fwd impl.Point;                 # re-export as Point
fwd Pt: impl.Point;             # re-export as Pt
fwd impl.helpers;               # a module path re-exports the whole module
```

Mirrors `use` grammar; always publishes.

### Shadow-module pattern

A surface file `foo.mach` co-exists with directory `foo/` holding split
implementations. The surface `use`s each split and `fwd`s its public symbols;
consumers `use myproj.foo;` and never name the splits. Topical splits forward
everything unconditionally; multiplatform splits pick one impl per target:

```mach
$if ($mach.build.os == $mach.os.linux) {
    use impl: myproj.os.linux;
}
$or ($mach.build.os == $mach.os.windows) {
    use impl: myproj.os.windows;
}
$or {
    $error("myproj.os: unsupported target");
}

fwd impl.page_size;
```

## Entrypoint and output

The stdlib provides the platform `_start`, which calls whatever function
exports the linker symbol `main`. `use std.runtime;` is required to link it in
even though nothing references it by name:

```mach
use std.runtime;
use std.print;

#[symbol("main")]
fun main(argc: i64, argv: **u8) i64 {
    print.println("hello, mach");
    ret 0;
}
```

`use std.print;` binds the leaf module `print` (`std` itself is not in scope).
It exposes `print`/`println` (stdout), `eprint`/`eprintln` (stderr), and the
format family `printf`/`printlnf`/`eprintf`/`eprintlnf` — pack-variadic, with
`{}` holes filled in argument order plus `{:x}`-style specs (`{:X}`, `{:c}`,
`{:5}`, `{:<5}`, `{:08x}`; `{{`/`}}` for literal braces). All return
`Result[usize, str]`.

```mach
print.printlnf("built {} in {}ms", name, elapsed);
```

## Declarations

Modifiers: `pub` (public surface; without it a declaration is file-private)
and `ext` (C-ABI external, functions only). Both apply to `fun`, `rec`, `uni`,
`def`, `val`, `var`.

### `def` — type alias

```mach
pub def Age:   i64;
pub def BinOp: fun(i64, i64) i64;
```

Aliases name any type; alias and underlying type are interchangeable.

### `rec` / `uni`

```mach
pub rec Point { x: i64; y: i64; }
pub rec Pair[T, U] { left: T; right: U; }      # generic

pub uni Number { i: i64; f: f64; }              # fields overlap; size of largest
```

The compiler does not track which `uni` field is live. The sum-type idiom
(stdlib `Result[T, E]` verbatim):

```mach
pub rec Result[T, E] {
    tag:   bool;
    value: uni { ok: T; err: E; };
}
```

Packed layout is not available; `#[align(N)]` raises a type's or global's
alignment.

### `fun`

```mach
pub fun add(a: i64, b: i64) i64 { ret a + b; }
pub fun identity[T](value: T) T { ret value; }  # generic; call: identity[i64](42)
pub fun load($order: u8, p: *i64) i64 { ... }   # comptime value param
pub fun sum(va: ...) i64 {                      # variadic pack
    var t: i64 = 0;
    $each a in va { t = t + a; }
    ret t;
}
```

- Generic params `[T]` take types only, no constraints; monomorphized per
  instantiation; call sites always supply the types explicitly.
- `$name: T` params must receive a comptime-evaluable argument; the body
  branches on them with `$if`, monomorphized per distinct value. Referenced
  bare (`order`, not `$order`) inside the body. See the **mach-comptime**
  skill for the full rules.
- A pack (`va: ...`) must be the last parameter. `va.len` folds to the element
  count; `g(va...)` forwards the whole pack to another pack-tailed function.
  Pack functions are source-level only — no stable ABI symbol, so no `ext` and
  no function pointer to one.

### `ext fun`

```mach
#[symbol("write")]
pub ext fun libc_write(fd: i64, buf: *u8, n: i64) i64;
```

Body-less, ends in `;`, C ABI is the contract. Provide the definition at link
time (`mach build . -l c`, manifest `libs`, or an explicit `.o`/`.a`/`.so`).
On PE targets pin imports to their DLL with `#[library("ws2_32.dll")]`.

### `val` / `var`

```mach
val pi: f64 = 3.14159;          # immutable; initializer required
var counter: i64 = 0;
var buf: [256]u8;               # default-initialized to zero
```

Work at module top level (`pub` exports) and in function bodies.

### `test`

Tests are declarations, inline next to the code they cover, in any module:

```mach
test "point: add is commutative" {
    if (add(1, 2) != add(2, 1)) { ret 1; }
    ret 0;
}
```

The label is a required string literal; convention is `"topic: behavior"`.
The body checks like an `i32` function: `ret 0` (or falling off the end)
passes, any non-zero return fails. There are no assertion builtins — return
early on a failed check. `mach test` collects every test in the project
(dependencies excluded unless `--include-deps`), builds one executable per
test, and reports per-module; `--filter <pattern>` narrows, `--list`
enumerates.

## Types

Compiler-seeded primitives (the complete set): `u8 u16 u32 u64`,
`i8 i16 i32 i64`, `f32 f64`, and the untyped pointer `ptr`. SIMD vector names
(`f32x4` …) are a planned design and **do not resolve as types yet**.

```mach
*T                  # pointer          ?x address-of, @p dereference
[N]T  [N][M]T       # array            val a: [4]i64 = [4]i64{1, 2, 3, 4};
fun(T1, T2) R       # function pointer val op: BinOp = add;  op(2, 3)
^T                  # secret-qualified (see below)
```

Pointers index like arrays: `p[i]` reads the i-th element (this is how `str`
is walked). A `fun(...)` type may carry a trailing `...` for FFI only.

### `^` — the secret qualifier

`^T` marks data as secret for the constant-time discipline. Secrets may move
and be stored but may never reach an observable position: a branch or loop
condition, the left operand of `&&`/`||`, a memory index, or a `/`/`%`
operand — each is a compile error. Public flows up to secret implicitly; the
**only** downgrade is the explicit strip cast `x:^` (or `x:^T`). Any operation
with a secret operand yields a secret result; `uni` variants must agree on
secrecy; a secret-welded pointer (`*^T`) cannot be erased to `ptr`. See
`doc/language/secrecy.md` before touching crypto code.

## Literals

| Form | Example | Notes |
|---|---|---|
| Int (dec/hex/bin/oct) | `42` `0xDEAD` `0b1010` `0o755` `1_000_000` | untyped until context |
| Typed suffix | `7i64` `255u8` `2.5f64` | use when context doesn't constrain |
| Float | `1.5` `1.5e10` | `.` must be followed by a digit |
| Char | `'M'` | `u8`; escapes `\n \t \r \\ \' \0 \xHH` |
| String | `"hi\n"` | `*u8`, null-terminated, single-line; adds `\"` |
| `nil` | `nil` | null address; coerces to any pointer **or function** type |

Untyped literals are *checked* against the declared type, never used to infer
it. `nil` with no context types as `*u8`; `var cb: fun(u32) = nil;` is legal.

## Operators and casts

- Arithmetic `+ - * / %` (ints and floats; `%` truncated remainder, sign of
  the dividend, floats included). Bitwise `& | ^ ~ << >>` (ints).
- Comparison `== != < > <= >=` → `u8`. Mixed int signedness/width compares
  **mathematical values** (a negative `i64` < any `u64`). Int vs float
  comparison is a compile error — cast explicitly.
- Logical `&& || !` — short-circuit, `u8` operands and result.
- Pointer: `?expr` address-of, `@p` dereference (`@p = x;` writes through).
- Casts (postfix): `expr::T` value conversion (resize, int↔float);
  `expr:~T` bit reinterpret (same byte size required); `expr:^` strips the
  secret qualifier (the only one that can).
- Assignment `=` is an expression form used in statement position;
  right-associative, lowest precedence.

Precedence is C-family: `* / %` > `+ -` > `<< >>` > relational > equality >
`&` > `^` > `|` > `&&` > `||` > `=`. Note bitwise binds looser than
comparison, as in C — parenthesize `(a & b) != 0`.

## Statements

Statements end with `;` unless they end with a block. Bodies are always
blocks — there is no brace-less form.

```mach
if (cond) { ... } or (cond) { ... } or { ... }   # `or {}` is the catch-all
for (cond) { ... }                               # condition loop
for { ... }                                      # no condition: infinite loop
ret expr;   ret;                                 # value / void return
brk;   cnt;                                      # break / continue enclosing for
fin { ... }                                      # run at scope exit, reverse order
{ ... }                                          # bare block = new scope
```

`fin` requires a block — `fin stmt;` is rejected. There is no for-each; loop a
counter or a pointer cursor. `$if`/`$or` (comptime) and `$each` (comptime
unroll) also appear in statement position — see the **mach-comptime** skill.

## Stdlib idioms

- `Result[T, E]` with `ok`/`err` constructors and `is_ok`/`is_err`/
  `unwrap_ok`/`unwrap_err`; `Option[T]` with `some`/`none`/`is_some`/`unwrap`.
  Generic arguments are always explicit at call sites:

```mach
val r: Result[usize, str] = parse(s);
if (is_err[usize, str](r)) { ret r; }
val n: usize = unwrap_ok[usize, str](r);
```

- `str` is `*char` (null-terminated); `std.types.string` provides `str_len`,
  comparison, and `StrView { data, len }` for length-aware slices.
- No methods, no UFCS: everything is a free function taking an explicit
  receiver (usually a pointer), reached through the module alias —
  `vec.push[T](?v, x)`, not `v.push(x)`.
- Naming: `snake_case` functions/bindings, `PascalCase` types,
  `SCREAMING_SNAKE` constants. Indent 4 spaces; align `:` columns in field
  blocks, import groups, and consecutive `val`/`var` runs when it aids
  scanning.

## Docstrings

`#` comments immediately above a declaration: a bare lowercase summary line
(no `name:` prefix, no trailing period), then — only when there are elements
to document — a `# ---` separator and one aligned component line per
parameter/field/return:

```mach
# read the wall-clock time
# ---
# out: pointer to Timespec to populate
# ret: 0 on success, negative errno on failure
pub fun realtime(out: *Timespec) i64 { ... }
```

Component identifiers: parameter name, `$name`, `[T]`, `ret`, field name,
variant name. Summary-only docstrings omit the separator. Every file opens
with a module docstring (summary, optional paragraphs separated by blank `#`
lines) before any `use`/`fwd`/decorator. Decorators sit between the docstring
and the declaration. Document every `pub` entity.

Much existing code still carries the older `# name: summary.` form (leading
identifier, trailing period) from before the convention tightened — when
editing such a file, match its surrounding style; use the bare form in new
files.

## Comptime channel — brief

`$` is the compiler-owned comptime channel: `$mach.*`/`$project.*`/`$bin.*`
reads, the closed intrinsic set (`$size_of`, `$type_of`, `$fields`, `$error`,
…), `$if`/`$or` conditional compilation, and `$each` unrolls. `#[...]`
decorators (`symbol`, `library`, `inline`, `align`, `section`) are a separate
closed set. Full rules: the **mach-comptime** skill. Inline `asm`: the
**mach-lowlevel** skill.

## Reference

The authoritative per-feature reference lives in the Mach repository under
[`doc/language/`](https://github.com/briar-systems/mach/tree/dev/doc/language),
including the full EBNF in `grammar.md`. When this skill and the reference
disagree, the reference wins.
