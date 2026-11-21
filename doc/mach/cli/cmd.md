[mach](../README.md) / [cli](README.md) / **cmd**

# mach.cli.cmd

the table of commands and the dispatch that routes a `mach` invocation to one of them

a command is its own file's `std_args.Command` and policy and its row here, and nothing
else registers it.

Source: [`src/cli/cmd.mach`](../../../src/cli/cmd.mach)

| Functions | |
| :--- | :--- |
| [`dispatch`](#dispatch) | the `mach` entry point: route the arguments to one command and return its exit code |

---

## Functions

### dispatch

```mach
pub fun dispatch(argc: usize, argv: **u8) i64
```

the `mach` entry point: route the arguments to one command and return its exit code. `--help`
renders the page of the command it follows in place of running it, whatever else the line is
refused for but an unknown command, and `--version` before a command prints the version alone.
with no command the overview prints and the code is 1, and an unknown command is refused with
the overview after it. a refusal is reported in the format `--report` selects, and
otherwise args.run reports a refused combination or runs the command

| Parameter | |
| :--- | :--- |
| `argc` | process argument count |
| `argv` | process arguments, argv[0] the program |
| **returns** | the exit code: exit.USER for a missing or unknown command, the code `exit.of` maps a failure to set up or read the line to, otherwise the command's own code |

