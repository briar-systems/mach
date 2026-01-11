---
name: dependency-management
description: Manages Mach project dependencies via cmach dep (remote submodules and local copies). Use when touching dep/ layout, mach.toml [deps] entries, or fixing vendoring/submodule issues.
---

# dependency management skill

## when to use

Use this skill when the user asks about:

- adding/updating/removing dependencies
- `dep/` directory contents
- git submodule problems (missing, detached, wrong version)
- local dependency vendoring
- `mach.toml` `[deps.*]` entries and version formats

## hard rule: do not edit vendored dep/ directly

Directories under `dep/` are managed by the Mach dependency tooling.

- do not hand-edit files inside `dep/<name>/...`
- instead, adjust `mach.toml` and use `cmach dep ...` to vendor/refresh

If you need to change the standard library, use the separate `mach-std` repository in this workspace (see the `standard-library-mach-std` skill).

## commands (bootstrap)

See `boot/src/commands/cmd_dep.c` for exact behavior.

- `cmach dep list`
- `cmach dep info <name>`
- `cmach dep tidy` (submodule maintenance pass)
- `cmach dep add [--local] <path-or-url> <name> [--version <spec>]`
- `cmach dep del <name>`
- `cmach dep pull [name]`

## version formats

Supported version spec patterns (from `cmd_dep_help`):

- `branch/<name>` (track a branch)
- `commit/<hash>` (pin a commit)
- semver-like strings: `1.2.3`, `^1.2.3`, `~1.2.3`

Notes:
- remote deps require `version` in `mach.toml`.
- local deps are copied into `dep/<name>`.

## troubleshooting checklist

- if `dep/<name>` exists but is not initialized as a git repo:
  - run `cmach dep pull <name>`
- if `.gitmodules` is out of sync:
  - run `cmach dep tidy`
- if a dependency changed branches/versions:
  - update `[deps.<name>].version` then run `cmach dep pull <name>`
