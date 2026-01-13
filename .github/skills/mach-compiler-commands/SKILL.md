---
name: "mach-compiler-commands"
description: "How to use the `cmach` toolchain: build/run/dep commands, options, and practical examples."
---

## Purpose
Actionable reference for common `cmach` commands, options, and dependency workflows used in this repository.

---

## `cmach` overview
- Common subcommands: `init`, `build`, `run`, `dep`, `help`.
- Build options of note:
  - `--target <name>` — select named target; note: for `build` this flag appears in help but the implementation currently does not parse it — build selects the project default target from `mach.toml` (or the first target).
  - `-o <file>` — set output file path
  - `-m <path>` — set module path (e.g. `std.print`) for single-file builds
  - `-I prefix=dir` — add include/search prefix

---

## Build/run examples
- Build a project (example):

```sh
out/bin/cmach build . -o out/bin/myapp
```

- Run the built target (arguments are passed to the executable; there is no special `--` separator):

```sh
out/bin/cmach run . arg1 arg2
# or to select a specific target defined in mach.toml:
out/bin/cmach run --target <target> . arg1 arg2
```

- Typical repo make targets:

```sh
make cmach   # build bootstrap compiler -> out/bin/cmach
make imach   # runs "$(CMACH) build . -o out/linux/bin/imach" (intermediate stage; incomplete)
make mach    # pipeline continuation/scaffolding (incomplete)
```

---

## Dependency management (`cmach dep`)
- `cmach dep list` — show current deps
- `cmach dep info <name>` — show metadata
- `cmach dep tidy` — clean unused deps (submodule maintenance)
- `cmach dep add [--local] <path> <name> [--version <version>]` — add dependency (local or remote); note: `dep add` vendors the dependency immediately
- `cmach dep del <name>` — remove dependency
- `cmach dep pull [name]` — fetch/update

**Version formats:** examples recognized by `cmach dep` include:
  - `branch/main` — track a specific branch
  - `commit/<hash>` — specific commit
  - `^1.2.3` — semver caret (>=1.2.3 <2.0.0)
  - `~1.2.3` — semver tilde (>=1.2.3 <1.3.0)
  - `1.2.3` — exact semver tag

Example: temporarily use a local stdlib for testing (run from the consuming project):

```sh
cmach dep add --local /ABS/PATH/TO/mach-std mach-std
# test changes, then revert before commit
cmach dep del mach-std
```

---

## Troubleshooting & tips
- Use `cmach help` to list full subcommand options.
- Inspect `out/` artifacts for build results and logs.
- If build failures appear after changing compiler or stdlib, rebuild the bootstrap compiler and intermediate stages in sequence.

