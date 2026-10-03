# Mach language reference

Per-element reference docs. Each file covers one language component;
read the index below or follow `see also` links to navigate.

This directory is the reference for the Mach language. Each file is a focused
doc with grammar, examples, and neighboring links. Start from the index below.

## Files and structure

- [files.md](files.md) — extensions, `lib.mach` / `main.mach` conventions
- [modules.md](modules.md) — module tree, path separator, shadow-module pattern
- [use.md](use.md) — imports
- [fwd.md](fwd.md) — re-exports

## Declarations

- [visibility.md](visibility.md) — `pub` and `ext` modifiers
- [decorators.md](decorators.md) — decorators, `#[<name>]`, and where each one applies
- [gpu.md](gpu.md) — the GPU decorators: pipeline stages, the shader interface, target handles and instructions
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
- [comptime-intrinsics.md](comptime-intrinsics.md) - the `$<name>(...)` intrinsics and `$each`
- [comptime-control.md](comptime-control.md) - $if / $or

## Low-level

- [asm.md](asm.md) — inline assembly
- [policy.md](policy.md) — compiler vs stdlib boundary

## Diagnostics

- [diagnostics.md](diagnostics.md) — diagnostic keys, the code registry and its never-reused rule
- [diagnostics-json.md](diagnostics-json.md) — `--diagnostics json`, the versioned NDJSON record schema
- [readout.md](readout.md) — what `-v` and `-vv` show for build, check and test, on which stream, and when

## Formal grammar

- [grammar.md](grammar.md) — full EBNF grammar of the implemented dialect,
  derived from the lexer and parser

## Conventions

- [documentation.md](documentation.md) — docstring style for functions,
  types, modules, and values
- The ```` ```mach ```` blocks on these pages are compiled by `test/run.sh --docs`.
  A plain block compiles, and runs when it declares a main. ```` ```mach fragment ````
  marks a block that is not a whole program, and ```` ```mach error <text> ```` a block
  whose compile fails with `<text>` in the output. A block showing several files starts
  each with `# file: src/<path>.mach`, in a project whose id is `example`. See
  [test/README.md](../../test/README.md#doc-blocks).

## Build system

- [manifest.md](manifest.md) — the `mach.toml` manifest reference
- [dependencies.md](dependencies.md) — how dependencies are resolved, pinned and verified
- [build.md](build.md) — which targets, profiles and artifacts a command builds
- [ir-output.md](ir-output.md) — `--emit-ir` and its two forms, and which one is stable for tooling
- `mach --help` and `mach help <command>` — the command-line reference

The supported, source-stable surface of the compiler is the editor API
(`mach.lang.editor`, documented by its docstrings and rendered by
`mach doc`), the command line, and the manifest schema. Everything else
under `src/` is internal. Source API authors can mark deprecated declarations with
[`#[deprecated]`](decorators.md#deprecated--deprecatedstr--source-use-notice), which warns on external use.
