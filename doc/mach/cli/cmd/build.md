[mach](../../README.md) / [cli](../README.md) / [cmd](README.md) / **build**

# mach.cli.cmd.build

`mach build` and `mach check`, and the planning of a selection other commands share

a project build makes one plan per selected profile and executes each, and an entry
build states its whole project on the command line. `mach check` plans the same cells
and runs only the front end. `mach clean` and `mach test` plan through
`plan_selection` and `plan_invocation`.

<sub>source: [`src/cli/cmd/build.mach`](../../../../src/cli/cmd/build.mach) · imported by 4 modules</sub>

<sub>`manifest_model` is [`mach.manifest.model`](../../manifest/model.md) · `manifest_select` is [`mach.manifest.select`](../../manifest/select.md) · `build_plan` is [`mach.build.plan`](../../build/plan.md) · `A` is `std.allocator` · `args` is [`mach.cli.args`](../args.md) · `request` is [`mach.compile.request`](../../compile/request.md) · `base_source` is [`mach.base.source`](../../base/source.md) · `session` is [`mach.session`](../../session.md) · `fail` is [`mach.base.fail`](../../base/fail.md) · `compile_closure` is [`mach.compile.closure`](../../compile/closure.md) · `cli_diagnostic` is [`mach.cli.diagnostic`](../diagnostic.md) · `std_args` is `std.args`</sub>

| Type | | |
| :--- | :--- | :--- |
| [`Planned`](#planned) | `rec` | a command's selection, planned: the cells the selectors resolved to, and one plan per selected profile in the order the cells name them |

| On `Planned` | |
| :--- | :--- |
| [`plan_invocation`](#plan_invocation) | initialise a session, load the manifest, resolve the selectors and plan a build of every resolved cell, one plan per profile |
| [`plan_selection`](#plan_selection) | resolve the selectors against a loaded manifest and plan every resolved cell, one plan per profile |
| [`announce_profile`](#announce_profile) | name the profile a plan builds before its progress lines, when a selection spans several profiles and the report is for a person |

| Functions | |
| :--- | :--- |
| [`run`](#run) | `mach build`: plan and execute a build of the project operand, rendering diagnostics to stderr as each unit reports them |
| [`check`](#check) | `mach check`: plan the same cells `mach build` would and run each through the frontend only, load, resolve and sema, reporting the same diagnostics with the same classification |

| Constants | |
| :--- | :--- |
| [`COMMAND`](#constants) | `mach build` |
| [`POLICY`](#constants) | what mach holds `mach build` to beyond the parse |
| [`CHECK`](#constants) | `mach check` |
| [`CHECK_POLICY`](#constants) | what mach holds `mach check` to beyond the parse |

---

## Planned

```mach
pub rec Planned {
    manifest: *manifest_model.Manifest;
    cells:    Vector[manifest_select.Cell];
    plans:    Vector[build_plan.BuildPlan];
}
```

a command's selection, planned: the cells the selectors resolved to, and one
plan per selected profile in the order the cells name them

| Field | Type | |
| :--- | :--- | :--- |
| `manifest` | `*manifest_model.Manifest` | the root manifest the plans were made from, in the planning session |
| `cells` | `Vector[manifest_select.Cell]` | the resolved (artifact, target, profile) cells |
| `plans` | `Vector[build_plan.BuildPlan]` | one plan per profile; each plans that profile's cells |

### plan_invocation

```mach
pub fun plan_invocation(a: *A.Allocator, root: str, cli: *args.CliArgs, goal: request.BuildGoal,
    mode: manifest_select.ArtifactDefault, configure_deps: bool, sources: *base_source.SourceMap,
    ps: *session.Session) res[Planned, fail.Fail]
```

initialise a session, load the manifest, resolve the selectors and plan a
build of every resolved cell, one plan per profile

| Parameter | |
| :--- | :--- |
| `a` | allocator for the request |
| `root` | the project root directory |
| `cli` | the parsed arguments |
| `goal` | what to produce, objects or the finished artifact |
| `mode` | how an artifact axis given no pattern is filled without `--all` |
| `configure_deps` | also resolve every cell against the realized dependency closure, filling in its exported dependency steps and export link requirements. a build that goes on to execute resolves each cell as it runs it, so only a caller that reports the plan instead of running it asks for this |
| `sources` | the invocation's source map, which the session's texts go to |
| `ps` | out; the session the plans refer to, initialised here; on an error it has already been torn down and the returned failure message is owned by `a` |
| **returns** | the plans, or a Fail: internal for session or registry setup, user for manifest, selection, request and dependency-configuration errors, `-o` over several cells, and whatever planning returns |

### plan_selection

```mach
pub fun plan_selection(a: *A.Allocator, ps: *session.Session, m: *manifest_model.Manifest, root: str, cli: *args.CliArgs,
    goal: request.BuildGoal, mode: manifest_select.ArtifactDefault, configure_deps: bool,
    unrealized: compile_closure.Unrealized) res[Planned, fail.Fail]
```

resolve the selectors against a loaded manifest and plan every resolved cell,
one plan per profile. plans of different profiles are not checked against
each other: a caller that runs them all checks they write disjoint outputs

| Parameter | |
| :--- | :--- |
| `a` | allocator for the requests |
| `ps` | the session the manifest was loaded in; owns the plans |
| `m` | the root manifest |
| `unrealized` | what a dependency nothing is realized for is |
| **returns** | the plans; err from selection, a request, planning or dependency configuration |

### announce_profile

```mach
pub fun announce_profile(r: *cli_diagnostic.Report, planned: *Planned, bp: *build_plan.BuildPlan, quiet: bool)
```

name the profile a plan builds before its progress lines, when a selection
spans several profiles and the report is for a person

| Parameter | |
| :--- | :--- |
| `r` | the report |
| `planned` | the planned selection |
| `bp` | the plan about to run |
| `quiet` | `--quiet` |

## Functions

### run

```mach
pub fun run(cx: *args.Call) res[i64, fail.Fail]
```

`mach build`: plan and execute a build of the project operand, rendering diagnostics to
stderr as each unit reports them. `--plan` prints the
effective plan and stops without running a generator, a compiler or a linker; `-v` adds
each phase row as the phase finishes, and `-vv` each module or file as it finishes

| Parameter | |
| :--- | :--- |
| `cx` | the call |
| **returns** | 0 success, 1 user error, 2 internal failure, 3 environment failure; every failure is reported on the call's report with the phase it came from |

### check

```mach
pub fun check(cx: *args.Call) res[i64, fail.Fail]
```

`mach check`: plan the same cells `mach build` would and run each through the
frontend only, load, resolve and sema, reporting the same diagnostics with the
same classification. no build step runs, nothing is generated, compiled, linked
or written, and a generated or embedded input that does not exist yet is an
error naming it rather than a reason to run the step

| Parameter | |
| :--- | :--- |
| `cx` | the call |
| **returns** | 0 accepted, 1 rejected or user error, 2 internal failure, 3 environment failure |

## Constants

| Name | Type | Value | |
| :--- | :--- | :--- | :--- |
| `COMMAND` | `std_args.Command` | `std_args.Command{name: "build", aliases: args.NO_WORDS, help: "compile a project or one entry file", usage: "<path\|entry.mach> [inputs...]", flags: Slice[*std_args.Flag]{data: ?BUILD_FLAGS[0], size: BUILD_FLAGS_N}, commands: args.NO_COMMANDS, operand_min: 0, operand_max: std_args.OPERANDS_ANY, passthrough: false}` | `mach build` |
| `POLICY` | `args.Policy` | `args.Policy{refused: args.NO_FLAGS, refusal: "", conflicts: Slice[args.Conflict]{data: ?BUILD_CONFLICTS[0], size: BUILD_CONFLICTS_N}}` | what mach holds `mach build` to beyond the parse |
| `CHECK` | `std_args.Command` | `std_args.Command{name: "check", aliases: args.NO_WORDS, help: "check project source without building", usage: "<path>", flags: Slice[*std_args.Flag]{data: ?CHECK_FLAGS[0], size: CHECK_FLAGS_N}, commands: args.NO_COMMANDS, operand_min: 1, operand_max: 1, passthrough: false}` | `mach check` |
| `CHECK_POLICY` | `args.Policy` | `args.Policy{refused: args.NO_FLAGS, refusal: "", conflicts: args.NO_CONFLICTS}` | what mach holds `mach check` to beyond the parse |

