[mach](../../README.md) / [cli](../README.md) / [cmd](README.md) / **help**

# mach.cli.cmd.help

`mach help`, and the page any command renders for `--help`

<sub>source: [`src/cli/cmd/help.mach`](../../../../src/cli/cmd/help.mach) · imported by 1 module</sub>

<sub>`A` is `std.allocator` · `std_args` is `std.args` · `fail` is [`mach.base.fail`](../../base/fail.md) · `args` is [`mach.cli.args`](../args.md)</sub>

| Functions | |
| :--- | :--- |
| [`page`](#page) | write the page of the command a parsed line matched to stdout: std.args' page of its usage, text, nested commands and flags |

| Constants | |
| :--- | :--- |
| [`COMMAND`](#constants) | `mach help`, and the page any command renders for `--help` |
| [`POLICY`](#constants) | what mach holds `mach help` to beyond the parse |

---

## Functions

### page

```mach
pub fun page(a: *A.Allocator, line: *std_args.Parsed) err[fail.Fail]
```

write the page of the command a parsed line matched to stdout: std.args' page of its usage,
text, nested commands and flags. the root's page is the overview

| Parameter | |
| :--- | :--- |
| `a` | holds the text of a refusal |
| `line` | a parsed line whose command is the page's |
| **returns** | ok, or the write that failed |

## Constants

| Name | Type | Value | |
| :--- | :--- | :--- | :--- |
| `COMMAND` | `std_args.Command` | `std_args.Command{name: "help", aliases: args.NO_WORDS, help: "show help for a command", usage: "[command [action]]", flags: Slice[*std_args.Flag]{data: ?FLAGS[0], size: FLAGS_N}, commands: args.NO_COMMANDS, operand_min: 0, operand_max: 2, passthrough: false}` | `mach help`, and the page any command renders for `--help` |
| `POLICY` | `args.Policy` | `args.Policy{refused: args.NO_FLAGS, refusal: "", conflicts: args.NO_CONFLICTS}` | what mach holds `mach help` to beyond the parse |

