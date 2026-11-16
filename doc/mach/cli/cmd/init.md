[mach](../../README.md) / [cli](../README.md) / [cmd](README.md) / **init**

# mach.cli.cmd.init

`mach init`, which scaffolds a project: its manifest, entry file and std dependency

the entry is `src/bin/main.mach` with a bin artifact, or under `--lib`
`src/lib/<id>.mach` with a static artifact. std is added at the caret range of the
release resolution picks for the running compiler, and checked out unless `--no-deps`.

<sub>source: [`src/cli/cmd/init.mach`](../../../../src/cli/cmd/init.mach) · imported by 1 module</sub>

<sub>`args` is [`mach.cli.args`](../args.md) · `fail` is [`mach.base.fail`](../../base/fail.md) · `std_args` is `std.args`</sub>

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
pub fun run(cx: *args.Call) res[i64, fail.Fail]
```

`mach init`: scaffold a project. the positional names the directory (default the working
directory) and, unless `--name` is given, the project id, which must be a portable
identifier. an absent directory is created whole; an existing one is filled in, and
`--force` allows overwriting mach.toml and the entry file. the entry is src/bin/main.mach and
a bin artifact, or with `--lib` src/lib/&lt;id>.mach and a static artifact. the std dependency
is then added at the caret range of the release resolution picks and checked out, unless
`--no-deps`, which still resolves the range (so needs the network) but skips the checkout.
`--quiet` silences progress

| Parameter | |
| :--- | :--- |
| `cx` | the call |
| **returns** | exit.OK, or the failure: a user one for an invalid id or a refused overwrite, otherwise the std dependency's own class when it could not be added |

## Constants

| Name | Type | Value | |
| :--- | :--- | :--- | :--- |
| `COMMAND` | `std_args.Command` | `std_args.Command{name: "init", aliases: args.NO_WORDS, help: "create a project", usage: "[dir]", flags: Slice[*std_args.Flag]{data: ?FLAGS[0], size: FLAGS_N}, commands: args.NO_COMMANDS, operand_min: 0, operand_max: std_args.OPERANDS_ANY, passthrough: false}` | `mach init` |
| `POLICY` | `args.Policy` | `args.Policy{refused: args.NO_FLAGS, refusal: "", conflicts: args.NO_CONFLICTS}` | what mach holds `mach init` to beyond the parse |

