[mach](../../README.md) / [cli](../README.md) / [cmd](README.md) / **fmt**

# mach.cli.cmd.fmt

`mach fmt`, which rewrites source into the canonical layout

the operand is a project, one source file formatted with no manifest read, or `-` for
standard input. under `--check` nothing is written and a source that is not canonical
fails the command.

Source: [`src/cli/cmd/fmt.mach`](../../../../src/cli/cmd/fmt.mach)

Uses types from [`mach.base.fail`](../../base/fail.md), [`mach.cli.args`](../args.md).

| Type | | |
| :--- | :--- | :--- |
| [`Operand`](#operand) | `def` | how the operand names its input |

| On `Operand` | |
| :--- | :--- |
| [`classify`](#classify) | which form the operand takes |

| Functions | |
| :--- | :--- |
| [`run`](#run) | `mach fmt <path>\|<file>\|- [--check]` |

| Constants | |
| :--- | :--- |
| [`COMMAND`](#constants) | `mach fmt` |
| [`POLICY`](#constants) | what mach holds `mach fmt` to beyond the parse |

---

## Operand

```mach
pub def Operand: u8
```

how the operand names its input

| Field | Type | |
| :--- | :--- | :--- |
| `stream` |  | `-`, the source arrives on stdin |
| `source` |  | one source file, formatted with no manifest read |
| `project` |  | a directory holding a mach.toml, or the mach.toml itself |

### classify

```mach
pub fun classify(a: *std.allocator.Allocator, operand: str) res[Operand, mach.base.fail.Fail]
```

which form the operand takes. `-` is the stream; a directory, or a file named
mach.toml, is a project; any other existing file is a single source

## Functions

### run

```mach
pub fun run(cx: *mach.cli.args.Call) res[i64, mach.base.fail.Fail]
```

`mach fmt <path>|<file>|- [--check]`

| Parameter | |
| :--- | :--- |
| `cx` | the call |
| **returns** | exit.OK when every input is canonical (or was made so), exit.USER when an input differs under --check or is malformed, otherwise the failure |

## Constants

| Name | Type | Value | |
| :--- | :--- | :--- | :--- |
| `COMMAND` | `std.args.Command` | `std_args.Command{name: "fmt", aliases: args.NO_WORDS, help: "format source in the canonical layout", usage: "<path>\|<file>\|-", flags: Slice[*std_args.Flag]{data: ?FLAGS[0], size: FLAGS_N}, commands: args.NO_COMMANDS, operand_min: 1, operand_max: 1, passthrough: false}` | `mach fmt` |
| `POLICY` | `mach.cli.args.Policy` | `args.Policy{refused: args.NO_FLAGS, refusal: "", conflicts: args.NO_CONFLICTS}` | what mach holds `mach fmt` to beyond the parse |

