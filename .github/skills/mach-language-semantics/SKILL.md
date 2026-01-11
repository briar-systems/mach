---
name: mach-language-semantics
description: Uses doc/ as the source of truth for Mach syntax and semantic rules. Use when answering language questions or implementing/validating language behavior in the compiler.
---

# mach language syntax + semantics skill

## when to use

Use this skill when:

- the user asks “how does Mach do X?” (syntax, types, modules, control flow)
- you are implementing or fixing a language rule in the compiler
- you need to verify whether behavior is intended vs a bug

## source of truth

Treat `doc/` as the canonical reference. Start here:

- `doc/README.md` (index)
- semantics: `doc/keywords.md`, `doc/expressions.md`, `doc/types.md`, `doc/literals.md`
- modules/deps/config: `doc/modules.md`, `doc/dependencies.md`, `doc/config.md`
- advanced: `doc/memory.md`, `doc/compiletime.md`, `doc/quirks.md`

## guardrail: confirm before changing the language

If a requested change affects syntax, semantics, or user-visible behavior:

1. explicitly call it out as a language change.
2. confirm the intent.
3. update docs in the same change set.

## high-signal rules to keep consistent

- comments: `#` single-line.
- imports: `use [alias:] project.module;` (modules map 1:1 to `.mach` files under `dir_src`).
- declarations: `val` immutable, `var` mutable, `pub` exports.
- casts: `value::TargetType` (sizes must match exactly).
- pointers:
  - `?expr` address-of lvalue
  - `@ptr` dereference
  - `*T` mutable pointer, `&T` read-only pointer
  - `*T` can cast to `&T` implicitly, not vice-versa
- control flow:
  - `if (...) {}` with `or (...) {}` / `or {}`
  - loops: `for (...) {}` / `for {}`
  - `brk`, `cnt`, `ret`, `fin` (defer)

## entrypoint convention

The runtime convention is documented in `doc/quirks.md` (runtime entry). A typical program imports runtime and sets `$main.symbol = "main"`.

If you change entrypoint naming or startup behavior, align:
- compiler (codegen/linking)
- standard library runtime (`mach-std` repo)
- docs (`doc/quirks.md`, `doc/getting-started.md`)
