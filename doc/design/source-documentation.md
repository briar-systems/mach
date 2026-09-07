# Source documentation

Every fact about the compiler has exactly one home. The home is decided by
the question the fact answers, and a fact written in the wrong home is
either duplicated or lost the next time the code moves.

## Three destinations

| Question | Home | Form |
|---|---|---|
| what a declaration is, how it is used | the docstring on the declaration | `#` lines directly above it |
| why it is shaped that way | `doc/design/` | a page per decision |
| when it changed | `CHANGELOG.md` | a line per release |

A docstring states what the declaration is and how a caller uses it: the
parameters that matter, what the result means including its error, the
preconditions the code enforces, and the one thing a caller most often gets
wrong when the code shows it. It never says why. A reader who wants the
reason follows a `doc/design/` page, which records the decision, the
alternatives, and what the decision cost. A reader who wants to know which
release changed the behaviour reads the changelog. Nothing is written twice.

A docstring describes what the code does. If the code has a defect, the
docstring describes the defective behaviour and the defect is reported
separately. A docstring is never used to describe what the code should do.

## The docstring form

The lexer folds consecutive `#` lines into one comment run. The parser
attaches the run that ends on the line directly above a declaration, or
directly above the declaration's decorators, as that declaration's
docstring. One blank line between the run and the declaration breaks the
attachment. Decorator lines (`#[...]`) are not comments and do not join a
run.

```mach
# load an element through a pointer
# ---
# [T]: element type
# $order: memory ordering, one of the ORDER_* constants
# ptr: pointer to the element, never nil
# ret: the loaded element
#[inline]
pub fun load[T]($order: i64, ptr: *T) T { ret @ptr; }
```

The run has two parts:

- **Summary.** Every line before the separator. One sentence for what the
  declaration is, then any prose about how it is used. Rendered verbatim
  with the `#` and one following space stripped from each line.
- **Components.** A line that is exactly `# ---` opens the component block.
  Each component is `# name: description` with at most one space between
  `#` and the name. A line with two or more spaces after the `#` continues
  the previous description. Names are spelled as they appear in the
  declaration: a parameter or field by its name, a comptime parameter as
  `$name`, a generic as `[T]`, the return value as `ret`.

The separator and the component block are optional. A summary alone is a
complete docstring.

`doc/language/documentation.md` is the reference for this form; this page
states it only so far as the paradigm needs it.

`doclint` (`src/lang/fe/doclint.mach`) runs on every parse and checks the
component block of every `pub fun`, `pub rec`, and `pub uni` that has one.
It warns when a component names nothing in the declaration (including `ret`
on a function with no return type), when a component has an empty
description, and when components appear out of declaration order (generics,
then parameters or fields, then `ret`). It does not require every item to
be documented, and it checks nothing about the summary. It is silent on
declarations without a docstring, on non-`pub` declarations, and on
`val`, `def`, `use`, and `fwd` declarations, whose docstrings are summary
only. "`doclint` clean" means a build of the tree prints none of its three
warnings.

`mach doc` renders every `pub` declaration of every module to
`doc/api/<module path>.md`: a `##` heading with the declaration's name, the
signature up to its body in a code block, the summary, and the component
lines as `name: description`. The form above is the form that renderer
reads. Text a declaration needs beyond it, such as a usage example or a
table, belongs on a `doc/tooling/` or `doc/language/` page that the summary
can name.

## Comments

A comment that is not a docstring is brief, lowercase, single-line, and
present only where the code is not self-evident. There are no sectional or
separator comments. Prose that explains why belongs in `doc/design/`; a
comment may name the page. A comment that restates the line below it is
deleted.

## Routing documentation work to small models

A documentation task handed to a small model is an extractive work order,
never a verdict. The order names the declaration, the facts the docstring
must state (read off the code by the author of the order), the form above,
and the rule that the diff is comments and docstrings only. The model
transcribes; it does not decide what the code means, whether the code is
right, or what to leave out. Judgment tasks (is this declaration part of the
supported surface, is this behaviour a defect, does this page still hold)
stay with the author of the order, who verifies the result against the
tree: strip comments from the before and after and diff, build, run
`doclint`, and read every docstring against its declaration.
