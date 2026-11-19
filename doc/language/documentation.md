# Documentation

Mach source-level documentation uses `#` comments immediately above
declarations. Each docstring is one summary followed by an optional
component block. `mach doc` renders them, and the compiler's docstring lint
checks the component block of every `pub fun`, `pub rec`, `pub uni`, and `pub tag`
against the declaration it documents.

A docstring states **what** a declaration is and **how** it is used, and a
module's docstring states the contract the module holds its callers to. The
language, command and manifest references under `doc/` say what the user
sees; when something changed belongs in the changelog. Documentation never
changes generated code.

## Grammar

```
# <summary>
# ---
# <component>: <description>
# <component>: <description>
```

- **Summary.** `<summary>` opens the docstring's first paragraph. Further
  text may follow it in that paragraph, and further paragraphs may follow on
  any docstring, each after a blank `#` line, up to the separator or the end
  of the docstring. `mach doc` shows the summary beside the name wherever it
  lists one (see [What `mach doc` writes](#what-mach-doc-writes)).
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
| Union field | field name |
| Tag case | case name |

## What the lint checks

For a `pub fun`, `pub rec`, `pub uni`, or `pub tag` whose docstring has a component
block, every component line must name an element of that declaration, carry
a description, and appear in declaration order (generics, then parameters,
fields, or cases, then `ret` for functions). Each violation is a warning naming the line:

```
documented component matches no parameter, field, generic, or `ret` of this declaration
documented component has no description
documented components are out of declaration order
```

Tags have no return value, so a `ret:` component line on a tag is warned. A bare
generic name without brackets (such as `T:` instead of `[T]:`) is also warned.

Like every warning, the lint's warnings show for the project being built,
checked or documented, and a dependency's only under `--dep-warnings` (see
[Whose warnings a build shows](manifest.md#whose-warnings-a-build-shows)). The
lint runs only on a module whose warnings the build shows. Its
warnings come with name resolution, so a file whose names are not resolved
in a run is not linted by it, except under `mach doc`, which lints every file
it documents.

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
declaration. A docstring inside a `$if` arm is written at the arm's
indentation:

```mach fragment
# terminate the program with a message
# ---
# msg: text to emit before terminating
#[symbol("panic")]
pub fun panic(msg: *u8) { ... }
```

The comment run that opens a file has one owner. When it ends directly above
the file's first declaration and that declaration is not a `use`, it is that
declaration's docstring: a first `fun`, `rec`, `fwd` or `$if` written directly
under it takes it, and the module has none. Otherwise it is the module
docstring, a `use` directly under it included, since a `use` is never
documented. A blank line after a module docstring keeps it the module's
whatever follows.

A docstring above a `$if` documents the declarations in its arms that carry
none of their own, so one docstring can serve a name every arm declares.

## Function

```mach fragment
# read the wall-clock time
# ---
# out: pointer to Timespec to populate
# ret: 0 on success, negative errno on failure
pub fun realtime(out: *Timespec) i64 { ... }
```

Generic and comptime parameters appear in the component block under their
syntactic form:

```mach fragment
# atomic load through a typed pointer
# ---
# [T]:    element type
# $order: memory ordering constraint
# ptr:    pointer to load from
# ret:    loaded value
pub fun load[T]($order: Order, ptr: *T) T { ... }
```

Summary-only, with no separator and no component block:

```mach fragment
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

## Tag

```mach
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

```mach
# a unit outcome with a typed failure
# ---
# [E]: error type
# err: failure case
# ok:  success case
pub tag Outcome[E]: u8 {
    err: E;
    ok;
}
```

Cases appear in declaration order after generic parameters. Tags have no return
value, so `ret:` is warned. Undocumented cases are permitted, but any documented
case must exist on the tag; `doclint` warns at the component otherwise:

```mach
# a unit outcome with a typed failure
# ---
# [E]: error type
# err:  failure case
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

Like every docstring, a module docstring may continue past its summary with
further paragraphs separated by blank `#` lines.

A file may hold a module docstring and nothing else. Such a module declares
nothing and is never imported for what it holds: it exists to describe the
directory of the same name beside it, such as `src/front.mach` for
`src/front/`, and `mach doc` gives it no page of its own and shows its
docstring as that directory's description.

```mach
# the front end: the lexer, the parser and the syntax tree, module loading and name
# resolution, comptime evaluation, decorators, type checking, and doc comments
```

## Value

```mach
# maximum counter value before saturation
pub val MAX: i64 = 100;

# module-local request counter
var calls: i64 = 0;
```

## What `mach doc` writes

`mach doc <path>` documents the whole project at `<path>` in one run: every
`.mach` file under its source root, each once. It parses the files and runs
nothing more, so the reference is the same on every host, needs no target
(`-t` and `-a` are refused) and is written for a project whose dependencies
are not pulled. A file that does not parse refuses the run. It writes markdown
into `doc/<project id>/` (`<dir>/<project id>/` with `-o <dir>`), a directory
it owns: a page it did not write in the run, such as one whose file is gone,
is removed, and so is a directory left empty. Nothing outside that directory
is written.

- **The project page**, `doc/<id>/README.md`: the project's id and version,
  its artifacts with their kinds and entry modules, its top-level directories
  with the summaries of their root modules, its top-level modules, and the ids
  of its dependencies.
- **A directory page**, `README.md` in each directory of the source tree:
  the directory's root module (the file beside it of the same name, such as
  `src/lower.mach` for `src/lower/`) with its summary and paragraphs when it
  has one, a table of its modules with their summaries, and its
  sub-directories.
- **A module page**, `doc/<id>/<path>.md`, for each module that declares
  anything (a `use` declares nothing): a breadcrumb up the tree, the module's path, its docstring, a line
  linking its source file, a line naming the modules of the project its
  signatures mention, each linked, a map of every public name with its
  summary, and then an entry for each. A type is shown with the functions and
  constants that belong to it: a function belongs to the first of its
  parameters whose type names a type the module declares (through pointers,
  `^`, arrays and generic arguments), or failing that to a type its return
  type names (through `res`, `opt`, `err` and pointers); a constant belongs to
  the type it is declared with. What belongs to no type follows under
  Functions, Constants, Variables and Forwards.

An entry shows its declaration in a code block, a record with its fields and
a function without its body, with its decorators above it as the source has
them. A type the source spells through an import alias is written with the
path of the module the alias names in place of the alias (`*A.Allocator`
reads `*std.allocator.Allocator`), and a name the source writes bare stays
bare; the source's line breaks and alignment are kept. An alias is what the
file's `use` lines bind, a `use` inside a `$if` arm binding under that arm's
condition. Every `$if` arm is read: a name several arms declare is one entry
with a code block per arm. Then comes its whole docstring.

A `$if` chain is tried in order and the first arm whose condition holds is
taken, and the pages say so once rather than repeat it per arm. Where several
arms are shown together (the arms of one entry, the providers of a `fwd`, the
values of a constant), a line states that the first whose condition holds is
taken, and each arm then carries its own condition alone ("where ..."), the
last arm with none reading "otherwise". An arm the run does not show beside the
arms before it also names those it excludes ("and not ..."), so a declaration
standing alone in a later arm reads exactly: "Only where `c` and not `a` nor
`b`". A `#[deprecated]` declaration
has a paragraph beginning **Deprecated:** with its message. Component lines
become a table: a function's parameters with `ret` as its returns row, a
type's fields or cases with their types. Constants are a table of name,
value and summary.

A `fwd` is an entry under the name it exports. Its alias and name are read
the same way, and the declaration it forwards is found by name in that
module's own file when the module is in the project; a forward of a forward
is followed link by link. When the alias is bound to a different module in
each arm, the entry lists every module that provides it, in order, each under
its own condition and linked, and shows the forwarded declaration once when every
provider spells it alike, or under each provider when they differ. Its own
docstring is shown, or else the forwarded declaration's. Every `pub`
declaration and every `fwd` is documented whether or not it has a docstring,
and a `#[testing]` declaration never is.

The pages are plain markdown: headings, paragraphs, emphasis, links, lists,
fenced code and pipe tables, and nothing else. The generator writes no HTML
tag, no byte outside ASCII and no syntax only one site renders, such as
GitHub's alerts. Names that share one table cell are separated by commas.
In prose, `<`, `&` and `\` are kept readable by markdown's backslash escape,
never by an HTML entity. The text of a docstring is its author's and is
carried as written, so a docstring that uses a character outside ASCII puts
it on the page.

Wherever a page lists a name with a few words beside it (the map, a table of
constants, a directory's modules), those words are the docstring's summary:
its first paragraph up to its first sentence end, a `.` followed by white
space or the end of the paragraph that is not inside a code span and does
not end `e.g.`, `i.e.`, `etc.` or `vs.`. The `.` itself is left out, and a
paragraph with no such `.` is its own summary. So the first sentence of a
docstring is what a reader scanning the reference sees: it should say what
the declaration is on its own, and the explanation belongs after it.

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
- `mach help doc` - the command that renders docstrings
