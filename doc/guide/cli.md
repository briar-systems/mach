# CLI

`mach` is the compiler and the project tool. Every command names the project it works on, so the working directory never decides what is built.

## Command line

```text
mach <command> [options] <path>
```

```sh
mach build . -t linux-aarch64 -p release
```

- `<path>` is a project directory or its `mach.toml`, and it is required.
- A flag's value is the next argument or follows `=`, as `--report json` or `--report=json`.
- `mach <command> --help` lists every flag of a command.

## Commands

| Command | What it does |
|---|---|
| `build` | Compile a project, or one entry file |
| `check` | Type-check a project without building it |
| `run` | Run an executable a build already wrote |
| `test` | Build the project's tests and run them |
| `fmt` | Format source in the canonical layout |
| `migrate` | Rewrite a project from the previous major's spelling |
| `clean` | Remove the project's build output |
| `dep` | Manage the project's dependencies |
| `init` | Create a project |
| `doc` | Write reference documentation from the doc comments |
| `info` | Show the host target, or every target with `mach info targets` |
| `help` | Show a command's help |

## Selection

A build cell is one artifact for one target in one profile. `build`, `check`, `test` and `run` choose cells with the same three flags.

| Flag | Selects | With no flag |
|---|---|---|
| `-a, --artifact <pattern>` | `[artifact.<name>]` tables | Those marked `default = true`, or all |
| `-t, --target <pattern>` | `[target.<name>]` tables | The declared target matching the host |
| `-p, --profile <pattern>` | `[profile.<name>]` tables | The only profile, or the one marked `default = true` |

A pattern is a declared name or a glob, quoted so the shell leaves it alone: `-t 'linux-*'`. `--all` selects everything on each axis no flag names. `run` takes exactly one cell and has no `--all`.

`build`, `check`, `test` and `fmt` take `-v` to show each phase as it closes and `-q` to print errors only.

## `build`

```sh
mach build .
mach build . -t windows-x86_64 -p release
mach build hello.mach --source std=dep/std
```

A build compiles every module the artifact's entry reaches and links the result at the artifact's `out` path. Objects are cached, so the next build recompiles only what changed. The last line is an entry build: one file with no manifest, where `--source` binds a module prefix to a directory.

| Flag | Effect |
|---|---|
| `-o, --output <path>` | Write the linked artifact here. The selection must be one cell |
| `-O, --optimize`, `--no-optimize` | Override the profile's `optimize` |
| `--debug`, `--no-debug` | Override the profile's `debug` |
| `--emit <asm\|ir\|ir-opt\|obj\|exe>` | A product to write. The default is `exe` |
| `--no-cache` | Rebuild without reusing cached objects |
| `--plan` | Print the build plan without building |

In a project, `-O` and `--debug` change the selected profile, so the build must say where the different result goes: `mach build . -O -o out/fast`.

## `check`

```sh
mach check .
```

`check` type-checks the source the selected artifacts reach and writes nothing, so it is the fast loop while editing. `--declassified` lists every [declassification](secrecy.md) in the project.

## `run`

```sh
mach run . -- one two
```

`run` launches the program a build wrote for one cell. It never builds, so after an edit it runs the old program until the next `mach build`. Arguments after `--` go to the program, and the program's exit status is the command's.

| Flag | Effect |
|---|---|
| `--runner <cmd>` | Launch the program through a command, such as `wine` |
| `--timeout <duration>` | Stop the program after a duration such as `30s` |

## `test`

```sh
mach test . --filter is_leap
```

`test` builds the tests the selected artifact reaches and runs each in its own process. [Testing](testing.md) covers writing them.

| Flag | Effect |
|---|---|
| `--filter <substr>` | Run only the tests whose name contains the substring |
| `--list` | List the tests without running them |
| `--include-deps` | Also run the tests of dependencies |
| `--timeout <duration>` | Stop each test after a duration. The default is `60s` |

## `fmt`

```sh
mach fmt .
mach fmt src/bin/main.mach
mach fmt --check .
```

There is one layout and no configuration. A project path formats that project's own source in place, a file path formats one file, and `-` formats stdin to stdout. `--check` writes nothing and exits 1 when a file would change.

## `dep`

```sh
mach dep add . std --git https://github.com/briar-systems/mach-std
mach dep pull .
```

`mach dep <subcommand> <path>` manages what is checked out under `dep/`. The subcommands are `list`, `add`, `remove`, `update`, `pull`, `outdated` and `verify`, described in [Dependencies](dependencies.md). A build never fetches, so `mach dep pull .` comes first in a fresh clone.

## `init`

```sh
mach init demo
```

`init` creates the directory with a complete `mach.toml`, the entry `src/bin/main.mach`, a Git repository, and std checked out under `dep/std`. The result builds and runs with no edits.

| Flag | Effect |
|---|---|
| `--lib` | Make a library: a `static` artifact with the entry `src/lib/<id>.mach` |
| `--name <id>` | The project id. The default is the directory name |
| `--no-deps` | Declare no dependency, so nothing is read from the network |
| `--no-git` | Skip Git initialization |

## Reports

Every command takes `--report <human|json|lines>`. `json` turns stderr into records, one JSON object per line: each diagnostic, each test result, `mach info`'s rows and a closing summary. `lines` writes the same records as tab-separated `key=value` fields. The records are the stable interface for editors, CI and scripts.

```sh
mach check . --report json
```

```text
{"schema":3,"record":"summary","errors":0,"warnings":0,"notes":0,"outcome":"success","exit_code":0}
```

## Exit codes

| Code | Meaning |
|---|---|
| `0` | Success. For `test`, every test that ran passed or skipped |
| `1` | A refusal: a usage error, a bad manifest, a compile error, a failed test, a file that differs under `--check` |
| `2` | An internal failure of the compiler |
| `3` | A failure of the environment, such as a directory that cannot be written |

A build of several cells attempts each one and exits with the worst result. `mach run` exits with the program's own status.

The full rules are in the reference for [selection and builds](../language/build.md), [the readout](../language/readout.md) and [report records](../language/report-records.md).
