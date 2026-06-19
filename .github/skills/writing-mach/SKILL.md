---
name: writing-mach
description: Use when writing, editing, or reviewing Mach (.mach) source files. Covers project/module structure, use/fwd imports and re-exports, the shadow-module pattern, all declaration forms (pub/ext, def, rec, uni, fun, ext fun, val/var), the type grammar (primitives, SIMD vectors, pointers, arrays, function types), literals, operators, statements (if/or, for, ret, brk/cnt, fin, blocks), expressions, docstring conventions, and the executable entrypoint and std.print output idioms.
---

# Writing Mach

Mach is a low-level, explicitly-typed language. No type inference, no garbage
collection, no hidden control flow. Files use the `.mach` extension. This skill
is the fast path for authoring correct Mach; the language reference (see the
Reference section) is the exhaustive source of truth.

## Hard rules — get these right

- **No type inference.** Every binding declares its type. `val x = 42;` is an
  error; write `val x: i64 = 42;`.
- **No compiler-known type aliases.** `bool`, `usize`, `str`, etc. are *stdlib*
  `def`s, not built-ins. The compiler ships only `ptr`, `va_list`, and the
  numeric/SIMD family. Import the alias from stdlib before using it. `bool` is
  `def bool: u8;`; `true`/`false` are stdlib `val`s (`1`/`0`). Comparison and
  logical operators produce `u8`.
- **Strings are `*u8`.** `"hello"` produces a `*u8` pointing at null-terminated
  bytes. There is no fat-pointer string type — `std.types.string.str` is itself
  `def str: *char;` (a null-terminated pointer, not length-aware). The
  length-aware view is the stdlib record `std.types.string.StrView`
  (`{ data: *char; len: usize; }`). Backtick `` ` `` has no role: the lexer
  treats it as an unexpected character (a lex error) — never emit it.
- **`fwd` is bare and always public.** No `pub fwd`. `ext fun` is the *only*
  body-less function form — there are no forward declarations of regular
  functions.
- **No tagged unions, no `match`.** Discriminated values are a `rec` carrying a
  discriminator plus a payload `uni`; consumers branch with `if`/`or`.
- **`$or` has no condition for the else arm.** There is no `$or $if`. Comptime
  control is `$if (C) {}` / `$or (C) {}` / `$or {}`.

## File and module structure

A project has a `mach.toml` at its root whose `[project] id` is the root of
every module path. A file at `src/foo/bar.mach` in a project with `id = "myproj"`
is the module `myproj.foo.bar`. Path separator is `.`. There is **no `this.`
self-prefix** — always reference modules by full project-rooted path.

Entrypoints by convention: `lib.mach` (library, no executable), `main.mach`
(executable). The dominant role is set by the target's `mode` / `entrypoint` in
`mach.toml`.

A file begins with a module docstring (see Docstrings), then `use`/`fwd` lines,
then declarations.

## Executable entrypoint

The standard library provides the platform `_start` symbol that calls your
`main`. Pull it in with `use std.runtime;`, tag `main` with the `` `symbol` ``
decorator so the linker finds it, and return an exit code:

```mach
use std.runtime;

`symbol("main")`
fun main(argc: i64, argv: **u8) i64 {
    ret 0;
}
```

`use std.runtime;` is what links the entrypoint into the binary.

## Output — `std.print`

There is no built-in `print`. Use the standard library, which returns a
`Result` (output can fail):

```mach
use std.print;

fun greet() {
    print.println("hello, mach");
}
```

`use std.print;` binds the leaf name `print` (the module), so members are
reached qualified as `print.<member>` — `std` is not itself in scope, so
`std.print.println(...)` would not resolve. The `print` module exposes
`print` / `println` (stdout), `eprint` / `eprintln` (stderr), and formatting
variants. Each returns `Result[usize, str]` — bytes written, or an error.

## `use` — private import

```mach
use std.types.size;        # binds module `size`; use as `size.usize`
use sz: std.types.size;    # module under alias `sz`; use as `sz.usize`
use std.types.size.usize;  # binds the symbol; use bare as `usize`
```

The resolver binds whatever the path ends at: a **module** (access members
qualified, `module.member`) or a **symbol** (used bare). Importing a module
does **not** pull its members in unqualified — to use `usize` bare, import the
symbol, not the module. **No splat, no combined forms** (`use foo.*` and
`use foo.{a,b}` do not exist). One name per line. A module must `use` every
dependency it directly names — even ones reached through a re-export. The
dependency graph is visible at the top of every file.

## `fwd` — public re-export

```mach
fwd impl.Point;        # re-export as `Point`
fwd Pt: impl.Point;    # re-export as `Pt`
```

Mirrors `use` grammar; always publishes; no `pub` modifier. One per line, no
splat.

## Shadow-module pattern

A surface file `foo.mach` co-exists with a directory `foo/`. The surface owns
the public face; the directory holds split implementations. The surface `use`s
each split and `fwd`s its public symbols, flattening them into the `foo`
namespace. Consumers `use myproj.foo;` and reach `foo.X` without naming splits.

```mach
use myproj.foo.data;
use myproj.foo.ops;

fwd data.Point;
fwd ops.add;
```

- **Topical split** — forward all impls unconditionally.
- **Multiplatform split** — pick one impl per target with `$if` on
  `$mach.build.os` / `$mach.build.arch`, then `use` + `fwd` the chosen one.

## Declarations

Modifiers: `pub` (public surface) and `ext` (C-ABI, functions only, no body).
Without `pub`, a declaration is file-private. `pub` applies to `fun`, `rec`,
`uni`, `def`, `val`, `var`, `ext fun`.

### `def` — type alias

```mach
pub def Age:    i64;
pub def BinOp:  fun(i64, i64) i64;
pub def Anon:   rec { x: i64; y: i64; };
pub def Choice: uni { a: i64; b: f64; };
```

Aliases name any type; alias and underlying type are interchangeable (no
nominal distinction).

### `rec` — record / `uni` — raw union

```mach
pub rec Point { x: i64; y: i64; }
pub rec Pair[T, U] { left: T; right: U; }   # generic

pub uni Number { i: i64; f: f64; }
pub uni Maybe[T] { some: T; none: u8; }     # generic
```

A `rec` lays fields out with padding; a `uni` overlaps all fields in one slot
(size of largest field). The compiler does not track which `uni` field is live.

Discriminated value (the only sum-type idiom) — this is exactly how the stdlib
`Result[T, E]` is built:

```mach
rec Result[T, E] {
    tag:   bool;
    value: uni { ok: T; err: E; }
}
```

Read the discriminator, then access the matching payload field with `if`/`or`.

### `fun` — function

```mach
fun NAME(args) RET { ... }        # with return type
fun NAME(args) { ... }            # no return
fun NAME[T](args) RET { ... }     # generic type params (no constraints)
fun NAME($p: T, args) RET { ... } # comptime value param
fun NAME(a, b, ...) RET { ... }   # variadic (trailing ...)
```

```mach
pub fun add(a: i64, b: i64) i64 { ret a + b; }
pub fun identity[T](value: T) T { ret value; }
```

- Generic type params in `[T]` brackets — **types only, no constraints**;
  compiler monomorphizes per instantiation. Call sites supply types:
  `identity[i64](42)`.
- Comptime value params use `$name: T` *in the regular paren list*; the caller
  passes a comptime-evaluable value (enforced at the call site, passed
  positionally like a runtime arg). This form is **function parameters only** —
  never record fields.
- Variadic via trailing `...`; access through `va_list` / `va_start` /
  `va_arg[T]` / `va_end` (C convention). The `va_list` builtin type and the
  `va_*` intrinsics are compiler-seeded — typed in sema and lowered to
  dedicated IR opcodes against the SysV register-save area. The limitation is
  ABI completeness at the edges (e.g. passing/fetching large by-value
  aggregates as variadic arguments); scalar and pointer varargs work.

### `ext fun` — external function

```mach
`symbol("write")`                          # optional linker-name override
pub ext fun libc_write(fd: i64, buf: *u8, n: i64) i64;
```

Body-less, ends in `;`. C ABI is the contract; arg/return types must be C-
representable. The only body-less function form.

### `val` / `var` — bindings

```mach
val pi: f64 = 3.14159;     # immutable, initializer required
var counter: i64 = 0;      # mutable, explicit init
var buf: [256]u8;          # mutable, default-initialized (zero)
```

Work at module top level (`pub val` / `pub var` export) and in function bodies.

## Type grammar

Primitives follow `<u|i|f><width>(x<count>)*`.

- Scalars: `u8 u16 u32 u64`, `i8 i16 i32 i64`, `f32 f64`.
- Untyped pointer: `ptr`. (`bool` is a stdlib `def` for `u8`, not a built-in.)
- Variadic cursor: `va_list` — the compiler-seeded register-save struct used
  with `va_start` / `va_arg[T]` / `va_end` (see Variadics under `fun`).
- SIMD vectors: append `x<count>` — `f32x4`, `i32x8`, `u8x16`. **Planned, not
  yet implemented**: vector type names do not resolve as types today. Higher
  dims (`f32x4x4`) are grammatically legal. Atomic/volatile live outside the
  type name (no bare-letter modifiers).

Compound:

```mach
*T              # pointer
[N]T  [N][M]T   # array, nested
fun(T1, T2) R   # function-pointer type
```

```mach
var x: i64 = 9;
var p: *i64 = ?x;     # address-of yields *i64
val v: i64  = @p;     # dereference
val a: [4]i64 = [4]i64{1, 2, 3, 4};
def BinOp: fun(i64, i64) i64;
```

## Literals

| Form | Example | Notes |
|---|---|---|
| Decimal / hex / bin / octal | `42` `0xDEAD` `0b1010` `0o755` | untyped |
| Underscores / scientific | `1_000_000` `1.5e10` | untyped |
| Typed suffix | `7i64` `255u8` `2.5f64` | typed by suffix |
| Char | `'M'` | `u8` |
| String | `"hi\n"` | `*u8`, null-terminated |
| `nil` | `nil` | null-address literal; coerces to any pointer type |

Untyped numeric literals are *checked* against the surrounding declared type;
they do not infer it. If context does not constrain the type, use a suffix
(`42i64`). `nil` types as `*u8` with no context and coerces to any pointer;
relate function pointers to `nil` via `==` / `!=`, not assignment. Char escapes:
`\n \t \r \\ \' \0 \xHH`. String escapes: same plus `\"`. A string literal may
span multiple lines — the lexer scans to the next unescaped `"`, so a raw
newline inside the quotes is part of the literal.

## Operators

- Arithmetic: `+ - * / %` — int/float scalars (and lane-wise on planned SIMD
  vectors).
- Bitwise: `& | ^ ~ << >>` — integer scalars (and planned integer vectors).
- Comparison: `== != < > <= >=` → `u8` (`1`/`0`). A pointer may be compared
  against `nil`.
- Logical: `&& || !` — short-circuiting; `u8` operands (`0` false, nonzero
  true) → `u8`.
- Unary: `-` negate, `~` bitwise NOT, `!` logical NOT.
- Pointer: `?expr` address-of, `@ptr` dereference (also `@p = x;` to write).
- Cast: `expr::Type` value conversion (width/sign/pointer, int↔float by value) and `expr:~Type` bit reinterpret (read the exact bits as an equal-size type).

Precedence follows C-family conventions. There is no compound assignment
(`+=`, `-=`, … do not exist).

## Statements

End with `;` except when ending in a block `{...}`. There is no
brace-less single-statement body.

```mach
if (cond) { ... } or (cond) { ... } or { ... }   # `or {}` is the catch-all
for (cond) { ... }                                # single condition-loop; no for-each
ret expr;  ret;                                   # value / void return
brk;  cnt;                                         # break / continue enclosing for
fin stmt;   fin { ... }                            # defer to scope exit, reverse order
{ ... }                                            # bare block = new lexical scope
```

`fin` schedules cleanup that runs when the enclosing scope exits, in reverse
declaration order. It takes a single statement or a block — no bare-expression
form.

## Expressions

```mach
counter                              # name in scope
core.add                             # module-qualified
Point{ x: 1, y: 2 }                  # record literal
[3]i64{10, 20, 30}                   # array literal
Number{ i: 99 }                      # union literal
Pair[i64, u8]{ left: 5, right: 6u8 } # generic literal: type args before body
p.x   a[0]                           # field / index access
add(2, 3)   identity[i64](42)        # call / generic call
```

Vector literals (`f32x4{ ... }`) follow the same shape but depend on the SIMD
vector types, which are not yet implemented.

## Docstrings

`#` comments immediately above a declaration. Summary line, optional `# ---`
separator, then one component line per exposed element. Prose is lowercase
except proper nouns and type names.

```mach
# realtime: read the wall-clock time.
# ---
# out: pointer to Timespec to populate
# ret: 0 on success, negative errno on failure
pub fun realtime(out: *Timespec) i64 { ... }
```

Component identifiers: parameter name, `$name` (comptime param), `[T]` (generic
param), `ret` (return), field name, variant name. Summary-only docstrings omit
the separator and component block:

```mach
# spin_hint: yield the CPU to other threads.
pub fun spin_hint() { ... }
```

A module file opens with a module docstring (full dotted path as `<name>`)
before any `use`/`fwd`/attribute. Modules may add paragraphs separated by blank
`#` lines; other declaration kinds do not extend past the component block.
Backtick decorators follow the docstring, immediately above the decl.

## Comptime channel (`$`) — brief

`$mach.*` reads compiler-provided state; per-declaration backtick decorators
`` `name(args)` `` attach codegen attributes to the following decl. Closed
decorator set: `` `symbol` `` `` `library` `` `` `inline` `` `` `align` `` `` `section` ``. Comptime control:
`$if (C) {} $or (C) {} $or {}` — only the taken branch compiles. Value
intrinsics `$size_of(T) $align_of(T) $offset_of(T, f)` yield comptime unsigned
integers; `$error("msg")` / `$assert(C, "msg")` are diagnostics. The full
comptime/asm grammar is out of scope here — see the **mach-comptime** skill.

## Reference

The authoritative per-feature language reference lives in the Mach repository
under
[`doc/language/`](https://github.com/octalide/mach/tree/dev/doc/language),
including the EBNF in `grammar.md`. When a skill and the reference disagree, the
reference wins.
