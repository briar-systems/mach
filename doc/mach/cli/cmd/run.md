[mach](../../README.md) / [cli](../README.md) / [cmd](README.md) / **run**

# mach.cli.cmd.run

`mach run`, which launches an already built executable with the arguments after `--`

a runner the target declares, or `--runner`, launches it instead, and a target that is
not the host's and has neither is refused. under `--timeout` the program runs in its
own process group and is stopped when the time is up.

<sub>source: [`src/cli/cmd/run.mach`](../../../../src/cli/cmd/run.mach) · imported by 1 module</sub>

<sub>`args` is [`mach.cli.args`](../args.md) · `fail` is [`mach.base.fail`](../../base/fail.md) · `std_args` is `std.args`</sub>

| Functions | |
| :--- | :--- |
| [`run`](#run) | `mach run`: execute the project's already-built binary, forwarding the arguments after `--` to it |

| Constants | |
| :--- | :--- |
| [`COMMAND`](#constants) | `mach run` |
| [`POLICY`](#constants) | what mach holds `mach run` to beyond the parse |

---

## Functions

### run

```mach
pub fun run(cx: *args.Call) res[i64, fail.Fail]
```

`mach run`: execute the project's already-built binary, forwarding the arguments after
`--` to it. a target that declares a `runner` launches the expanded runner followed by
the arguments instead, and `--runner <cmd>` replaces it for this run with
`<cmd> <binary> <args>`; a target that is not the host's and has neither is refused.
`--timeout <duration>` runs the program in its own process group and terminates it
after the duration

| Parameter | |
| :--- | :--- |
| `cx` | the call |
| **returns** | the program's exit code; 128 plus the signal number when it was killed by a signal; exit.TIMEOUT when the timeout expired; otherwise mach's own failure: a user failure for a usage error or an artifact that is missing or that the system refuses to execute (a malformed image fails the spawn on windows), an environment failure for a wait, an internal one out of memory |

## Constants

| Name | Type | Value | |
| :--- | :--- | :--- | :--- |
| `COMMAND` | `std_args.Command` | `std_args.Command{name: "run", aliases: args.NO_WORDS, help: "run a built executable, passing arguments after --", usage: "<path> [-- args...]", flags: Slice[*std_args.Flag]{data: ?FLAGS[0], size: FLAGS_N}, commands: args.NO_COMMANDS, operand_min: 0, operand_max: std_args.OPERANDS_ANY, passthrough: true}` | `mach run` |
| `POLICY` | `args.Policy` | `args.Policy{refused: Slice[*std_args.Flag]{data: ?REFUSED[0], size: REFUSED_N}, refusal: "run executes an existing artifact", conflicts: args.NO_CONFLICTS}` | what mach holds `mach run` to beyond the parse |

