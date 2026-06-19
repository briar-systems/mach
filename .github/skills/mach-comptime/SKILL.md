---
name: mach-comptime
description: Use when writing or reviewing Mach code that touches the comptime channel: the $ namespace, conditional compilation, target/compiler/build/project/source queries ($mach.*), backtick codegen decorators (symbol/library/inline/align/section), compiler intrinsics ($size_of/$align_of/$offset_of/$error/$assert), $if/$or comptime control flow, and comptime function parameters ($name: T).
---

# Mach comptime channel

`$` opens the compiler-owned comptime channel — read-only: it *selects and
expands*, never executes or mutates. Three structurally distinct shapes — the
parser disambiguates by shape, not by a mode flag:

| Shape | Meaning |
|---|---|
| `$mach.path.to.value` | read compiler-owned value |
| `$sym(args)` | comptime call — closed intrinsic set, or a user call passing comptime args (no user comptime defs) |
| `$if` / `$or` | comptime control flow |

`mach` is reserved at the top of `$`; user symbols cannot collide there.
Per-declaration codegen directives are backtick decorators (a separate
mechanism — see Decorators below), not `$` writes.

## `$mach.*` — read-only compiler state

All reads, all comptime constants. Closed tree. Subtrees:

```mach
$mach.build.os                  # live; compare against $mach.os.* tags
$mach.build.arch                # live; compare against $mach.arch.* tags
$mach.build.abi                 # live; compare against $mach.abi.* tags
$mach.build.pointer_width       # live; integer byte count
$mach.build.mode                # live; compare against $mach.mode.* tags
$mach.build.timestamp           # stub
$mach.build.git.commit          # stub
$mach.build.git.dirty           # stub
$mach.build.host                # stub

$mach.version                   # live; version string, e.g. "2.0.0"
$mach.version.major             # live; integer component
$mach.version.minor             # live
$mach.version.patch             # live
$mach.compiler.name             # live
$mach.compiler.version          # live

$mach.project.name              # stub
$mach.project.version           # stub
$mach.project.root              # stub

$mach.source.file               # stub
$mach.source.line               # stub
$mach.source.module             # stub
$mach.source.function           # stub

$mach.os.linux $mach.os.darwin $mach.os.windows $mach.os.freestanding
$mach.arch.x86_64 $mach.arch.aarch64
$mach.abi.sysv $mach.abi.win64 $mach.abi.aapcs64
$mach.mode.debug $mach.mode.release
```

> **Implementation status.** The resolved-build facts
> `$mach.build.{os,arch,abi,pointer_width,mode}`, the tag tables
> `$mach.{os,arch,abi,mode}.*`, `$mach.version[.{major,minor,patch}]`, and
> `$mach.compiler.{name,version}` are live. `$mach.build.{timestamp,host,git.*}`,
> `$mach.project.*`, and `$mach.source.*` are reserved stubs — reading one is a
> compile error. `$mach.arch.aarch64` resolves as a value; the aarch64 dispatch
> branches below illustrate the multi-arch pattern.

Tag comparison is path-value — no `.id` suffix, no unwrapping:

```mach
$if ($mach.build.os == $mach.os.linux) { ... }
$if ($mach.build.arch == $mach.arch.x86_64) { ... }
```

`$mach.os.*` / `$mach.arch.*` are closed lists; a tag exists only because the
compiler can target it. A `$mach.*` read can fold into a runtime binding (no
type inference — the binding still declares its type):

```mach
pub val IS_LINUX: u8  = $mach.build.os == $mach.os.linux;
pub val COMPILER: *u8 = $mach.compiler.name;
```

## Decorators — backtick codegen directives

Per-declaration codegen directives are **backtick decorators** (#1476), on the
line(s) above the decl — not `$` writes. The `$sym.attr = value;` setter form was
removed in v2.0.0. Closed set:

| Decorator | Applies to | Argument | Purpose |
|---|---|---|---|
| `` `symbol(str)` `` | functions, vars | `*u8` literal | linker name override |
| `` `library(str)` `` | `ext` functions | `*u8` literal | PE import DLL routing |
| `` `inline` `` | functions | none | strong inline hint |
| `` `align(expr)` `` | vars, records, unions | comptime int | alignment in bytes |
| `` `section(str)` `` | functions, vars | `*u8` literal | linker section name |

```mach
`symbol("read")`
ext fun read(fd: i32, buf: *u8, n: u64) i64;

`align(64)`
pub var cache_line: u8 = 0;
```

Each directive is its own backtick pair; stack several on one decl (e.g.
`` `library("ws2_32.dll")` `` `` `symbol("socket")` ``). There is **no** `$`-write
setter and **no** decl-attached prefix sugar (`$inline pub fun ...` does not
exist). See `doc/language/decorators.md` for argument rules and applicability.

## Intrinsics — `$name(args)`

Closed compiler-shipped set; same call shape as user comptime calls.

Value intrinsics resolve to comptime constant **unsigned integers**. Storage
type is whatever the binding declares (no compiler-known `usize`):

```mach
$size_of(T)             # byte size of T
$align_of(T)            # byte alignment of T
$offset_of(T, field)    # byte offset of T's field

pub val POINT_SIZE: i64 = $size_of(Point);
pub val POINT_X:    i64 = $offset_of(Point, x);
```

Diagnostic intrinsics operate at compile time:

```mach
$error("msg")           # fails compilation
$assert(cond, "msg")    # sugar: $if (!cond) { $error("msg"); }

$assert($size_of(i64) == 8, "expected 64-bit i64");
```

> **Implementation status.** `$error` / `$assert` parse but are not yet
> evaluated — a failing directive is currently accepted rather than aborting the
> build. The shapes above describe the intended behavior.

Runtime-instruction emitters (`trap`, `fence`, `pause`, …) are **not**
intrinsics — they live in stdlib as functions with per-arch `asm` bodies. The
intrinsic set is closed; new ones need a compiler patch.

## `$if` / `$or` — comptime control flow

```mach
$if (cond) { ... }
$or (cond) { ... }
$or { ... }             # comptime else — no condition
```

Only the taken branch compiles. Discarded branches are **not** resolved,
type-checked, or emitted — names inside them are never looked up. This is what
makes per-arch `asm` blocks safe: the untaken block may reference registers a
backend doesn't know.

`cond` must be comptime: `$mach.*` reads, or comparisons of comptime constants.

```mach
$if ($mach.build.os == $mach.os.linux) {
    use std.runtime.linux;
}
$or ($mach.build.os == $mach.os.windows) {
    use std.runtime.windows;
}
$or {
    $error("unsupported OS");
}
```

Multi-arch `asm` dispatch happens at the outer `$if` level; there is no nested
arch-block construct.

## Comptime function parameters — `$name: T`

A comptime value parameter sits in the regular paren list as `$name: T`. The
caller must pass a comptime-evaluable value; the compiler enforces it at the
call site. Inside the body, reference it by bare `name` (no `$`). This is the
idiom for passing memory orderings and similar variants into stdlib wrappers.

```mach
pub fun load($order: Order, ptr: *i64) i64 {
    var result: i64 = 0;

    $if ($mach.build.arch == $mach.arch.aarch64) {
        $if (order == RELAXED) {
            asm aarch64 { ldr {result}, [{ptr}] }
        }
        $or (order == ACQUIRE) {
            asm aarch64 { ldar {result}, [{ptr}] }
        }
    }

    ret result;
}
```

> **Implementation status.** The `$name: T` parameter is accepted in a
> signature, but it is not yet usable as a comptime constant inside the body —
> `$if (order == RELAXED)` reports that the identifier is not a comptime
> constant in scope. Per-call-site dispatch requires monomorphizing the body per
> distinct value, which is not yet done. Until it lands, branch on such a
> parameter with a runtime `if`.

The `$name: T` form applies to **function parameters only** — not record
fields, not generic-bracket contexts. Generic *type* parameters use `[T]`
brackets and are a separate mechanism.

## Pitfalls

- **Tag comparison is path-value.** Write `$mach.build.os == $mach.os.linux`,
  never an `.id` suffix or any unwrap.
- **No `$or $if`.** The else arm is bare `$or { ... }` with no condition. A
  conditional alternative is `$or (cond) { ... }`.
- **Codegen directives are backtick decorators, not `$` writes.** Use
  `` `symbol("...")` `` / `` `inline` `` / `` `align(N)` `` on the line above the
  decl; the `$sym.attr = value;` setter form was removed in v2.0.0.
- **No prefix sugar.** `$inline pub fun ...` does not exist — a decorator sits on
  its own line above the decl.
- **Closed sets.** Intrinsic names, decorator names, and `$mach.os.*`/`.arch.*`
  tags are all closed. Don't invent members; new ones require a compiler change.
- **Value intrinsics are unsigned constants with no implicit type.** The binding
  declares the storage type (Mach has no type inference and no `usize`).
- **Comptime param is referenced bare in the body.** Declared `$order: Order`,
  used as `order`.
- **Not in the channel:** no `$<Type>.*` reflection subtree (types aren't
  first-class comptime values), no comptime function definitions, no comptime
  loops.
- **`$if` vs runtime `if`.** `$if` discards untaken branches entirely at compile
  time; runtime `if`/`or` emits a runtime branch. Use `$if` for conditional
  compilation, never to fake reflection over untaken code.

## Reference

The authoritative comptime reference lives in the Mach repository under
[`doc/language/`](https://github.com/octalide/mach/tree/dev/doc/language) —
`comptime.md`, `comptime-mach.md`, `comptime-attrs.md`,
`comptime-intrinsics.md`, and `comptime-control.md`. When a skill and the
reference disagree, the reference wins.
