---
name: repo-navigation
description: Navigates the Mach compiler repository quickly and safely. Use to locate code by subsystem, understand boundaries (boot vs src vs doc), and avoid off-limits vendored dependencies.
---

# repo navigation skill

## when to use

Use this skill when you need to:

- find where a subsystem lives (compiler, CLI, docs, std)
- decide which repo/folder to edit
- understand staging (bootstrap vs self-hosted)

## top-level layout

- `boot/` — C bootstrap compiler (“cmach”), including CLI commands and the current working compiler pipeline.
  - `boot/src/` implementation
  - `boot/include/` headers
- `src/` — Mach sources for the self-hosted compiler (under active development).
- `doc/` — language and tooling documentation (source of truth for syntax/semantics).
- `dep/` — vendored dependencies managed by `cmach dep ...`.
- `.github/` — repo automation and Copilot customization (including these skills).

## boundaries and safety rules

- avoid editing anything under `dep/` manually.
- changes affecting Mach language syntax/semantics must be discussed/confirmed before implementation.
- if you change bootstrap compiler C code, run `clang-format` when available.

## where to look for common tasks

- “why does build do X?”: `boot/src/commands/cmd_build.c`
- “how are deps vendored?”: `boot/src/commands/cmd_dep.c`, `boot/src/git.c`
- “module resolution / imports”: `boot/src/compiler/sema.c` and docs `doc/modules.md`
- “Mach syntax and keywords”: `doc/keywords.md`, `doc/expressions.md`, `doc/types.md`
- “MASM backend / object emission”: `boot/src/compiler/masm/`
