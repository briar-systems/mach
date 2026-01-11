---
name: cli-and-project-config
description: Works on cmach CLI behavior and mach.toml project configuration. Use when editing build/run/init/dep commands, config parsing/validation, target selection, module roots, or output path rules.
---

# cli + project config skill

## when to use

Use this skill for tasks involving:

- `mach.toml` schema and parsing
- build/run/init/dep CLI behavior
- target selection and output file layout
- module resolution roots (`project id`, `src`, `dep`) and dependency config loading

## key paths

- CLI dispatch: `boot/src/main.c`
- command implementations: `boot/src/commands/`
  - build: `boot/src/commands/cmd_build.c`
  - dep: `boot/src/commands/cmd_dep.c`
  - run: `boot/src/commands/cmd_run.c`
  - init: `boot/src/commands/cmd_init.c`
  - help: `boot/src/commands/cmd_help.c`
- config schema + parsing: `boot/include/config.h`, `boot/src/config.c`
- toml parser wrapper: `boot/include/toml.h`, `boot/src/toml.c`

Project docs:
- `doc/config.md`
- `doc/dependencies.md`
- `doc/modules.md`

## mach.toml mental model

`mach.toml` has three main parts (see `mach.toml` at repo root for an example):

- `[project]`: metadata and directory layout (`dir_src`, `dir_out`, `dir_dep`) and default `target`
- `[targets.<name>]`: OS/ISA/ABI + `mode`, plus `entrypoint` and `binary` output path
- `[deps.<name>]`: dependency registry for module resolution and vendoring (`type`, `path`, `version`)

The bootstrap compiler parses this via `config_load()` in `boot/src/config.c`.

## build command behavior (bootstrap)

`cmach build <project|file>` supports two modes (see `boot/src/commands/cmd_build.c`):

- project mode: argument is a directory containing `mach.toml`
  - picks target from `project.target` (fallback: first target)
  - computes entrypoint path: `<project>/<dir_src>/<entrypoint>`
  - computes output: `<project>/out/<target.binary>`
  - sets module roots for sema: project id + src_root (+ dep_root)

- single-file mode: argument is a `.mach` file
  - output defaults to `output` unless `-o` is passed
  - module path defaults to `main` unless `-m` is passed
  - additional module roots can be injected via `-I prefix=/abs/path`

### target selection gotchas

- `project.target = "native"` triggers matching against `masm_target_native()`.
- ensure `masm_target_native()` and the `[targets.*]` table stay consistent, or config lookup will return NULL.

## changing config fields safely

1. decide whether this is user-visible behavior. if yes, update docs under `doc/`.
2. update parsing and validation in `boot/src/config.c`.
3. update serialization in `config_save()` if the field should round-trip.
4. update the `mach.toml` example(s) in the repo if appropriate.
5. rebuild `cmach` and smoke-test `cmach build .`, `cmach dep list`, `cmach dep pull`.
