[mach](../../README.md) / [cli](../README.md) / [cmd](README.md) / **doc**

# mach.cli.cmd.doc

`mach doc`: the project's reference, every module some artifact reaches, written by
the documentation generator (`mach.tool.doc`) into `doc/<project id>/`

<sub>source: [`src/cli/cmd/doc.mach`](../../../../src/cli/cmd/doc.mach) · imported by 1 module</sub>

<sub>`args` is [`mach.cli.args`](../args.md) · `fail` is [`mach.base.fail`](../../base/fail.md) · `std_args` is `std.args`</sub>

| Functions | |
| :--- | :--- |
| [`run`](#run) | `mach doc`: load and resolve every module the project's artifacts reach, then write one page per module, one per directory and the project's page under `<root>/doc/<project id>/`, or under `-o <dir>` inside the project root, removing what that directory holds that the run did not write |

| Constants | |
| :--- | :--- |
| [`COMMAND`](#constants) | `mach doc` |
| [`POLICY`](#constants) | what mach holds `mach doc` to beyond the parse |

---

## Functions

### run

```mach
pub fun run(cx: *args.Call) res[i64, fail.Fail]
```

`mach doc`: load and resolve every module the project's artifacts reach, then write one
page per module, one per directory and the project's page under `<root>/doc/<project id>/`,
or under `-o <dir>` inside the project root, removing what that directory holds that the
run did not write. a summary line prints unless `--quiet`

| Parameter | |
| :--- | :--- |
| `cx` | the call |
| **returns** | exit.OK, or the failure: a user one for a project, manifest, selection or output path error, an internal one for an allocator or session failure, an environment one for a read, write or removal |

## Constants

| Name | Type | Value | |
| :--- | :--- | :--- | :--- |
| `COMMAND` | `std_args.Command` | `std_args.Command{name: "doc", aliases: args.NO_WORDS, help: "write the project's reference documentation", usage: "<path>", flags: Slice[*std_args.Flag]{data: ?FLAGS[0], size: FLAGS_N}, commands: args.NO_COMMANDS, operand_min: 0, operand_max: 1, passthrough: false}` | `mach doc` |
| `POLICY` | `args.Policy` | `args.Policy{refused: Slice[*std_args.Flag]{data: ?REFUSED[0], size: REFUSED_N}, refusal: "doc documents every artifact in one run", conflicts: args.NO_CONFLICTS}` | what mach holds `mach doc` to beyond the parse |

