# Mach language reference

Per-element reference docs. Each file covers one language component;
read the index below or follow `see also` links to navigate.

This directory is the authoritative reference for the locked Mach
dialect. Each file is a focused doc with grammar, examples, and
neighboring links; start from the index below.

## Files and structure

- [files.md](files.md) — extensions, `lib.mach` / `main.mach` conventions
- [modules.md](modules.md) — module tree, path separator, shadow-module pattern
- [use.md](use.md) — imports
- [fwd.md](fwd.md) — re-exports

## Declarations

- [visibility.md](visibility.md) — `pub` and `ext` modifiers
- [def.md](def.md) — type alias
- [rec.md](rec.md) — record
- [uni.md](uni.md) — raw union and the discriminated-value convention
- [fun.md](fun.md) — function
- [ext-fun.md](ext-fun.md) — external function
- [val-var.md](val-var.md) — immutable and mutable bindings

## Values and types

- [literals.md](literals.md) — numeric, char, string
- [types.md](types.md) — primitive grammar, compound types
- [operators.md](operators.md) — arithmetic, bitwise, comparison, logical, pointer, cast
- [expressions.md](expressions.md) — construction, access, calls, generic instantiation

## Control flow

- [statements.md](statements.md) — `if`/`or`, `for`, `ret`, `brk`, `cnt`, `fin`, blocks

## Comptime channel

- [comptime.md](comptime.md) — channel overview
- [comptime-mach.md](comptime-mach.md) — `$mach.*` compiler-owned namespace
- [comptime-attrs.md](comptime-attrs.md) — symbol attributes
- [comptime-intrinsics.md](comptime-intrinsics.md) — `$size_of`, `$align_of`, `$offset_of`, `$error`, `$assert`
- [comptime-control.md](comptime-control.md) — `$if` / `$or`

## Low-level

- [asm.md](asm.md) — inline assembly
- [policy.md](policy.md) — compiler vs stdlib boundary

## Conventions

- [documentation.md](documentation.md) — docstring style for functions,
  types, modules, and values
