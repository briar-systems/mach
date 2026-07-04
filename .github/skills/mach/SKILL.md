---
name: mach
description: Use when writing, editing, or reviewing Mach (.mach) source files. Covers the full language: project/module structure, use/fwd imports and re-exports, the shadow-module pattern, all declaration forms (def, rec, uni, fun, ext fun, val/var, test), the type grammar with the ^ secret qualifier, literals, operators and casts, statements, docstring conventions, stdlib idioms (Result/Option, std.print), the comptime channel ($mach.*/$project.*/$bin.* reads, $if/$or, $each, intrinsics, comptime parameters, variadic packs), #[...] decorators, and inline assembly (asm x86_64/aarch64/riscv64).
---

# Mach

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
applies to `fun`, `rec`, `uni`, `def`, `val`, `var`; `ext` (C-ABI external)
applies to functions only.

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
pub fun load($order: u8, p: *i64) i64 { ... }   # comptime value param (see Comptime)
pub fun sum(va: ...) i64 {                      # variadic pack (see Comptime)
    var t: i64 = 0;
    $each a in va { t = t + a; }
    ret t;
}
```

Generic params `[T]` take types only, no constraints; monomorphized per
instantiation; call sites always supply the types explicitly.

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

The label is a required string literal. Its internal format is a convention,
not an enforced standard: choose whatever names the test clearly and stays
consistent within a project. The compiler uses a fully-qualified
`module.path.symbol:behavior` form (e.g.
`"mach.cli.cmd.doc.write_doc:summary_only"`) that pins each test to the exact
unit it exercises and groups cleanly under `--filter`; the libraries favor a
shorter `"topic: behavior"` (e.g. `"abs: i64 min saturates"`). Either reads
well. The body checks like an `i32` function: `ret 0` (or falling off the end)
passes, any non-zero return fails. There are no assertion builtins; return
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
unroll) also appear in statement position — see Comptime below.

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

## Comptime channel

`$` opens the compiler-owned comptime channel — read-only: it *selects and
expands*, never executes or mutates. The parser disambiguates by shape:

| Shape | Meaning |
|---|---|
| `$mach.*` / `$project.*` / `$bin.*` | read a compiler-owned tree (roots are reserved) |
| `$sym(args)` | comptime call — the closed intrinsic set |
| `$if` / `$or` | comptime control flow |
| `$each x in SEQ { }` | comptime unroll over `$fields(T)` or a variadic pack |

A bare `$ident` (`$mode`, `$foo`) is none of these and is rejected: comptime
*parameters* are referenced without `$`; comptime *paths* are rooted. The
`$sym.attr = value;` setters were removed in v2.0.0 — codegen directives are
`#[...]` decorators. Not in the channel: no `$<Type>.*` reflection, no
comptime function definitions, no comptime loops beyond `$each` over
fields/packs.

### `$mach.*` — compiler and build state

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

### `$project.*` / `$bin.*` — manifest state

```mach
$project.id / .version / .name / .description   # [project] metadata
$project.version.major / .minor / .patch        # folded integer components
$project.target.os / .arch / .abi               # the selected target's declared *strings*
$bin.name                                       # the artifact being built
```

`$project.target.*` carries the manifest's string spellings (`"linux"`,
`"x86_64"`) — distinct from `$mach.build.*`'s numeric tags. A field the
manifest does not declare is reported unavailable, not folded to `""`.

### Decorators — `#[...]`

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

### Intrinsics — `$name(args)`

Closed compiler-shipped set. New ones require a compiler change; runtime
instruction emitters (`trap`, `fence`, `pause`) are **not** intrinsics — they
are stdlib functions with per-arch `asm` bodies.

**Layout values** — comptime unsigned integers; the binding declares the
storage type (`pub val POINT_SIZE: i64 = $size_of(Point);`):

```mach
$size_of(T)   $align_of(T)   $offset_of(T, field)
```

**`$type_of(expr)`** produces a comptime type value, comparable with
`==`/`!=` against a type name inside `$if`. Dead arms are **pruned before
type-checking**, so each arm can use the value at its own concrete type with
no cast — the idiom for per-element dispatch inside `$each` bodies (stdlib
`vformat` is built exactly this way):

```mach
$if ($type_of(arg) == str)  { write_str(w, arg); }
$or ($type_of(arg) == i64)  { write_i64(w, arg); }
$or { $error("no writer for this argument type"); }
```

**`$fields(T)`** yields a comptime sequence of field descriptors for a
`rec`/`uni`, consumed by `$each`. Each descriptor `f` carries `f.name`
(`*u8`), `f.type` (type value), `f.offset` (integer); `v.[f]` projects the
concrete field off an instance — an lvalue, re-typed per iteration:

```mach
fun sum_fields(p: Pair) i64 {
    var t: i64 = 0;
    $each f in $fields(Pair) { t = t + p.[f]; }
    ret t;
}
```

**`$each`** is statement-scope only. The body is spliced once per element —
not a runtime loop. Two sequence forms: `$each f in $fields(T)` and
`$each a in va` (packs). Enclosing runtime variables thread across the
unrolled copies; nesting is allowed.

**Diagnostics.** `$error("msg")` fails the build when **reached** —
unconditional position or a selected `$if`/`$or` arm. A `$error` in a
discarded arm never fires, making it the natural exhaustiveness fallback for
target and type dispatch. Valid at declaration and statement scope.
**`$assert` parses but is not yet evaluated** — write
`$if (!cond) { $error("msg"); }` instead.

### `$if` / `$or` — conditional compilation

```mach
$if (cond) { ... }
$or (cond) { ... }
$or { ... }             # comptime else — no condition; there is no `$or $if`
```

Only the taken branch compiles: discarded branches are not resolved,
type-checked, or emitted — names inside them are never looked up. This is
what makes per-arch `asm` and per-OS `use` safe. Valid at declaration scope
(selecting `use`/`fwd`/declarations) and statement scope. `$if` selects at
compile time and discards the rest; runtime `if` emits a branch — never use
`$if` to fake reflection over code that must exist at runtime.

Conditions must be comptime: `$mach.*`/`$project.*` reads, comptime constants
(`pub val`), comptime parameters, `$type_of` comparisons. Comptime comparison
follows the runtime rules — mathematical values, mixed signedness fine;
overflow in comptime arithmetic is a compile error, not a wrap.

### Comptime function parameters — `$name: T`

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

### Variadic packs

`va: ...` (trailing parameter) collects call-site arguments into a comptime
sequence; monomorphized per distinct type-list. `$each a in va` consumes it
(heterogeneous packs work — dispatch with `$type_of` per element), `va.len`
folds to the count, `g(va...)` forwards the whole pack (sole trailing argument
only; no partial forward). No runtime `va_list` exists. Pack functions have no
stable ABI symbol: not `ext`, not addressable. See `doc/language/variadics.md`.

## Inline assembly

One form: an ISA-tagged block of raw instruction lines.

```mach
asm x86_64 {
    # raw instructions, one per line, # for comments
    mov rcx, {ptr}
    mov rax, [rcx]
    mov {result}, rax
}
```

Locked rules:

- The ISA tag is **mandatory**; bare `asm { }` is rejected. The tag set is
  closed: `x86_64`, `aarch64`, `riscv64` — each with a working native
  assembler, all three exercised in CI (riscv64 under qemu; riscv64 is a
  self-hosting target with a byte-identical fixpoint).
- The body is **raw text**, not tokens: unquoted lines in the ISA's native
  syntax, captured to the brace-matched `}` (nested braces balance). The `asm`
  body is not a string.
- `#` starts a line comment; everything after it on the line is inert.

### Operand substitution — `{name}`

`{name}` substitutes a local in scope; the compiler resolves it to a register
or memory operand from liveness and the instruction's operand class. In
practice a `{name}` binds the local's **storage — typically a stack slot** —
so to reach a pointee, stage the pointer through a scratch register first;
never write a double indirection like `[{ptr}]`:

```mach
mov rcx, {ptr}          # x86_64: load the pointer value
mov rax, [rcx]          # then address through the register
                        # aarch64: ldr x12, {ptr}  /  ldr x9, [x12]
```

`{name}` is the only substitution: no `in`/`out` lists, no `%0` positionals,
no `=` constraints. The compiler infers **operand direction** (from position),
the **clobber set** (from each instruction's semantics), and assumes a
conservative **memory clobber** for every block. Writing a clobber list is a
syntax error, not optional metadata. Branch targets inside a block are numeric
local labels with direction suffixes (`1:` … `bnez a4, 1b`, `b.ne 2f`), the
stdlib's convention across all three ISAs.

### Multi-arch and multi-OS dispatch

No nested arch construct exists inside `asm`. Dispatch at the outer level with
`$if` on `$mach.build.arch` (or `.os` for syscall ABIs); discarded branches
never compile, so each block only needs to be valid for its own ISA:

```mach
$if ($mach.build.arch == $mach.arch.x86_64) {
    asm x86_64 { hlt }
}
$or ($mach.build.arch == $mach.arch.aarch64) {
    asm aarch64 { brk 0 }
}
$or ($mach.build.arch == $mach.arch.riscv64) {
    asm riscv64 { ebreak }
}
$or {
    $error("mymod.trap: unsupported architecture");
}
```

A platform-specific module guards itself at the top so misuse fails loudly at
compile time — the stdlib pattern:

```mach
$if ($mach.build.arch != $mach.arch.x86_64) {
    $error("myproj.os.linux.x86_64: requires x86_64 target");
}
```

Variant dispatch (memory orderings and similar) is one function with a
`$name: T` comptime parameter whose `$if` arms select per-variant `asm` — one
monomorphized instance per distinct value (see Comptime function parameters).

### `asm` vs stdlib

Write `asm` only for truly target-specific operations with no stdlib wrapper:
raw syscalls, special-register reads, stack-frame surgery. For anything that
maps to a named function — atomics, fences, `trap()`, bit ops (popcount, clz,
bswap), syscall wrappers — **call the stdlib**: those functions already
contain the arch-dispatched `asm`, and reimplementing them inline duplicates
the dispatch. Rule of thumb: if the operation is a fixed instruction sequence
per arch, there is (or should be) a stdlib wrapper.

### Secrecy — `asm` is a trusted-base crossing

The `^` secret qualifier's flow rules stop at an `asm` boundary: the type
system cannot check instruction streams, so an `asm` block can observe or
launder secrets silently. Inside `asm` that touches `^` data, constant-time
discipline (no secret-dependent branches, addresses, or variable-latency
instructions) is entirely on you.

## Reference

The authoritative per-feature reference lives in the Mach repository under
[`doc/language/`](https://github.com/briar-systems/mach/tree/dev/doc/language)
— including the full EBNF in `grammar.md`, `variadics.md`, `secrecy.md`,
`decorators.md`, the `comptime-*.md` set, `asm.md`, and `policy.md` (the
compiler-vs-stdlib boundary). When this skill and the reference disagree, the
reference wins.
