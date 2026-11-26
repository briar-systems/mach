[mach](../../README.md) / [cli](../README.md) / [cmd](README.md) / **clean**

# mach.cli.cmd.clean

`mach clean`, which removes every path a build of every cell of the project writes

the paths come from planning the project as `mach build --all` plans it. they are
removed inside the project root, and a path that escapes it or crosses a symlink is
refused.

Source: [`src/cli/cmd/clean.mach`](../../../../src/cli/cmd/clean.mach)

Uses types from [`mach.base.fail`](../../base/fail.md), [`mach.cli.args`](../args.md).

| Functions | |
| :--- | :--- |
| [`run`](#run) | `mach clean`: remove every path a build of every cell writes, as the build plans it: each artifact's output and the object, IR, assembly, test, cache, staging and dependency trees beneath each cell's work directory, dependency requirement cells included; a dependency never realized was never built, so it adds no cell |

| Constants | |
| :--- | :--- |
| [`COMMAND`](#constants) | `mach clean` |
| [`POLICY`](#constants) | what mach holds `mach clean` to beyond the parse |

---

## Functions

### run

```mach
pub fun run(cx: *mach.cli.args.Call) res[i64, mach.base.fail.Fail]
```

`mach clean`: remove every path a build of every cell writes, as the build
plans it: each artifact's output and the object, IR, assembly, test, cache,
staging and dependency trees beneath each cell's work directory, dependency
requirement cells included; a dependency never realized was never built, so
it adds no cell. they are sorted, collapsed under their ancestors,
and removed inside the project root; a path that escapes the root or crosses a symlink is refused. a missing path is
not an error. each path removed has its result line once all are tried, and a run that removed
nothing says the project is unchanged

| Parameter | |
| :--- | :--- |
| `cx` | the call |
| **returns** | exit.OK, or the shared code of the failure: exit.USER for a manifest error or an escaping target, exit.INTERNAL for an allocator failure, exit.ENVIRONMENT for a read or removal failure; a removal that fails is reported and the rest are still attempted |

## Constants

| Name | Type | Value | |
| :--- | :--- | :--- | :--- |
| `COMMAND` | `std.args.Command` | `std_args.Command{name: "clean", aliases: args.NO_WORDS, help: "remove project build output", usage: "<path>", flags: Slice[*std_args.Flag]{data: ?FLAGS[0], size: FLAGS_N}, commands: args.NO_COMMANDS, operand_min: 0, operand_max: std_args.OPERANDS_ANY, passthrough: false}` | `mach clean` |
| `POLICY` | `mach.cli.args.Policy` | `args.Policy{refused: args.NO_FLAGS, refusal: "", conflicts: args.NO_CONFLICTS}` | what mach holds `mach clean` to beyond the parse |

