# Documentation

Mach source-level documentation uses `#` comments immediately above
declarations. Each docstring is one summary followed by an optional
component block. `mach doc` renders them, and the compiler's docstring lint
checks the component block of every `pub fun`, `pub rec`, `pub uni`, and `pub tag`
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
| Tag case | case name |

## What the lint checks

For a `pub fun`, `pub rec`, `pub uni`, or `pub tag` whose docstring has a component
block, every component line must name an element of that declaration, carry
a description, and appear in declaration order (generics, then parameters,
fields, or cases, then `ret` for functions). Each violation is a warning naming the line:

```
documented component matches no parameter, field, case, generic, or `ret` of this declaration
documented component has no description
documented components are out of declaration order
```

Tags have no return value, so a `ret:` component line on a tag is rejected. A bare
generic name without brackets (such as `T:` instead of `[T]:`) is also rejected.

A summary-only docstring, a docstring on a `val`, `var`, `def`, `use`, or
`fwd` (which are summary-only forms), and a non-`pub` declaration are not
checked. A component block need not be complete: an element with no line is
not a warning, a line with no element is. `mach doc` renders every `pub`
declaration whether or not it is documented.

## Placement

The lexer folds line-adjacent `#` lines that each start their own line into
one run, and a run becomes a declaration's docstring when it ends on the line
directly above the declaration or directly above the declaration's `#[...]`
decorators. A blank line between the run and the declaration breaks the
attachment, and a decorator line is not a comment, so it never joins a run. A
comment that shares its line with code (`val x: i32 = 1; # note`) is not
documentation: it neither starts a run, nor joins the comment on the next
line, nor attaches to the declaration below it. The docstring is therefore
the first thing above the declaration, with decorators between it and the
declaration:

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

```mach accept
# a 2D Cartesian point with i64 coordinates
# ---
# x: horizontal coordinate
# y: vertical coordinate
pub rec Point { x: i64; y: i64; }
```

```mach accept
# holds either an integer or a float
# ---
# i: integer interpretation
# f: float interpretation
pub uni Number { i: i64; f: f64; }
```

```mach accept
# an i64 representing years since birth
pub def Age: i64;
```

## Tag

```mach accept
# a value that may be absent
# ---
# [T]: value type
# none: empty case
# some: payload case
pub tag Maybe[T]: u8 {
    none;
    some: T;
}
```

```mach accept
# a unit outcome with a typed failure
# ---
# [E]: error type
# err: failure case
# ok: success case
pub tag Outcome[E]: u8 {
    err: E;
    ok;
}
```

Cases appear in declaration order after generic parameters. Tags have no return
value, so `ret:` is refused. Undocumented cases are permitted, but any documented
case must exist on the tag; `doclint` warns at the component otherwise:

```mach warn "documented component matches no parameter, field, generic, or `ret`"
# a unit outcome with a typed failure
# ---
# [E]: error type
# err: failure case
# done: no such case
pub tag Outcome[E]: u8 {
    err: E;
    ok;
}
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

- [fun.md](fun.md) - function declaration grammar
- [rec.md](rec.md), [uni.md](uni.md), [tag.md](tag.md), [def.md](def.md) - type forms
- [val-var.md](val-var.md) - binding declarations
- [modules.md](modules.md) - module structure and file layout
- [../cli.md](../cli.md#mach-doc) - mach doc command, which renders docstrings
