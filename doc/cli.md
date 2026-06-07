# `mach` — the command-line interface

```
mach <command> [options]
```

The compiler dispatches on `argv[1]`. With no command, or an unknown one, it
prints usage and exits non-zero. Every command except `init` runs from inside a
project tree: it walks up from the working directory until a `mach.toml` is
found.

This page documents the flags the current binary actually parses. Flags are
matched exactly: `--flag value` (a value follows in the next argument) or a bare
`--flag` toggle. The combined `--flag=value` form and bundled short flags are
**not** recognized.

## Commands

| Command | Summary |
|---------|---------|
| `build` | compile the project to objects and (in executable mode) a linked binary |
| `run`   | build, then execute the produced binary |
| `test`  | build the test binary and run the collected tests |
| `dep`   | inspect and (when supported) manage vendored dependencies |
| `init`  | scaffold a new project |
| `doc`   | generate Markdown reference docs from source doc-comments |
| `help`  | print usage; `mach help <command>` for detail |

## Global flags

Read by `build`, `run`, `test`, and `doc` (they share one config parser).
`--verbose` and `--quiet` together is a parse error.

| Flag             | Value            | Effect |
|------------------|------------------|--------|
| `--verbose`, `-v`| —                | write extra detail (per-stage compile progress) to stderr |
| `--quiet`, `-q`  | —                | suppress non-error output |
| `--color <mode>` | `auto`\|`always`\|`never` | color preference for terminal output (default `auto`); an unknown mode is a parse error |
| `--cwd <path>`   | path             | run as if started in `<path>` (project-root search begins there) |
| `--target <name>`| target name      | select a `[targets.<name>]` entry; absent, defers to `[project].target` |
| `-o <path>`      | path             | override the linked-binary path, rooted at the project root (build/run/test) |
| `--artifacts <dir>` | dir           | override the per-target object directory, rooted at `<dir_out>` (build/run/test) |
| `--emit-asm`     | —                | emit per-module assembly text (`.s`) beside each object |
| `--emit-ir`      | —                | emit per-module SSA IR text (`.ir`) beside each object |
| `--verify-ir`    | —                | run the IR verifier after each optimisation pass |

> `mach dep` and `mach init` do not use the shared config parser; they read only
> their own flags listed below.

## `mach build`

```
mach build [options]
```

Compiles the current project. Every reachable module is driven through
sema → lower → optimise → codegen to one relocatable object, written to
`<dir_out>/<artifacts>/obj/<fqn-as-path>.o`. In executable mode the objects are
linked into `<dir_out>/<binary>`; in library mode (or with `--emit obj`) the
objects are the deliverable and nothing is linked.

| Flag           | Value          | Effect |
|----------------|----------------|--------|
| `--release`    | —              | select the release optimisation pipeline |
| `-O0`          | —              | force the debug pipeline (overrides `--release`) |
| `-O1`          | —              | select the release pipeline |
| `-O2`          | —              | select the release pipeline |
| `--emit <kind>`| `obj`\|`exe`   | `obj` stops at the relocatable objects; `exe` (default) links a binary |
| `-L <dir>`     | dir            | add a search directory for `-l`-resolved objects; repeatable |
| `-l <name>`    | name           | link a named loose object, resolved through the `-L` dirs (see below); repeatable |
| *(positional)* | object path    | a bare argument that contains `/` or ends in `.o` is linked verbatim |

Plus the global flags above. The `-O<n>` flags override `--release` when both
are present; absent any optimisation flag the build uses the debug pipeline
(the bootstrap-stable default). `-O1` and `-O2` currently select the same
release pipeline.

### External link inputs

`ext fun` declarations are forward references whose definitions are supplied at
link time by external precompiled objects. Those objects come from the
command line and from the target's `[targets.*].libs` manifest field (see
[manifest.md](manifest.md)); both sets are linked. An input that resolves to no
existing file is a hard error, so a typo never silently drops a dependency.

- **Explicit object path** — a bare (non-flag) argument that contains a `/` or
  ends in `.o` is treated as an object path. The first non-flag positional after
  `build` is the project root and is skipped; remaining object-path positionals
  are link inputs. A relative path is tried verbatim against the working
  directory first, then rooted at the project root.
- **`-l <name>`** — resolves to a loose object. Each `-L <dir>` is searched for
  `<dir>/lib<name>.o`, then `<dir>/<name>.o`; if none hit, `lib<name>.o` then
  `<name>.o` relative to the working directory are tried.
- **`-L <dir>`** — adds a search directory for the `-l` resolution above. Both
  `-L` and `-l` may be repeated.

Only loose `.o` relocatable objects are supported — static archives (`.a`) and
shared libraries (`.so`) are not. Manifest `libs` are resolved before the CLI
inputs, giving a stable, deterministic link order.

Exit codes: `0` ok, `1` user error (no `mach.toml`, unknown target, compile
errors, an unresolvable link input), `2` internal error.

## `mach run`

```
mach run [options] [-- args...]
```

Builds the project exactly like `mach build`, then executes the produced binary.
Arguments after a `--` separator are forwarded to the child as its `argv`. The
child's exit code becomes this command's exit code.

`--emit` is rejected — running a relocatable object is not meaningful. All other
`build` and global flags apply.

Exit codes: the child's exit code, `1` on a build/user error, `2` on internal
error.

## `mach test`

```
mach test [options]
```

Builds a test binary — a synthesized runner over every `test` declaration the
resolver collected, replacing the project's own entry point — then runs it.
A test build always links an executable, even for a library target.

| Flag                | Value   | Effect |
|---------------------|---------|--------|
| `--filter <pattern>`| pattern | forwarded to the runner: run only tests whose name matches `<pattern>` |
| `--list`            | —       | forwarded to the runner: list the collected tests and exit |

Plus the `build` and global flags. `--filter`/`--list` are passed through to the
test runner as its argv; the runner itself selects or enumerates the tests.

Exit codes: `0` all passed, `1` any failed, `2` build/internal error.

## `mach dep`

```
mach dep <action> [args]
```

Manages vendored dependencies under `<dir_dep>`. Dispatches on `argv[2]`.

| Action   | Status | Effect |
|----------|--------|--------|
| `list`   | implemented | print each `[deps.<alias>]` entry from `mach.toml`, with its `path=` (or `source=`) field |
| `add`    | stub        | not supported in this build |
| `remove` | stub        | not supported in this build |
| `sync`   | stub        | not supported in this build |
| `vendor` | stub        | not supported in this build |

The mutating actions (`add`, `remove`, `sync`, `vendor`) require on-disk
fetch/copy and in-place manifest editing that the current runtime does not
expose. Each fails loudly with `'mach dep <action>' is not supported in this
build` rather than pretending to succeed.

| Flag        | Effect |
|-------------|--------|
| `--dry-run` | for a mutating action, describe the intended action and exit `0` instead of failing |

`mach dep list` reads `mach.toml` from the current directory directly (it does
not walk up to find a project root). Exit codes: `0` ok, `1` user error, `2`
internal error.

## `mach init`

```
mach init [dir] [options]
```

Scaffolds a new project in `[dir]` (default: the current directory). Writes a
minimal `mach.toml` with a `[project]` block and a `[targets.host]` entry
matching the running compiler, a starter source file, and an empty `dep/`
directory. Refuses to overwrite an existing `mach.toml` unless `--force`.

| Flag           | Value | Effect |
|----------------|-------|--------|
| `--name <name>`| name  | project id (default: the directory base name) |
| `--force`      | —     | scaffold even when a `mach.toml` already exists |
| `--lib`        | —     | library layout: write `src/lib.mach` instead of `src/main.mach`, and omit `entrypoint` from the manifest |

The first non-flag argument after `init` is the target directory.

> The scaffolded manifest omits the `binary` key (and, with `--lib`, the
> `entrypoint` key too) — both of which a build requires. A scaffolded project
> therefore needs its `[targets.host]` entry completed before `mach build`
> succeeds. See [manifest.md](manifest.md) for the required keys.

Exit codes: `0` ok, `1` user error, `2` internal error.

## `mach doc`

```
mach doc [options]
```

Loads the project's module graph and generates Markdown reference docs from
source doc-comments — one page per module plus an index. Each `pub` declaration
is paired with the run of `#` comment lines immediately preceding it. The
hand-written `doc/language/` material is never touched.

| Flag             | Value | Effect |
|------------------|-------|--------|
| `--out <dir>`    | dir   | output directory, rooted at the project (default `doc/api`) |
| `--target <name>`| name  | select a `[targets.<name>]` entry for module discovery |

Plus the global flags. Exit codes: `0` ok, `1` user error, `2` internal error.

## `mach help`

```
mach help [command]
```

Prints the top-level usage summary, or — with a known `[command]` — that
command's detail page. An unknown command prints the top-level usage and exits
non-zero.

## See also

- [manifest.md](manifest.md) — the `mach.toml` reference
- [language/test.md](language/test.md) — the `test` declaration and `mach test`
- [language/files.md](language/files.md) — project file layout
