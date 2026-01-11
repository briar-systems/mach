---
name: docs-maintenance
description: Maintains Mach documentation under doc/ with consistent semantics and style. Use when changing compiler behavior, adding language features, or updating build/deps guidance.
---

# docs maintenance skill

## when to use

Use this skill when:

- you changed anything user-visible (syntax, semantics, CLI behavior, project config)
- you added/removed a feature or a keyword
- build/dependency instructions changed
- docs drifted from implementation

## doc structure

- entry index: `doc/README.md`
- getting started / build: `doc/getting-started.md`
- config/deps/modules: `doc/config.md`, `doc/dependencies.md`, `doc/modules.md`
- core semantics: `doc/keywords.md`, `doc/expressions.md`, `doc/types.md`, `doc/literals.md`
- advanced: `doc/memory.md`, `doc/compiletime.md`, `doc/quirks.md`
- compiler internals: `doc/mir/` (keep accurate to the implementation)

## workflow

1. identify the single best doc page for the change.
2. update that page first; add cross-links rather than duplicating content.
3. update `doc/README.md` if you added/renamed a page.
4. keep examples compiling (prefer minimal examples; avoid pseudo-syntax).

## consistency checks

- terminology matches code: module naming, keywords, pointer operators, cast rules.
- if backend pipeline changes (MASM vs MIR), update `doc/mir/` to match reality.
