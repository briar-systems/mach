---
name: bootstrap-compiler-c
description: Works on the C bootstrap compiler (cmach) under boot/. Use when changing the C CLI, command handlers, config/TOML parsing, filesystem/git helpers, or fixing C build warnings/errors.
---

# bootstrap compiler (C) skill

## when to use

Use this skill when you are:

- changing `boot/` C code that builds into `out/bin/cmach`
- adding/modifying CLI commands (`mach build`, `mach dep`, `mach init`, `mach run`, `mach help`)
- adjusting config/TOML parsing for `mach.toml`
- debugging build failures in the C toolchain stage

## repo map

- command dispatch: `boot/src/main.c`
- commands: `boot/src/commands/` + `boot/include/commands/`
- config + toml: `boot/src/config.c`, `boot/src/toml.c`, `boot/include/config.h`, `boot/include/toml.h`
- filesystem helpers: `boot/src/filesystem.c`, `boot/include/filesystem.h`
- git/submodules helpers (deps): `boot/src/git.c`, `boot/include/git.h`
- compiler pipeline implementation: `boot/src/compiler/` + `boot/include/compiler/`

## build + verify workflow

1. keep changes small and rebuild often.
2. build the bootstrap compiler:
   - `make cmach`
3. run quick smoke checks:
   - `out/bin/cmach help`
   - `out/bin/cmach dep list` (in a project directory)
   - `out/bin/cmach build .` (in a project directory)

### formatting requirement

If you changed the bootstrap compiler C code, format it with `clang-format` (if available on the host). Keep diffs minimal and avoid unrelated reformatting.

## adding a new CLI command (pattern)

1. add a new header in `boot/include/commands/`:
   - `cmd_<name>.h` exporting:
     - `int cmd_<name>_handle(int argc, char **argv);`
     - `void cmd_<name>_help(FILE *stream);` (if applicable)
2. implement the command in `boot/src/commands/cmd_<name>.c`.
3. register it in `boot/src/main.c` (string compare + dispatch).
4. update `boot/src/commands/cmd_help.c` so it appears in help output.
5. rebuild (`make cmach`) and run `out/bin/cmach <name> ...`.

## common pitfalls

- the Makefile uses `-Werror`; warnings are failures. prefer explicit casts, check return values, and avoid unused variables.
- avoid editing vendored code under `dep/` directly. use `mach dep ...` to vendor/update dependencies.
- if a change impacts Mach language syntax/semantics, pause and confirm intent before proceeding (see `doc/`).
