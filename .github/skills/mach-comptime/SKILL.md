---
name: mach-comptime
description: Use when writing or reviewing Mach code that touches the comptime channel: the $ namespace, conditional compilation, target/build/project/source queries ($mach.*), symbol attributes ($sym.attr), compiler intrinsics ($size_of/$align_of/$offset_of/$error/$assert), $if/$or comptime control flow, and comptime function parameters ($name: T).
---

# Mach comptime channel

`$` opens the single bidirectional channel between developer and compiler. Reads
observe compiler-provided state; writes annotate developer-declared symbols.
Four structurally distinct shapes — the parser disambiguates by shape, not by a
mode flag:

| Shape | Meaning |
|---|---|
| `$mach.path.to.value` | read compiler-owned value |
| `$sym.attr = value` / `$sym.attr` | write/read attribute on a declared symbol |
| `$sym(args)` | comptime call — closed intrinsic set, or a user call passing comptime args (no user comptime defs) |
| `$if` / `$or` | comptime control flow |

`mach` is reserved at the top of `$`; user symbols cannot collide there.

## `$mach.*` — read-only compiler state

All reads, all comptime constants. Closed tree. Subtrees:

```mach
$mach.target.os                 # compare against $mach.os.* tags
$mach.target.arch               # compare against $mach.arch.* tags
$mach.target.abi
$mach.target.pointer_width      # integer byte count

$mach.build.mode
$mach.build.timestamp
$mach.build.git.commit
$mach.build.git.dirty
$mach.build.host

$mach.project.name
$mach.project.version
$mach.project.root

$mach.source.file
$mach.source.line
$mach.source.module
$mach.source.function

$mach.os.linux $mach.os.darwin $mach.os.windows $mach.os.freestanding
$mach.arch.x86_64 $mach.arch.aarch64
```

Tag comparison is path-value — no `.id` suffix, no unwrapping:

```mach
$if ($mach.target.os == $mach.os.linux) { ... }
$if ($mach.target.arch == $mach.arch.x86_64) { ... }
```

`$mach.os.*` / `$mach.arch.*` are closed lists; a tag exists only because the
compiler can target it. A `$mach.*` read can fold into a runtime binding (no
type inference — the binding still declares its type):

```mach
pub val IS_LINUX: u8  = $mach.target.os == $mach.os.linux;
pub val VERSION:  *u8 = $mach.project.version;
```

## Symbol attributes — `$sym.attr`

Metadata written onto a declared symbol; the compiler reads it during codegen.

```mach
$sym.attr = value;          # write
$sym.attr                   # read
```

Rules:

- `sym` must be declared **in the same module** (resolver verifies).
- Path is flat: exactly one symbol component, one attribute component. No nesting.
- Writes are write-once; RHS must be comptime-evaluable.
- `attr` must be from the closed set below.
- Convention: write the attribute **before** the decl so it reads as a header.
  Purely lexical — order doesn't affect correctness (resolution is module-scope).

```mach
$panic.noreturn = true;
pub fun panic(msg: *u8) {
    asm x86_64 { ud2 }
}
```

Closed attribute set:

| Attribute | Applies to | Value | Purpose |
|---|---|---|---|
| `.symbol` | functions, vars | `*u8` literal | linker name override |
| `.noreturn` | functions | `u8` flag | never returns; dead code after calls omitted |
| `.inline` | functions | `u8` flag | strong inline hint |
| `.align` | vars, records, unions | power-of-two int | alignment in bytes |
| `.section` | functions, vars | `*u8` literal | linker section name |
| `.packed` | records, unions | `u8` flag | disable field padding |

The `ext fun` rename uses `.symbol`:

```mach
$read.symbol = "read";
ext fun read(fd: i32, buf: *u8, n: u64) i64;
```

There is **no** decl-attached prefix sugar (`$inline pub fun ...` does not
exist). Always use an attribute write.

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

`cond` must be comptime: `$mach.*` reads, comparisons of comptime constants, or
comptime function parameters.

```mach
$if ($mach.target.os == $mach.os.linux) {
    use full.os.linux;
}
$or ($mach.target.os == $mach.os.windows) {
    use full.os.windows;
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
call site. Inside the body, reference it by bare `name` (no `$`) — `$if` then
dispatches on it. This is the idiom for passing memory orderings and similar
variants into stdlib.

```mach
pub fun load($order: Order, ptr: *i64) i64 {
    var result: i64 = 0;

    $if ($mach.target.arch == $mach.arch.aarch64) {
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

The `$name: T` form applies to **function parameters only** — not record
fields, not generic-bracket contexts. Generic *type* parameters use `[T]`
brackets and are a separate mechanism.

## Pitfalls

- **Tag comparison is path-value.** Write `$mach.target.os == $mach.os.linux`,
  never an `.id` suffix or any unwrap.
- **No `$or $if`.** The else arm is bare `$or { ... }` with no condition. A
  conditional alternative is `$or (cond) { ... }`.
- **Attribute symbol must be local.** `$sym.attr` only attaches to a symbol
  declared in the same module; writes are write-once.
- **No prefix-sugar attributes.** `$inline`/`$noreturn` as decl prefixes do not
  exist — use `$sym.inline = true;` / `$sym.noreturn = true;` before the decl.
- **Closed sets.** Intrinsic names, attribute names, and `$mach.os.*`/`.arch.*`
  tags are all closed. Don't invent members; new ones require a compiler change.
- **Value intrinsics are unsigned constants with no implicit type.** The binding
  declares the storage type (Mach has no type inference and no `usize`).
- **Comptime param is referenced bare in the body.** Declared `$order: Order`,
  used as `order` inside `$if`.
- **Not in the channel:** no `$<Type>.*` reflection subtree (types aren't
  first-class comptime values), no comptime function definitions, no comptime
  loops.
- **`$if` vs runtime `if`.** `$if` discards untaken branches entirely at compile
  time; runtime `if`/`or` emits a runtime branch. Use `$if` for conditional
  compilation, never to fake reflection over untaken code.

## Reference

- [doc/language/comptime.md](../../../doc/language/comptime.md) — channel overview, four shapes
- [doc/language/comptime-mach.md](../../../doc/language/comptime-mach.md) — `$mach.*` namespace
- [doc/language/comptime-attrs.md](../../../doc/language/comptime-attrs.md) — symbol attributes
- [doc/language/comptime-intrinsics.md](../../../doc/language/comptime-intrinsics.md) — intrinsics
- [doc/language/comptime-control.md](../../../doc/language/comptime-control.md) — `$if` / `$or`
- [doc/language/](../../../doc/language/README.md) — full language reference index
