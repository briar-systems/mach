# Documentation

Mach source-level documentation uses `#` comments immediately above
declarations. Each docstring is one summary followed by an optional
component block. `mach doc` renders them, and the compiler's docstring lint
checks the component block of every `pub fun`, `pub rec`, and `pub uni`
against the declaration it documents.

A docstring states **what** a declaration is and **how** it is used. Why a
design is the way it is belongs in `doc/design/`; when something changed
belongs in the changelog. Documentation never changes generated code.

## Grammar

```
# <summary>
# ---
# <component>: <description>
# <component>: <description>
```

- **Summary.** `<summary>` is a single sentence by convention. Additional
  paragraphs may follow, separated by blank `#` lines, and the summary runs
  to the separator or to the end of the docstring.
- **Separator.** `# ---` is present when one or more component lines
  follow; absent when the docstring is summary-only.
- **Component lines.** Each is `# <component>: <description>`. One line
  per element the declaration exposes, in declaration order. A component
  head has zero or one space between `#` and the name; a line with two or
  more spaces after `#` is a continuation of the previous description, which
  is how a long description wraps.
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

## What the lint checks

For a `pub fun`, `pub rec`, or `pub uni` whose docstring has a component
block, every component line must name an element of that declaration, carry
a description, and appear in declaration order (generics, then parameters or
fields, then `ret`). Each violation is a warning naming the line:

```
documented component matches no parameter, field, generic, or `ret` of this declaration
documented component has no description
documented components are out of declaration order
```

A summary-only docstring, a docstring on a `val`, `var`, `def`, `use`, or
`fwd` (which are summary-only forms), and a non-`pub` declaration are not
checked. A component block need not be complete: an element with no line is
not a warning, a line with no element is. `mach doc` renders every `pub`
declaration whether or not it is documented.

## Placement

The lexer folds line-adjacent `#` lines into one run, and a run becomes a
declaration's docstring when it ends on the line directly above the
declaration or directly above the declaration's `#[...]` decorators. A blank
line between the run and the declaration breaks the attachment, and a
decorator line is not a comment, so it never joins a run. The docstring is
therefore the first thing above the declaration, with decorators between it
and the declaration:

```mach
# terminate the program with a message
# ---
# msg: text to emit before terminating
#[symbol("panic")]
pub fun panic(msg: *u8) { ... }
```

## Function

```mach
# read the wall-clock time
# ---
# out: pointer to Timespec to populate
# ret: 0 on success, negative errno on failure
pub fun realtime(out: *Timespec) i64 { ... }
```

Generic and comptime parameters appear in the component block under their
syntactic form:

```mach
# atomic load through a typed pointer
# ---
# [T]:    element type
# $order: memory ordering constraint
# ptr:    pointer to load from
# ret:    loaded value
pub fun load[T]($order: Order, ptr: *T) T { ... }
```

Summary-only, with no separator and no component block:

```mach
# yield the CPU to other threads
pub fun spin_hint() { ... }
```

## Record / union / def

```mach
# a 2D Cartesian point with i64 coordinates
# ---
# x: horizontal coordinate
# y: vertical coordinate
pub rec Point { x: i64; y: i64; }
```

```mach
# holds either an integer or a float
# ---
# i: integer interpretation
# f: float interpretation
pub uni Number { i: i64; f: f64; }
```

```mach
# an i64 representing years since birth
pub def Age: i64;
```

## Module

A `.mach` file begins with a module docstring as the first content in the
file, before any `use`, `fwd`, or decorator.

```mach
# cross-platform OS interface
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
# maximum counter value before saturation
pub val MAX: i64 = 100;

# module-local request counter
var calls: i64 = 0;
```

## Comments that are not docstrings

A comment that is not directly above a declaration is an ordinary comment.
Comments are brief, lowercase, and single-line, and appear only where the
code is not self-evident. There are no sectional or separator comments. A
line comment that begins `#[` with no space opens a decorator; write such a
comment as `# [...]`.

## See also

- [fun.md](fun.md) — function declaration grammar
- [rec.md](rec.md), [uni.md](uni.md), [def.md](def.md) — type forms
- [val-var.md](val-var.md) — binding declarations
- [modules.md](modules.md) — module structure and file layout
- [../cli.md](../cli.md#mach-doc) — `mach doc`, which renders docstrings
