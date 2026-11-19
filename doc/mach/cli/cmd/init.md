[mach](../../README.md) / [cli](../README.md) / [cmd](README.md) / **init**

# mach.cli.cmd.init

`mach init`, which scaffolds a project: its manifest, entry file and std dependency

the entry is `src/bin/main.mach` with a bin artifact, or under `--lib`
`src/lib/<id>.mach` with a static artifact. std is declared at the caret range of the
release resolution picks for the running compiler, which is resolved before anything is
written, so a project is made whole or not at all, and is then checked out. `--no-deps`
declares no dependency and reads nothing from the network. a project made in a git
repository ignores its work directory.

Source: [`src/cli/cmd/init.mach`](../../../../src/cli/cmd/init.mach)

Uses types from [`mach.base.fail`](../../base/fail.md), [`mach.cli.args`](../args.md).

| Functions | |
| :--- | :--- |
| [`run`](#run) | `mach init`: scaffold a project |

| Constants | |
| :--- | :--- |
| [`COMMAND`](#constants) | `mach init` |
| [`POLICY`](#constants) | what mach holds `mach init` to beyond the parse |

---

## Functions

### run

```mach
pub fun run(cx: *mach.cli.args.Call) res[i64, mach.base.fail.Fail]
```

`mach init`: scaffold a project. the positional names the directory (default the working
directory) and, unless `--name` is given, the project id: the directory's name, with each '-'
written as '_' where the name is not a project id as written. an absent directory is created
whole; an existing one is filled in, and `--force` allows overwriting mach.toml and the entry
file. the entry is src/bin/main.mach and a bin artifact, or with `--lib` src/lib/\<id>.mach and
a static artifact. std is declared at the selection this compiler states for it, a range being
resolved before anything is written, and is checked out once the project is. `--no-deps` declares no dependency and reads nothing
from the network. in git mode the project is made a repository unless it stands in one, and a
`.gitignore` naming its work directory is written where none is. `--quiet` silences progress

| Parameter | |
| :--- | :--- |
| `cx` | the call |
| **returns** | exit.OK, or the failure: a user one for an invalid id or a refused overwrite, otherwise the class of the failure that resolving, initializing git or checking out met |

## Constants

| Name | Type | Value | |
| :--- | :--- | :--- | :--- |
| `COMMAND` | `std.args.Command` | `std_args.Command{name: "init", aliases: args.NO_WORDS, help: "create a project", usage: "[dir]", flags: Slice[*std_args.Flag]{data: ?FLAGS[0], size: FLAGS_N}, commands: args.NO_COMMANDS, operand_min: 0, operand_max: std_args.OPERANDS_ANY, passthrough: false}` | `mach init` |
| `POLICY` | `mach.cli.args.Policy` | `args.Policy{refused: args.NO_FLAGS, refusal: "", conflicts: args.NO_CONFLICTS}` | what mach holds `mach init` to beyond the parse |

