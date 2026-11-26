[mach](../../README.md) / [cli](../README.md) / [cmd](README.md) / **doc**

# mach.cli.cmd.doc

`mach doc`: the project's reference, every module its source root holds, written by
the documentation generator (`mach.tool.doc`) into `doc/<project id>/`

Source: [`src/cli/cmd/doc.mach`](../../../../src/cli/cmd/doc.mach)

Uses types from [`mach.base.fail`](../../base/fail.md), [`mach.cli.args`](../args.md).

| Functions | |
| :--- | :--- |
| [`run`](#run) | `mach doc`: parse every source file under the project's source root and write one page per module that declares anything, one per directory and the project's page under `<root>/doc/<project id>/`, or under `-o <dir>` inside the project root, removing what that directory holds that the run did not write |

| Constants | |
| :--- | :--- |
| [`COMMAND`](#constants) | `mach doc` |
| [`POLICY`](#constants) | what mach holds `mach doc` to beyond the parse |

---

## Functions

### run

```mach
pub fun run(cx: *mach.cli.args.Call) res[i64, mach.base.fail.Fail]
```

`mach doc`: parse every source file under the project's source root and write one page
per module that declares anything, one per directory and the project's page under
`<root>/doc/<project id>/`, or under `-o <dir>` inside the project root, removing what
that directory holds that the run did not write. under -v each stage logs its line, and a
result line closes the run unless `--quiet`

| Parameter | |
| :--- | :--- |
| `cx` | the call |
| **returns** | exit.OK, or the failure: a user one for a project, manifest, output path or file that does not parse, an internal one for an allocator or session failure, an environment one for a read, write or removal |

## Constants

| Name | Type | Value | |
| :--- | :--- | :--- | :--- |
| `COMMAND` | `std.args.Command` | `std_args.Command{name: "doc", aliases: args.NO_WORDS, help: "write the project's reference documentation", usage: "<path>", flags: Slice[*std_args.Flag]{data: ?FLAGS[0], size: FLAGS_N}, commands: args.NO_COMMANDS, operand_min: 0, operand_max: 1, passthrough: false}` | `mach doc` |
| `POLICY` | `mach.cli.args.Policy` | `args.Policy{refused: Slice[*std_args.Flag]{data: ?REFUSED[0], size: REFUSED_N}, refusal: "doc documents every source file, for no artifact or target", conflicts: args.NO_CONFLICTS}` | what mach holds `mach doc` to beyond the parse |

