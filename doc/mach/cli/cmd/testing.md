[mach](../../README.md) / [cli](../README.md) / [cmd](README.md) / **testing**

# mach.cli.cmd.testing

`mach test`, which builds the selection's test dispatchers and runs their tests

tests are run by target and profile through the test pool, launched through the runner
a target declares. a target and profile whose tests cannot run here are built and
reported as not run, and `--list` prints the collected tests and stops.

Source: [`src/cli/cmd/testing.mach`](../../../../src/cli/cmd/testing.mach)

Uses types from [`mach.base.fail`](../../base/fail.md), [`mach.cli.args`](../args.md).

| Functions | |
| :--- | :--- |
| [`run`](#run) | `mach test`: build the project's test dispatcher and run the collected tests |

| Constants | |
| :--- | :--- |
| [`COMMAND`](#constants) | `mach test` |
| [`POLICY`](#constants) | what mach holds `mach test` to beyond the parse |

---

## Functions

### run

```mach
pub fun run(cx: *mach.cli.args.Call) res[i64, mach.base.fail.Fail]
```

`mach test`: build the project's test dispatcher and run the collected tests. `--emit` is
refused; `--list` prints the collected tests and stops; `--diagnostics json` adds the run's
records to the report; `--filter`, `--runner`, `--timeout`, and `--threads` shape the run; `-o`
names the dispatcher binary the way `mach build -o` names a built one, inside the project
root, so runs in one tree with different `-o` do not collide

| Parameter | |
| :--- | :--- |
| `cx` | the call |
| **returns** | exit.OK when every test passed, exit.USER for at least one failed test, exit.INTERNAL for an infrastructure failure, otherwise the code `exit.of` maps the reported failure to |

## Constants

| Name | Type | Value | |
| :--- | :--- | :--- | :--- |
| `COMMAND` | `std.args.Command` | `std_args.Command{name: "test", aliases: args.NO_WORDS, help: "build and run project tests", usage: "<path>", flags: Slice[*std_args.Flag]{data: ?FLAGS[0], size: FLAGS_N}, commands: args.NO_COMMANDS, operand_min: 0, operand_max: std_args.OPERANDS_ANY, passthrough: false}` | `mach test` |
| `POLICY` | `mach.cli.args.Policy` | `args.Policy{refused: Slice[*std_args.Flag]{data: ?REFUSED[0], size: REFUSED_N}, refusal: "test always builds its internal test dispatcher", conflicts: args.NO_CONFLICTS}` | what mach holds `mach test` to beyond the parse |

