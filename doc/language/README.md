# Mach language reference

Per-element reference docs. Each file covers one language component;
read the index below or follow `see also` links to navigate.

This directory is the authoritative reference for the Mach 5 dialect. Each
file is a focused doc with grammar, examples, and neighboring links; start
from the index below. Examples whose fence carries an expectation (`accept`,
`reject`, `warn`, `run`) are compiled by `test/doc-examples.py` against the
compiler in the tree; see [Exercised examples](#exercised-examples). Users
moving from 4.x read [../migration-v5.md](../migration-v5.md) first.

## Files and structure

- [files.md](files.md) — extensions, `lib.mach` / `main.mach` conventions
- [modules.md](modules.md) — module tree, path separator, shadow-module pattern
- [use.md](use.md) — imports
- [fwd.md](fwd.md) — re-exports

## Declarations

- [visibility.md](visibility.md) — `pub` and `ext` modifiers
- [decorators.md](decorators.md) — codegen decorators, `#[name]` (`symbol`, `library`, `inline`, `align`, `section`, `embed`)
- [def.md](def.md) — type alias
- [rec.md](rec.md) - record
- [uni.md](uni.md) - raw union
- [tag.md](tag.md) - tagged value
- [fun.md](fun.md) - function
- [ext-fun.md](ext-fun.md) - external function
- [val-var.md](val-var.md) - immutable and mutable bindings
- [test.md](test.md) - test declaration and the mach test workflow
- [variadics.md](variadics.md) - variadic packs (va: ..., $each, va.len, va...)

## Values and types

- [literals.md](literals.md) - numeric, char, string
- [types.md](types.md) - primitive grammar, compound types, tag types and the std failure tags
- [secrecy.md](secrecy.md) - the ^ secret qualifier, flow typing, gates, :>T
- [operators.md](operators.md) - arithmetic, bitwise, comparison, logical, pointer, cast
- [expressions.md](expressions.md) - construction, access, calls, generic instantiation, `sel`

## Control flow

- [statements.md](statements.md) - if/or, for, ret, brk, cnt, fin, blocks, payload guards

## Comptime channel

- [comptime.md](comptime.md) - channel overview
- [comptime-mach.md](comptime-mach.md) - $mach.* compiler-owned namespace
- [decorators.md](decorators.md) - codegen decorators, #[name] (replaces the removed $sym.attr setters)
- [comptime-intrinsics.md](comptime-intrinsics.md) - $size_of, $length_of, $align_of, $offset_of, $type_of, $fields, $cases, $is_tag, $discriminant_of, $pointee_of, $is_record, $is_union, $is_pointer, $is_secret, $type_name, $each, $error
- [comptime-control.md](comptime-control.md) - $if / $or

## Low-level

- [asm.md](asm.md) — inline assembly
- [policy.md](policy.md) — compiler vs stdlib boundary

## Formal grammar

- [grammar.md](grammar.md) — full EBNF grammar of the implemented dialect,
  derived from the lexer and parser

## Conventions

- [documentation.md](documentation.md) — docstring style for functions,
  types, modules, and values

## Build system

- [../manifest.md](../manifest.md) — the `mach.toml` manifest reference
- [../cli.md](../cli.md) — the `mach` command-line reference
- [../distribution.md](../distribution.md) — shipping an application to users
- [../migration-v5.md](../migration-v5.md) — moving a 4.x project to 5.0

## Tooling

- [../tooling/editor-api.md](../tooling/editor-api.md) — the editor query
  surface (`mach.lang.editor`) a language server binds to
- [../tooling/test-json.md](../tooling/test-json.md) — the `mach test
  --format json` event schema

The supported, source-stable surface of the compiler is the editor API, the
command line, and the manifest schema. Everything else under `src/` is
internal. Source API authors can mark deprecated declarations with
[`#[deprecated]`](decorators.md#deprecated--deprecatedstr--source-use-notice), which warns on external use.

## Exercised examples

A fenced `mach` block in this directory is either a display fragment or an
exercised example. An exercised fence names its expectation after the
language: `mach accept` must compile clean, `mach reject "text"` must be
refused with a diagnostic containing `text`, `mach warn "text"` must compile
with a warning containing `text`, and `mach run "text"` must build, run and
print `text`. A block may hold several files, each introduced by a line
`# file: <path>` (the first file is `src/root.mach` when no line names it).
`python3 test/doc-examples.py` extracts every exercised fence, compiles it
with the compiler under test against the pinned `dep/std`, checks the
expectation, and reports how many fences exist and how many are exercised.
A fence with no expectation is a fragment and is counted, never compiled.

## Design

- [../design/](../design/) — why the compiler is shaped the way it is: the
  dependency model, the release shape, the IR operation descriptor,
  publication, and the closed failure kind
