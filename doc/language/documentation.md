# Documentation

Mach source-level documentation uses `#` comments immediately above
declarations. Each docstring is one summary line followed by an optional
component block.

## Grammar

```
# <name>: <summary>.
# ---
# <component>: <description>
# <component>: <description>
```

- **Summary line.** `<name>` is the declared identifier; for modules it
  is the full dotted path. `<summary>` is a single sentence ending in a
  period.
- **Separator.** `# ---` is present when one or more component lines
  follow; absent when the docstring is summary-only.
- **Component lines.** Each is `# <component>: <description>`. One line
  per element exposed by the declaration.
- Prose is lowercase except for proper nouns and type names.

## Component identifiers

| Declaration element | Component identifier |
|---|---|
| Function parameter | parameter name |
| Comptime parameter | the `$name` form |
| Generic type parameter | the `[T]` form |
| Return value | `ret` |
| Record field | field name |
| Union variant | variant name |

## Placement

The docstring is the first thing above the declaration. Attribute writes
follow the docstring, immediately above the decl:

```mach
# panic: terminate the program with a message.
# ---
# msg: text to emit before terminating
$panic.noreturn = true;
pub fun panic(msg: *u8) { ... }
```

## Function

```mach
# realtime: read the wall-clock time.
# ---
# out: pointer to Timespec to populate
# ret: 0 on success, negative errno on failure
pub fun realtime(out: *Timespec) i64 { ... }
```

Generic and comptime parameters appear in the component block under their
syntactic form:

```mach
# load: atomic load through a typed pointer.
# ---
# [T]:    element type
# $order: memory ordering constraint
# ptr:    pointer to load from
# ret:    loaded value
pub fun load[T]($order: Order, ptr: *T) T { ... }
```

Summary-only — no separator, no component block:

```mach
# spin_hint: yield the CPU to other threads.
pub fun spin_hint() { ... }
```

## Record / union / def

```mach
# Point: a 2D Cartesian point with i64 coordinates.
# ---
# x: horizontal coordinate
# y: vertical coordinate
pub rec Point { x: i64; y: i64; }
```

```mach
# Number: holds either an integer or a float.
# ---
# i: integer interpretation
# f: float interpretation
pub uni Number { i: i64; f: f64; }
```

```mach
# Age: an i64 representing years since birth.
pub def Age: i64;
```

## Module

A `.mach` file begins with a module docstring as the first content in the
file — before any `use`, `fwd`, or attribute write. The summary uses the
module's full dotted path as `<name>`.

```mach
# myproj.system.os: cross-platform OS interface.
#
# forwards the portable intersection of all supported targets.
# for platform-specific functionality, import the target module
# directly (e.g. myproj.system.os.linux).
```

Modules may extend beyond the summary with additional paragraphs
separated by blank `#` lines. Other declaration kinds do not extend
beyond the summary and the component block.

## Value

```mach
# MAX: maximum counter value before saturation.
pub val MAX: i64 = 100;

# calls: module-local request counter.
var calls: i64 = 0;
```

## See also

- [fun.md](fun.md) — function declaration grammar
- [rec.md](rec.md), [uni.md](uni.md), [def.md](def.md) — type forms
- [val-var.md](val-var.md) — binding declarations
- [modules.md](modules.md) — module structure and file layout
