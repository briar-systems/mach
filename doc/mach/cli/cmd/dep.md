[mach](../../README.md) / [cli](../README.md) / [cmd](README.md) / **dep**

# mach.cli.cmd.dep

`mach dep` and its actions: list, add, remove, update, pull, outdated and verify

each action's flags are read here and its work is the package manager's (mach.package).

Source: [`src/cli/cmd/dep.mach`](../../../../src/cli/cmd/dep.mach)

Uses types from [`mach.base.fail`](../../base/fail.md), [`mach.cli.args`](../args.md).

| Functions | |
| :--- | :--- |
| [`run`](#run) | `mach dep`: route to one action: list, add, remove, update, pull, verify or outdated, each a command over the dependency manager (mach.package) that renders what it reports |

| Constants | |
| :--- | :--- |
| [`LIST`](#constants) | `mach dep list` |
| [`ADD`](#constants) | `mach dep add` |
| [`REMOVE`](#constants) | `mach dep remove` |
| [`UPDATE`](#constants) | `mach dep update` |
| [`PULL`](#constants) | `mach dep pull` |
| [`OUTDATED`](#constants) | `mach dep outdated` |
| [`VERIFY`](#constants) | `mach dep verify` |
| [`ADD_POLICY`](#constants) | what mach holds `mach dep add` to beyond the parse |
| [`UPDATE_POLICY`](#constants) | what mach holds `mach dep update` to beyond the parse |
| [`VERIFY_POLICY`](#constants) | what mach holds `mach dep verify` to beyond the parse |
| [`POLICY`](#constants) | what mach holds `mach dep` and its other actions to beyond the parse |
| [`COMMAND`](#constants) | `mach dep` |

---

## Functions

### run

```mach
pub fun run(cx: *mach.cli.args.Call) res[i64, mach.base.fail.Fail]
```

`mach dep`: route to one action: list, add, remove, update, pull, verify or outdated, each a
command over the dependency manager (mach.package) that renders what it reports

| Parameter | |
| :--- | :--- |
| `cx` | the call, its action read and its operands counted |
| **returns** | exit.OK, the code `exit.of` maps the action's rendered failure to, or the failure that kept the action from starting |

## Constants

| Name | Type | Value | |
| :--- | :--- | :--- | :--- |
| `LIST` | `std.args.Command` | `std_args.Command{name: "list", aliases: args.NO_WORDS, help: "list declared dependencies and their state", usage: "<path>", flags: Slice[*std_args.Flag]{data: ?PLAIN_FLAGS[0], size: 3}, commands: args.NO_COMMANDS, operand_min: 1, operand_max: 1, passthrough: false}` | `mach dep list` |
| `ADD` | `std.args.Command` | `std_args.Command{name: "add", aliases: args.NO_WORDS, help: "declare a dependency", usage: "<path> <name> (--git <url> [--ref <ref> \| --range <range>] \| --path <dir>)", flags: Slice[*std_args.Flag]{data: ?ADD_FLAGS[0], size: 9}, commands: args.NO_COMMANDS, operand_min: 2, operand_max: 2, passthrough: false}` | `mach dep add` |
| `REMOVE` | `std.args.Command` | `std_args.Command{name: "remove", aliases: args.NO_WORDS, help: "remove a dependency", usage: "<path> <name>", flags: Slice[*std_args.Flag]{data: ?REMOVE_FLAGS[0], size: 4}, commands: args.NO_COMMANDS, operand_min: 2, operand_max: 2, passthrough: false}` | `mach dep remove` |
| `UPDATE` | `std.args.Command` | `std_args.Command{name: "update", aliases: args.NO_WORDS, help: "advance dependencies to their newest selected revisions", usage: "<path> (<name> \| --all)", flags: Slice[*std_args.Flag]{data: ?UPDATE_FLAGS[0], size: 7}, commands: args.NO_COMMANDS, operand_min: 1, operand_max: 2, passthrough: false}` | `mach dep update` |
| `PULL` | `std.args.Command` | `std_args.Command{name: "pull", aliases: args.NO_WORDS, help: "realize dependencies at their pinned revisions", usage: "<path>", flags: Slice[*std_args.Flag]{data: ?PULL_FLAGS[0], size: 4}, commands: args.NO_COMMANDS, operand_min: 1, operand_max: 1, passthrough: false}` | `mach dep pull` |
| `OUTDATED` | `std.args.Command` | `std_args.Command{name: "outdated", aliases: args.NO_WORDS, help: "report pinned, compatible and latest releases", usage: "<path>", flags: Slice[*std_args.Flag]{data: ?OUTDATED_FLAGS[0], size: 5}, commands: args.NO_COMMANDS, operand_min: 1, operand_max: 1, passthrough: false}` | `mach dep outdated` |
| `VERIFY` | `std.args.Command` | `std_args.Command{name: "verify", aliases: args.NO_WORDS, help: "check the realized dependencies against the manifest", usage: "<path>", flags: Slice[*std_args.Flag]{data: ?VERIFY_FLAGS[0], size: 4}, commands: args.NO_COMMANDS, operand_min: 1, operand_max: 1, passthrough: false}` | `mach dep verify` |
| `ADD_POLICY` | `mach.cli.args.Policy` | `args.Policy{refused: args.NO_FLAGS, refusal: "", conflicts: Slice[args.Conflict]{data: ?ADD_CONFLICTS[0], size: ADD_CONFLICTS_N}}` | what mach holds `mach dep add` to beyond the parse |
| `UPDATE_POLICY` | `mach.cli.args.Policy` | `args.Policy{refused: args.NO_FLAGS, refusal: "", conflicts: args.NO_CONFLICTS}` | what mach holds `mach dep update` to beyond the parse |
| `VERIFY_POLICY` | `mach.cli.args.Policy` | `args.Policy{refused: args.NO_FLAGS, refusal: "", conflicts: args.NO_CONFLICTS}` | what mach holds `mach dep verify` to beyond the parse |
| `POLICY` | `mach.cli.args.Policy` | `args.Policy{refused: args.NO_FLAGS, refusal: "", conflicts: args.NO_CONFLICTS}` | what mach holds `mach dep` and its other actions to beyond the parse |
| `COMMAND` | `std.args.Command` | `std_args.Command{name: "dep", aliases: args.NO_WORDS, help: "manage project dependencies", usage: "", flags: Slice[*std_args.Flag]{data: ?PLAIN_FLAGS[0], size: 3}, commands: Slice[*std_args.Command]{data: ?ACTIONS[0], size: ACTIONS_N}, operand_min: 0, operand_max: 0, passthrough: false}` | `mach dep` |

