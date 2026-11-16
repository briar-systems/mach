[mach](../../README.md) / [cli](../README.md) / [cmd](README.md) / **migrate**

# mach.cli.cmd.migrate

`mach migrate`, which rewrites a project's manifest and source to this major's spelling

a script that may run mach beside the source, and a step whose `argv` runs mach, is read
for removed options, which are reported and never rewritten. under `--check` nothing is
written.

Source: [`src/cli/cmd/migrate.mach`](../../../../src/cli/cmd/migrate.mach)

Uses types from [`mach.base.fail`](../../base/fail.md), [`mach.cli.args`](../args.md).

| Functions | |
| :--- | :--- |
| [`run`](#run) | `mach migrate [<path>] [--check]` |

| Constants | |
| :--- | :--- |
| [`COMMAND`](#constants) | `mach migrate` |
| [`POLICY`](#constants) | what mach holds `mach migrate` to beyond the parse |

---

## Functions

### run

```mach
pub fun run(cx: *mach.cli.args.Call) res[i64, mach.base.fail.Fail]
```

`mach migrate [<path>] [--check]`

| Parameter | |
| :--- | :--- |
| `cx` | the call |
| **returns** | exit.OK when the project is in this major's spelling (or was made so), exit.USER when a file would change under --check, or in either mode when the manifest still writes a removed key no rewrite migrates or a source could not be read as one; a script's or a step's removed option is reported and fails nothing. otherwise the failure |

## Constants

| Name | Type | Value | |
| :--- | :--- | :--- | :--- |
| `COMMAND` | `std.args.Command` | `std_args.Command{name: "migrate", aliases: args.NO_WORDS, help: "rewrite a project into this major's spelling", usage: "[<path>]", flags: Slice[*std_args.Flag]{data: ?FLAGS[0], size: FLAGS_N}, commands: args.NO_COMMANDS, operand_min: 0, operand_max: 1, passthrough: false}` | `mach migrate` |
| `POLICY` | `mach.cli.args.Policy` | `args.Policy{refused: args.NO_FLAGS, refusal: "", conflicts: args.NO_CONFLICTS}` | what mach holds `mach migrate` to beyond the parse |

