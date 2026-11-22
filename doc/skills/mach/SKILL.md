---
name: mach
description: Authoring guide for Mach 7. Load before writing, reviewing or fixing any .mach source or mach.toml, or before running the mach compiler. Leads with the rules where C, Rust, Zig and Go habits produce wrong Mach, then the reference, std idioms and commands.
---

# Mach 7

Mach is explicitly typed and compiled ahead of time, with no garbage collector and no hidden control flow. The authority is the compiler, then the language reference in the mach repository's `doc/language/*.md` (`migrate.md` lists what changed from 6), then the std source under `dep/std/src`. Every sample here builds with mach 7.0.0. A line `# wrong: ... -> key` shows the spelling a habit produces in place of the line above it, and the diagnostic key the compiler refuses it with.

## Where habits from other languages fail

**Types are never inferred, and nothing converts implicitly.** Every `val` and `var` states its type, and a literal suffix types the literal, not the binding. Integers of different types never mix, not even to widen: cast with `::`. The one implicit conversion is between `ptr` and `*T`, both ways.

**Conditions are `u1`.** Comparisons, `&&`, `||`, `!` and `sel` produce `u1`, and `if` and `for` accept nothing else. `bool` is std's `def bool: u1`, with `true` and `false` as std `val`s, imported like any name. A `u1` has no arithmetic and never flows into a `u8`: `b::u8` converts it, and `n::u1` asks whether `n` is non-zero.

**Comparison binds tighter than `&`, `^` and `|`**, so a bit test is parenthesized, and the bare form is refused with a message about reading a `u1` as a wider integer. **Assignment is `=` only**: `+=` and `++` are syntax errors.

```mach
use std.types.bool.bool;

fun low_nibble_set(flags: u32) bool {
    ret (flags & 0x0f) != 0;
    # wrong: ret flags & 0x0f != 0;   -> type.mismatch
}

fun scaled(n: i32, k: i64) i64 {
    val wide: i64 = n::i64;
    # wrong: val wide: i64 = n;   -> type.mismatch
    # wrong: val wide = n::i64;   -> binding.untyped
    ret wide * k;
    # wrong: ret n * k;   -> operator.operand_mismatch
}

fun count_to(n: u64) u64 {
    var i: u64 = 0;
    for (i < n) {
        i = i + 1;
        # wrong: i += 1;   -> syntax.expected_expr
    }
    if (n != 0) { ret i; }
    # wrong: if (n) { ret i; }   -> condition.type
    ret 0;
}
```

**Integer literals.** An unsuffixed literal takes its context's type, and arithmetic on literals wraps in that type, so `val m: u8 = 0 - 1;` is 255. A suffixed literal is range-checked against its suffix. With no context a literal is `i64` (a float `f64`), and beside a typed operand it takes that operand's type. Comparisons of any two integer types are legal and compare true values, so an `i64` holding -1 is less than any `u64`. A constant shift count at or past the width is refused, and a run-time one saturates.

```mach
val MAX: u8  = 0 - 1;
val ALL: u64 = 0xFFFF_FFFF_FFFF_FFFF;
val MIX: u32 = 0b1010 | 0o17 | 0x2A;
# wrong: val MIX: u32 = 256u8;   -> literal.out_of_range
val ONE: u8 = 'A' - 64;
```

**Strings are `*u8` with no length.** A literal is a NUL-terminated `*u8` in static data, and std's `str` is `def str: *char` with `char` a `u8`. There is no `.len`, and `==` compares addresses: use `std.types.string` (`str_len`, `str_equals`, `str_starts_with`, `str_index_of`). A literal checked against `[N]u8` is that array and holds exactly its bytes, with a terminator only when written, and the counts must match. `[_]u8` takes the count. Text of several lines starts each line with `\\`, joins them with `\n`, adds no final newline, and puts the `;` on the next line.

```mach
use std.types.bool.bool;
use std.types.size.usize;
use std.types.string.str;
use std.types.string.str_equals;
use std.types.string.str_len;

val TAG:  [3]u8 = "abc";
val TAGZ: [4]u8 = "abc\0";
# wrong: val TAGZ: [4]u8 = "abc";   -> array.literal_length
val NAME:  [_]u8 = "mach";
val USAGE: str   =
    \\usage: tool [options]
    \\  -v  verbose
;

fun is_hi(s: str) bool {
    val n: usize = str_len(s);
    ret n == 2 && str_equals(s, "hi");
}
```

**Control flow is `if`/`or` and `for`.** `or (cond) { }` chains and a bare `or { }` is the fallback. `for (cond) { }` is the loop and a bare `for { }` loops until `brk` or `ret`. `brk` and `cnt` leave or continue the innermost `for`. Every body is braced. For `else` write `or`, for `while` or `loop` write `for`, for `switch` or `match` an `if`/`or` chain (with `sel` on a tag), for `defer` write `fin { }`, and for `try` or `?` an explicit test and `ret`. `goto` and `do` have no counterpart. `else` and `while` are plain names to the parser, so using them gives confusing syntax errors. The compiler does not check that a function with a result returns on every path, and the reference states no value for falling off the end, so end every path with `ret`.

**Pointers.** `?place` takes an address (of a place only, never a temporary), `@p` dereferences, and `p.field` reads through a pointer once. Prefix operators take the whole postfix chain after them, so `@p.x` is `@(p.x)` and `@p::T` is `@(p::T)`: parenthesize as `(@p)::T`. `p[i]` steps by the pointee, so on `xs: *[4]i64` the element is `xs[0][i]` or `(@xs)[i]`. Pointer arithmetic is `?p[i]`, never `p + i`.

```mach
rec Point {
    x: i64;
    y: i64;
}

fun nudge(p: *Point, xs: *[4]i64) i64 {
    p.x = p.x + 1;
    # wrong: @p.x = 1;   -> ptr.deref_non_pointer
    val first: i64 = xs[0][1];
    # wrong: val first: i64 = xs[1];   -> type.mismatch
    val q: *i64 = ?xs[0][2];
    # wrong: val q: *i64 = ?xs[0][0] + 2;   -> operator.operand_mismatch
    ret first + @q;
}
```

**No methods.** Every operation is a free function taking its receiver explicitly, as `vector.push[i64](?v, 9)`, never `v.push(9)`, which is read as a field and refused with `field.unknown`.

**Tags, `sel` and guards.** A `tag` holds one case. `sel place.case` is a `u1` test that reads only the discriminator, and its operand must be a place, never a call. A payload `place.case` is readable, writable and addressable only inside a guard, a lexical region matched by the place's spelling:

1. the block of an arm whose whole condition is `sel p.c`
2. the right operand of `&&` whose left operand is `sel p.c`, and only that operand: the arm's body is not guarded
3. the rest of the enclosing block after a chain whose every arm exits by `ret`, `brk` or `cnt` and leaves one case untested (arms may test `sel` or `!sel` of the place), and likewise that chain's final bare `or`

`||`, `!` and other operators open nothing. Assigning the whole guarded place inside its guard is refused, and assigning a name the place is spelled with (the `i` of `items[i]`, a pointer `p`) ends the guard from there on, for the whole loop when the guard opened outside it. Read the payload into a name first.

```mach
tag Shape: u8 {
    dot;
    line:   i64;
    square: i64;
}

fun size(s: Shape) i64 {
    if (sel s.dot)  { ret 0; }
    or (sel s.line) { ret s.line; }
    ret s.square * s.square;
}

fun capped(s: Shape) i64 {
    if (sel s.line && s.line > 100) {
        ret 100;
        # wrong: ret s.line;   -> tag.payload_unguarded
    }
    ret 0;
}

fun next_line(items: *Shape) i64 {
    var i: u64 = 0;
    if (sel items[i].line) {
        val v: i64 = items[i].line;
        i = i + 1;
        ret v;
        # wrong: ret items[i].line;   -> tag.payload_unguarded
    }
    ret 0;
}
```

**Failure is a value, handled by hand.** std's `res[T, E]`, `opt[T]` and `err[E]` are ordinary tags with the failure case first, so a zeroed one is a failure or `none`. Test the failure, return, and read the success after: the guard rule 3 above makes that read legal.

```mach
use std.types.result.res;

tag ParseError: u8 {
    empty;
    bad_digit: u8;
}

fun digit(c: u8) res[i64, ParseError] {
    if (c < '0' || c > '9') { ret res[i64, ParseError].err{ParseError.bad_digit{c}}; }
    ret res[i64, ParseError].ok{(c - '0')::i64};
}

fun doubled(c: u8) res[i64, ParseError] {
    val r: res[i64, ParseError] = digit(c);
    if (sel r.err) { ret res[i64, ParseError].err{r.err}; }
    ret res[i64, ParseError].ok{r.ok * 2};
}
```

**Generics take explicit arguments**, at every call and every literal. There are no constraints: a body may use operators, casts and fields on `T`, and each instance is checked against its type arguments, with an error pointing into the body. `f[T]` without a call is the instance as a function value. A generic nothing instantiates is checked only for names.

```mach
rec Pair[T] {
    a: T;
    b: T;
}

fun larger[T](p: Pair[T]) T {
    if (p.a > p.b) { ret p.a; }
    ret p.b;
}

fun pick() i64 {
    val p: Pair[i64] = Pair[i64]{a: 3, b: 9};
    # wrong: val p: Pair[i64] = Pair{a: 3, b: 9};   -> generic.missing_type_args
    val f: fun(Pair[i64]) i64 = larger[i64];
    ret f(p) + larger[i64](p);
    # wrong: ret larger(p);   -> generic.missing_type_args
}
```

**Zero is the default, arrays are values, and `fin` takes a block.** A `var` with no initializer is zero in every byte: numbers 0, pointers `nil`, a tag its first case with that payload zeroed. Fields a literal omits are zero, and `T{}` is all zero. Padding is unspecified. A `val` needs an initializer and is never assigned, though through a `val` pointer the pointee is writable. Parameters are assignable locals. `[N]T` is copied by assignment, argument and return, so pass `*[N]T` or a pointer and length (std's `Slice[T]`) to share. `fin { }` runs when its block exits, covering the statements after it, latest first, and replays on `ret`, `brk` and `cnt`. A `ret` value is computed before the fins run.

```mach
use std.print;
use std.runtime;

rec Point {
    x: i64;
    y: i64;
}

fun bump(a: [3]i64) i64 {
    a[0] = 100;
    ret a[0];
}

#[symbol("main")]
fun main(argc: i64, argv: **u8) i64 {
    var counts: [3]i64;
    val p:      Point = Point{x: 4};
    val n:      i64   = bump(counts);
    {
        fin { print.printlnf("{} {} {}", counts[0], p.y, n); }
        # wrong: fin print.println("done");   -> syntax.expected_token
        counts[0] = 7;
    }
    # prints: 7 0 100
    ret 0;
}
```

**`==` is refused on a whole `rec`, `uni` or `tag`** (`operator.aggregate_equality`). Compare fields, use `std.derive.eq[T](?a, ?b)`, or `sel` for a tag's case. Integer against float is refused too. Pointers compare as addresses.

## Projects and modules

A project is a directory with a `mach.toml`. Every command takes the project path explicitly (`mach build .`). `mach init <dir>` writes this shape with six targets and std checked out under `dep/std`:

```toml
[project]
id = "app"
version = "0.1.0"
mach = "^7"
src = "src"
work = "out/{target.name}/{profile.name}"

[target.linux-x86_64]
arch = "x86_64"
sys  = "linux"
abi  = "sysv64"

[artifact.app]
kind = "bin"
entry = "bin/main.mach"
out = "{project.work}/bin/app{artifact.suffix}"
targets = ["*"]

[profile.debug]
default = true
optimize = false
debug = true
simd = "scalarize"

[profile.release]
optimize = true
debug = false
simd = "scalarize"

[dep.std]
git = "https://github.com/briar-systems/mach-std"
ref = "branch/dev"
```

Target keys are `arch`, `sys`, `abi`, `env`, `isa`, `ext` (never 6's `os` or `isa` naming an architecture). Artifact kinds are `bin`, `static`, `shared`, `object` and `module`. Every profile states `optimize`, `debug` and `simd` (`"scalarize"` or `"require"`). The entry is `src/bin/main.mach` for an executable and `src/lib/<id>.mach` for a library, by convention. A build compiles what the entry reaches through `use` and `fwd`.

- A file `src/a/b.mach` in project `app` is the module `app.a.b`. Every path is `<project id>.<source path>`, in the project itself too, and `use std;` (a project with no module) is refused.
- `use path;` binds the last segment, `use name: path;` an alias. A path ending at a module binds the module (`print.println`), one ending at a symbol binds the symbol. One name per line, no globs or braces. Imports are private.
- `fwd path;` re-exports, always publicly. A surface `foo.mach` beside a directory `foo/` loads its splits and forwards them, with `$if` choosing a split per target.
- Declarations are file-private unless `pub`. `#[export]` (which needs `pub`) is what a shared library exports. An executable's entry has `use std.runtime;` and a function with `#[symbol("main")]` taking `(argc: i64, argv: **u8)` and returning `i64`.

```mach
# file: src/bin/main.mach
use app.geometry;
use std.print;
use std.runtime;

#[symbol("main")]
fun main(argc: i64, argv: **u8) i64 {
    print.printlnf("{}", geometry.area(3, 4));
    # prints: 12
    ret 0;
}
# file: src/geometry.mach
use app.geometry.flat;

fwd flat.area;
# file: src/geometry/flat.mach
pub fun area(w: i64, h: i64) i64 {
    ret w * h;
}
```

## Declarations

| Form | Example |
|---|---|
| constant, variable | `val N: i64 = 5;` `var buf: [256]u8;` |
| alias, function type | `def Age: u32;` `def BinOp: fun(i64, i64) i64;` |
| record | `rec Point { x: i64; y: i64; }`, literal `Point{x: 1, y: 2}` (positional braces are refused) |
| union, untracked | `uni Number { i: i64; f: f64; }`, literal `Number{i: 9}` |
| tag | `tag Status: u8 { pending; failed: i32; }`, literal `Status.failed{-1}`, `Status.pending{}` |
| function | `fun add(a: i64, b: i64) i64 { ret a + b; }`, no result type for none |
| generic | `fun id[T](v: T) T`, `rec Box[T] { v: T; }` |
| comptime parameter | `fun f($mode: u8, n: i64) i64` |
| pack | `fun sum(va: ...) i64`, unrolled with `$each` |
| foreign | `ext fun strlen(s: *u8) i64;` `ext var errno: i32;` |
| test | `test subject__case { }` |

A tag's discriminator is `u8`, `u16`, `u32` or `u64`, and each case holds one payload (a record for several). A module-level initializer must be a compile-time value, so a call there is refused. A body-less `fun` exists only as `ext fun` or `#[intrinsic]`. Doc comments are `#` lines directly above a declaration: a lowercase summary, then optionally `# ---` and one `# name: description` line per parameter, `[T]`, `$param`, field or case, in declaration order, then `ret`.

```mach
# the sum of two integers
# ---
# a:   first operand
# b:   second operand
# ret: a + b
pub fun add(a: i64, b: i64) i64 {
    ret a + b;
}
```

## Types

| Family | Spelling |
|---|---|
| truth | `u1` (one byte, 0 or 1, no arithmetic) |
| integers | `u8` `u16` `u32` `u64` `u128` `u256` `u512`, `i8` through `i512` |
| floats | `f16` `f32` `f64` |
| pointers | `ptr` (untyped), `*T`, `nil` for any pointer or function value |
| arrays | `[N]T`, `[_]T` only where an initializing literal gives the length |
| vectors | element, `x`, lane count: `f32x4`, `i32x8`, `u8x16`. Literal `f32x4{1.0, 2.0, 3.0, 4.0}`, lane `v[0]` by constant index |
| functions | `fun(i64, *u8) i64` |
| secret | `^T`, constant-time data |
| std aliases | `bool` (`u1`), `usize`/`isize` (pointer width), `char` (`u8`), `str` (`*char`) |

A constant array index is bounds-checked. `x[start, n]` is an `[n]T` window with constant `n`. Vector operators are lane-wise between the same shape, and a comparison yields an unsigned mask vector.

## Operators and casts

| Binds | Operators |
|---|---|
| tightest | postfix `f()` `a[i]` `a.b` `::T` `:~T` `:>T`, then prefix `-` `!` `~` `?` `@` |
| | `*` `/` `%` |
| | `+` `-` |
| | `<<` `>>` |
| | `<` `<=` `>` `>=` |
| | `==` `!=` |
| | `&`, then `^`, then `\|` |
| | `&&` (short-circuit), then `\|\|` |
| loosest | `=` (right-associative) |

`x::T` converts a value (resize, int and float, same-count array and vector). `x:~T` reinterprets bits between types of one size. `x:>T` strips `^` and is the only way to. `%` is the truncated remainder. `~` is bitwise not, `!` is logical not on `u1`. A cast is postfix, so `-x::i64` is `-(x::i64)`.

## The comptime channel

`$` reads compile-time facts and expands code. It never runs user functions.

| Read | Value |
|---|---|
| `$mach.build.sys` | `$mach.sys.` `linux` `darwin` `windows` `freestanding` `uefi` `vulkan` `wasi` |
| `$mach.build.arch` | `$mach.arch.` `x86_64` `aarch64` `riscv64` `riscv32` `spirv` `wasm32` |
| `$mach.build.abi` | `$mach.abi.` `sysv64` `win64` `aapcs64` `darwinpcs` `aapcs64win` `lp64d` `shader` and the other ABIs |
| `$mach.build.mode` | `$mach.mode.debug` or `$mach.mode.release` |
| `$mach.build.pointer_width`, `.pie`, `.ext.<name>` | bytes, 1 or 0, 1 when the target selects that extension |
| `$mach.version`, `$mach.project.version`, `$mach.source.line` | compiler version, the owning project's version, the line |
| `$project.id`, `.version`, `.target.sys` | the root project being built |
| `$bin.name`, `$bin.kind` | the artifact being built |

`$if (cond) { } $or (cond) { } $or { }` selects at compile time, and untaken arms are not even resolved, so per-target code and imports live in them. A gate is `u1`. Comptime `&&` and `||` short-circuit in a gate: the right operand is not read when the left decides, so a question valid on one architecture only (`$mach.build.ext.sha`) may follow the architecture test in one gate. `$error("msg")` refuses the build where it is reached. A `$` parameter is a compile-time argument, named bare in the body, giving one instance per value. `va: ...` is a pack: `$each a in va { }` unrolls it, `va.len` counts it, `g(va...)` forwards it. Runtime C-style variadics exist only on `ext fun`.

```mach
$if ($mach.build.sys == $mach.sys.linux) {
    val PLATFORM: *u8 = "linux";
}
$or ($mach.build.sys == $mach.sys.windows) {
    val PLATFORM: *u8 = "windows";
}
$or {
    $error("unsupported system");
}

$if ($mach.build.arch == $mach.arch.x86_64) {
    $if ($mach.build.ext.sha == 1) { val HAS_SHA: u1 = 1; }
    $or                            { val HAS_SHA: u1 = 0; }
}
$or {
    val HAS_SHA: u1 = 0;
}

fun apply($mode: u8, n: i64) i64 {
    $if (mode == 0) { ret n + n; }
    $or             { ret n * n; }
}

fun sum(va: ...) i64 {
    var t: i64 = 0;
    $each a in va {
        $if ($type_of(a) == i64) { t = t + a; }
        $or                      { t = t + a::i64; }
    }
    ret t;
}

fun demo() i64 {
    ret apply(1, 3) + sum(1i64, 2u8, 3i32) + $size_of([4]u32);
}
```

Intrinsics: `$size_of(T)`, `$align_of(T)`, `$length_of(T or binding)`, `$offset_of(T, field)` give untyped integers that take the binding's width. `$type_of(x)`, `$type_name(T)`, `$type_id(T)`, `$pointee_of(T)`, `$discriminant_of(T)`. Predicates `$is_record` `$is_union` `$is_tag` `$is_pointer` `$is_integer` `$is_float` `$is_secret` `$holds_secret`. `$each f in $fields(T) { v.[f] }` and `$each c in $cases(T) { sel v.[c] }` reflect. `$zeroize(p, n)` is a wipe no pass removes. There is no `$assert`: write `$if (!cond) { $error("msg"); }`.

## Attributes

`#[name]` or `#[name(args)]` before a declaration, one per line or several on a line. Arguments are constants (strings for names and modes, `val`s allowed). A v6 spelling is refused naming its v7 one.

| Attribute | On | Meaning |
|---|---|---|
| `symbol("n")` | fun, val, var | linker name (C prefix added per target) |
| `export` | pub fun, val, var | export from a shared library |
| `library("key")` | ext | the `[link.key]` entry an import comes from (needed on PE and Mach-O) |
| `weak("n")` | fun | a default a strong `n` replaces at link |
| `inline("always")`, `inline("never")` | fun | force or forbid inlining |
| `vectorize("never")` | fun | no auto-vectorization |
| `naked` | fun | body of `asm` only, no prologue or return |
| `noreturn` | fun, ext, asm statement | control never comes back |
| `constant_time("soft")`, `("hard")` | fun, def of a fun type | constant-time boundary, see below |
| `extension("sha", ...)` | fun | allow extensions the target lacks (caller checks the CPU) |
| `align(N)` | fun, rec, uni, tag, val, var | raise alignment |
| `packed` | rec, uni, tag | no padding (a packed field's address is refused) |
| `volatile` | rec, uni, tag | every access to the type is volatile |
| `section(".s")`, `at(offset)` | fun, val, var (`at` not fun) | named section, offset in the image |
| `thread` | module var | one copy per thread |
| `embed("path")` | val `[_]u8` or `[N]u8` | file bytes as the initializer |
| `deprecated`, `deprecated("msg")` | declarations, tag cases | warn at uses from other modules |
| `testing` | declarations | exists only for tests |
| `expect("key")` | fun, rec, uni, tag, val, var, test | acknowledge a warning |
| `intrinsic`, `handle`, `abi_type("va_list")` | fun, def | target instructions and types |
| `gpu.stage`, `gpu.workgroup`, `gpu.input`, `gpu.output`, `gpu.builtin`, `gpu.uniform`, `gpu.storage`, `gpu.sampler`, `gpu.attachment`, `gpu.push`, `gpu.spec`, `gpu.shared` | shader declarations | SPIR-V interface, see `doc/language/gpu.md` |

## Secrecy and constant time

`^T` marks constant-time data, not confidential data: nothing is wiped or zeroed for it. In every function a secret may not reach a branch or loop condition, the left side of `&&`/`||`, an index or address, `/`, `%` or float arithmetic, and inside a `#[constant_time]` function the build also proves the emitted code keeps it from branches, addresses and variable-time instructions and requires every callee given a secret to be constant time ("hard" refuses a target that cannot guarantee it). `:>T` declassifies, and `mach check --declassified` lists every one.

```mach
#[constant_time("hard")]
fun ct_eq(a: ^[8]u8, b: ^[8]u8) ^u8 {
    var d: ^u8 = 0;
    var i: u64 = 0;
    for (i < 8) {
        d = d | (a[i] ^ b[i]);
        i = i + 1;
    }
    ret d;
}

fun same(a: ^[8]u8, b: ^[8]u8) u1 {
    ret (ct_eq(a, b):>u8) == 0;
    # wrong: if (ct_eq(a, b) == 0) { ret 1; }   -> secret.branch
}
```

## Inline assembly

`asm <selection> { }` takes a mandatory instruction-set head in the assembler's syntax (`x86-64`, `x86-64,avx2`, `armv8-a`, `rv64i`), read for the compile target, so `$if` picks one block per architecture. The body uses its dialect's comments (`#` on x86 and RISC-V, `//` on aarch64). `{name}` substitutes a local, parameter or constant, placed in a register, memory or an immediate as the instruction allows, so load a pointer into a register before dereferencing it, never `[{p}]`. Clobbers and memory effects are inferred. Labels are numbered (`1:`, `1b`).

```mach
pub fun add_asm(a: i64, b: i64) i64 {
    var r: i64 = 0;
    $if ($mach.build.arch == $mach.arch.x86_64) {
        asm x86-64 {
            mov rax, {a}
            add rax, {b}
            mov {r}, rax
        }
    }
    $or {
        r = a + b;
    }
    ret r;
}
```

## Foreign functions and linking

`ext fun` declares a C-ABI function with no body, `#[symbol]` renames it, and `...` after a fixed parameter makes it C-variadic, where narrow integers and `f32` need explicit casts. An artifact names `[link.X]` entries in its `link` array (`link = ["c"]`), and `-l name` and paths on the command line add inputs. System libraries on linux need `env = "gnu"` on the target.

```mach
use std.runtime;

#[symbol("write")]
ext fun libc_write(fd: i64, buf: *u8, n: i64) i64;
#[library("c")]
ext fun strlen(s: *u8) i64;

#[symbol("main")]
fun main(argc: i64, argv: **u8) i64 {
    val msg: *u8 = "hello from c\n";
    libc_write(1, msg, strlen(msg));
    # prints: hello from c
    ret 0;
}
```

```toml
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
```

## Tests

`test name { }` sits in any module the artifact's entry reaches, named `subject__case` by convention, in a namespace of its own. The body returns an exit status: `0` or falling off the end passes, `77` skips, any other value up to 255 fails, so distinct codes name the failing check. A host-specific test sits in a `$if`. Fixtures are `#[testing]`. There are no assertion intrinsics.

```mach
use std.types.bool.bool;

fun is_leap(y: i64) bool {
    if (y % 400 == 0) { ret 1; }
    if (y % 100 == 0) { ret 0; }
    ret y % 4 == 0;
}

test is_leap__centuries {
    if (!is_leap(2000)) { ret 1; }
    if (is_leap(1900))  { ret 2; }
}

test is_leap__needs_a_feature {
    ret 77;
}
```

`mach test .` runs each test as its own process and reports per module. Flags: `--filter <substr>`, `--list`, `--include-deps`, `--threads <n>`, `--timeout <dur>` (default 60s), `--runner <cmd>`. A test's name in output is `<module>#<name>`.

## std idioms

| Need | Use |
|---|---|
| print | `std.print`: `print`, `println`, `eprintln`, `printf("x={}\n", v)`, `printlnf`. Holes `{}`, `{:x}` `{:X}` `{:b}` `{:c}` `{:8}` `{:08}` `{:<8}`, braces `{{` `}}`. A bare literal in a pack is `i64` or `f64`, so suffix any other |
| outcomes | `std.types.result.res`, `std.types.option.opt`, `std.types.error.err` |
| allocation | `std.allocator` (`A.Allocator`, `allocate[T]`, `deallocate[T]`), backings `std.allocator.page.make`, `arena`, `heap`, `bump`, `fixed` |
| growable array | `std.collections.vector` (`init`, `push`, `get`, `length`, `dnit`), `Slice[T]` in `std.collections.slice` |
| memory | `std.memory`: `copy[T]`, `move[T]`, `fill[T]`, `zero[T]`, `equal[T]` |
| strings | `std.types.string`: `str_len`, `str_equals`, `str_compare`, `str_find`, `str_copy(?a, s)` |
| arguments | `std.args`: `args.of(argc::usize, argv)`, `args.get(a, i)` gives `opt[str]` |
| abort | `std.system.panic.panic("msg")` exits 255 |

```mach
use A: std.allocator;
use std.allocator.page;
use std.collections.vector;
use std.collections.vector.Vector;
use std.print;
use std.runtime;
use std.types.error.err;
use std.types.option.opt;
use std.types.result.res;
use std.types.size.usize;

#[symbol("main")]
fun main(argc: i64, argv: **u8) i64 {
    var a:    A.Allocator;
    val made: err[A.Error] = page.make(?a);
    if (sel made.err) { ret 1; }
    var v: Vector[i64] = vector.init[i64](?a);
    fin { vector.dnit[i64](?v); }
    # wrong: fin { v.dnit(); }   -> field.unknown
    var i: i64 = 0;
    for (i < 4) {
        val pushed: res[usize, A.Error] = vector.push[i64](?v, i * i);
        if (sel pushed.err) { ret 2; }
        i = i + 1;
    }
    val third: opt[*i64] = vector.get[i64](?v, 2);
    if (!sel third.some) { ret 3; }
    print.printlnf("len {} third {} hex {:x}", vector.length[i64](?v), @third.some, 255u32);
    # prints: len 4 third 4 hex ff
    ret 0;
}
```

## Commands

```sh
mach check .                     # parse, resolve, type-check: the fast loop
mach build .                     # compile and link every selected cell
mach build . -t linux-aarch64 -p release
mach run . -- arg1 arg2          # run what build wrote. never builds
mach test . --filter is_leap
mach fmt .                       # canonical layout, no options. --check for CI
mach dep pull .                  # realize dep/ at its pins. build never fetches
mach check . --report lines      # one tab-separated record per diagnostic, for scripts
mach build tool.mach --source std=dep/std   # one file outside a project
```

A build never touches `dep/`: a missing dependency is refused naming `mach dep pull .`, and only `mach dep update` moves a pin. `mach run` after an edit runs the old binary until the next build. Flags of 6 (`--jobs`, `-O2`, `-g`) are refused naming their replacements (`--threads`, `-O`, `--debug`). Diagnostics carry a key such as `error[tag.payload_unguarded]` and a `@ file:line:col` position.
