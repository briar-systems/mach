---
name: "mach-project-overview"
description: "Overview of the `mach` compiler repository, build lifecycle, dependency rules, and workspace guidance for contributors."
---

## Purpose
This skill documents the repository layout, common build workflows, dependency rules, and workspace tips for contributors and agents acting on behalf of contributors.

---

## Quick facts
- Build targets:
  - `make cmach` — builds the bootstrap compiler into `out/bin/cmach`.
  - `make imach` — runs `$(CMACH) build . -o out/linux/bin/imach` (intermediate stage; incomplete).
  - `make mach` — pipeline continuation/scaffolding (incomplete).
- CLI: `cmach` supports subcommands `init`, `build`, `run`, `dep`, `help`.
- Docs: Mach language reference lives under `doc/*`.

---

## Dependency rules & workspace guidance
- **Do not** manually edit files under `dep/*`. Use `cmach dep` subcommands instead.
- Useful `cmach dep` commands:
  - `cmach dep list`
  - `cmach dep info <name>`
  - `cmach dep tidy`
  - `cmach dep add [--local] <path> <name> [--version <version>]`
  - `cmach dep del <name>`
  - `cmach dep pull [<name>]`
- Supported version formats: `branch/<name>`, `commit/<hash>`, `^1.2.3`, `~1.2.3`, `1.2.3` (see `cmach dep` help for details); note: `cmach dep add` vendors dependencies immediately.

**Workspace note (this repo):** Prefer editing the `mach-std/` repository directly rather than the vendored copy under `dep/mach-std/`.

If you need the compiler to use a local copy of the standard library for testing, temporarily switch the dependency to local from the consuming project (e.g., the top-level `mach` repo or a scratch project), not from inside the `mach-std` repository itself:

```sh
cmach dep add --local /ABS/PATH/TO/mach-std mach-std
```

Revert before committing:

```sh
cmach dep del mach-std
# or re-add the canonical remote/version
cmach dep add <remote-or-path> mach-std --version <version>
```

---

## Editing the boot compiler (C)
- Files under `boot/*` are the bootstrap compiler implementation in C. If you modify them, run `clang-format` where available to keep formatting consistent.

---

## When to pause / escalate
- If a change would affect Mach language **syntax**, **semantics**, or **public stdlib APIs**, pause and discuss with maintainers before proceeding.

---

## Example workflows (quick)
- Build bootstrap compiler and intermediate compiler:

```sh
make cmach
make imach
```

- Build full pipeline (note scaffolding):

```sh
make mach
```

- Build a specific target and run it:

```sh
out/bin/cmach build . -o out/bin/<file>
out/bin/cmach run . arg1 arg2
# to select a specific target defined in mach.toml:
out/bin/cmach run --target <name> . arg1 arg2
```

---

## Where to find more
- Language reference and deeper docs: `doc/*`
- Style and contribution notes: repository `README` and `doc` files.

