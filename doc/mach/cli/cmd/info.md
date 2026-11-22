[mach](../../README.md) / [cli](../README.md) / [cmd](README.md) / **info**

# mach.cli.cmd.info

`mach info`: the version and the host target, or with `targets` every target a build can be made for

Source: [`src/cli/cmd/info.mach`](../../../../src/cli/cmd/info.mach)

Uses types from [`mach.base.fail`](../../base/fail.md), [`mach.cli.args`](../args.md).

| Functions | |
| :--- | :--- |
| [`run`](#run) | `mach info`: print "mach \<version>" and the host target this binary resolves for the machine it runs on, or with the verb `targets` every target a build can be made for; any other verb is an error |

| Constants | |
| :--- | :--- |
| [`COMMAND`](#constants) | `mach info` |
| [`POLICY`](#constants) | what mach holds `mach info` to beyond the parse |

---

## Functions

### run

```mach
pub fun run(cx: *mach.cli.args.Call) res[i64, mach.base.fail.Fail]
```

`mach info`: print "mach \<version>" and the host target this binary resolves for the
machine it runs on, or with the verb `targets` every target a build can be made for; any other
verb is an error

| Parameter | |
| :--- | :--- |
| `cx` | the call |
| **returns** | exit.OK, or a user failure for a usage error, or a host resolution failure |

## Constants

| Name | Type | Value | |
| :--- | :--- | :--- | :--- |
| `COMMAND` | `std.args.Command` | `std_args.Command{name: "info", aliases: args.NO_WORDS, help: "show the host target, or every target", usage: "[targets]", flags: Slice[*std_args.Flag]{data: ?FLAGS[0], size: FLAGS_N}, commands: args.NO_COMMANDS, operand_min: 0, operand_max: 1, passthrough: false}` | `mach info` |
| `POLICY` | `mach.cli.args.Policy` | `args.Policy{refused: args.NO_FLAGS, refusal: "", conflicts: args.NO_CONFLICTS}` | what mach holds `mach info` to beyond the parse |

