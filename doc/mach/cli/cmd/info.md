[mach](../../README.md) / [cli](../README.md) / [cmd](README.md) / **info**

# mach.cli.cmd.info

`mach info`: the version and the host target, or with `targets` every target mink spells

<sub>source: [`src/cli/cmd/info.mach`](../../../../src/cli/cmd/info.mach) · imported by 1 module</sub>

<sub>`args` is [`mach.cli.args`](../args.md) · `fail` is [`mach.base.fail`](../../base/fail.md) · `std_args` is `std.args`</sub>

| Functions | |
| :--- | :--- |
| [`run`](#run) | `mach info`: print "mach &lt;version>" and the host target this binary resolves for the machine it runs on, or with the verb `targets` every target mink's catalog spells; any other verb is an error |

| Constants | |
| :--- | :--- |
| [`COMMAND`](#constants) | `mach info` |
| [`POLICY`](#constants) | what mach holds `mach info` to beyond the parse |

---

## Functions

### run

```mach
pub fun run(cx: *args.Call) res[i64, fail.Fail]
```

`mach info`: print "mach &lt;version>" and the host target this binary resolves for the
machine it runs on, or with the verb `targets` every target mink's catalog spells; any other
verb is an error

| Parameter | |
| :--- | :--- |
| `cx` | the call |
| **returns** | exit.OK, or a user failure for a usage error, or a host resolution failure |

## Constants

| Name | Type | Value | |
| :--- | :--- | :--- | :--- |
| `COMMAND` | `std_args.Command` | `std_args.Command{name: "info", aliases: args.NO_WORDS, help: "show the host target, or every target", usage: "[targets]", flags: Slice[*std_args.Flag]{data: ?FLAGS[0], size: FLAGS_N}, commands: args.NO_COMMANDS, operand_min: 0, operand_max: 1, passthrough: false}` | `mach info` |
| `POLICY` | `args.Policy` | `args.Policy{refused: args.NO_FLAGS, refusal: "", conflicts: args.NO_CONFLICTS}` | what mach holds `mach info` to beyond the parse |

