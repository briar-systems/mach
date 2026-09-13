# Mach language reference

Per-element reference docs. Each file covers one language component;
read the index below or follow `see also` links to navigate.

This directory is the authoritative reference for the Mach 5 dialect. Each
file is a focused doc with grammar, examples, and neighboring links; start
from the index below.

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

- [manifest.md](manifest.md) — the `mach.toml` manifest reference
- `mach --help` and `mach help <command>` — the command-line reference

The supported, source-stable surface of the compiler is the editor API
(`mach.lang.editor`, documented by its docstrings and rendered by
`mach doc`), the command line, and the manifest schema. Everything else
under `src/` is internal. Source API authors can mark deprecated declarations with
[`#[deprecated]`](decorators.md#deprecated--deprecatedstr--source-use-notice), which warns on external use.
