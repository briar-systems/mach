---
name: standard-library-mach-std
description: Works on the Mach standard library (mach-std) and runtime integration. Use when editing std modules, Linux runtime/syscalls, or keeping mach and mach-std in sync without editing vendored dep/ directly.
---

# mach-std (standard library) skill

## when to use

Use this skill when you need to:

- change standard library APIs or modules
- adjust runtime startup, platform support, syscalls, or constants
- fix issues that involve `use std.system.runtime;` and friends
- sync dependency versions between `mach` and `mach-std`

## where to edit

In this workspace there are two copies:

- preferred: the standalone repo at `../mach-std/` (edit here)
- vendored: `mach/dep/mach-std/` (do not edit; managed by `cmach dep ...`)

## integration points

- `mach` depends on `mach-std` via `mach.toml` `[deps.mach-std]`.
- the bootstrap compiler loads dependency configs from `dep/<name>/mach.toml` when present (see `boot/src/config.c`).

## keeping mach and mach-std in sync

1. make changes in the `mach-std/` repo.
2. commit them there.
3. update `mach/mach.toml` dependency version if needed.
4. vendor/update in `mach` using `cmach dep pull mach-std` (or `cmach dep pull`).

## mach-std map (high level)

- top-level modules: `mach-std/src/*.mach` (e.g. `lib.mach`, `memory.mach`, `filesystem.mach`, `runtime.mach`)
- platform-specific runtime/system: `mach-std/src/system/platform/linux/...` and `mach-std/src/system/runtime/linux...`
- core types: `mach-std/src/types/` (string, slice, option/result, etc.)

When changing runtime conventions, also update `mach` docs:
- `mach/doc/quirks.md`
- `mach/doc/getting-started.md`
