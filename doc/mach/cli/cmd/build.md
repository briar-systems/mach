# mach.cli.cmd.build

## rec Planned

```mach
pub rec Planned;
```

a command's selection, planned: the cells the selectors resolved to, and one
plan per selected profile in the order the cells name them

cells: the resolved (artifact, target, profile) cells
plans: one plan per profile; each plans that profile's cells

## fun plan_invocation

```mach
pub fun plan_invocation(a: *A.Allocator, root: str, cli: *args.CliArgs, goal: request.BuildGoal,
mode: manifest.ArtifactDefault, configure_deps: bool, ps: *session.Session) res[Planned, outcome.Fail];
```

initialise a session, load the manifest, resolve the selectors and plan a
build of every resolved cell, one plan per profile

a: allocator for the request
root: the project root directory
cli: the parsed arguments
goal: what to produce, objects or the finished artifact
mode: how an artifact axis given no pattern is filled without `--all`
configure_deps: also resolve every cell against the realized dependency closure,
                filling in its exported dependency steps and export link requirements. a build
                that goes on to execute resolves each cell as it runs it, so only a caller that
                reports the plan instead of running it asks for this
ps: out; the session the plans refer to, initialised here; on an error it has already been
                torn down and the returned failure message is owned by `a`
ret: the plans, or a Fail: internal for session or registry setup, user for manifest,
                selection, request and dependency-configuration errors, `-o` over several cells,
                and whatever planning returns

## fun announce_profile

```mach
pub fun announce_profile(r: *cli_diag.Report, planned: *Planned, bp: *plan.BuildPlan, quiet: bool);
```

name the profile a plan builds before its progress lines, when a selection
spans several profiles and the report is for a person

r: the report
planned: the planned selection
bp: the plan about to run
quiet: `--quiet`

## fun run

```mach
pub fun run(argv: **u8, inv: *args.ParsedInvocation) i64;
```

`mach build`: plan and execute a build of the project operand, rendering diagnostics and
the outcome to stderr. `-O1` is refused before parsing; `--plan` prints the effective
plan and stops without running a generator, a compiler or a linker; `-v` and `-vv`
render the phase readout after the build

argv: the full process arguments
inv: the parsed invocation for this command
ret: 0 success, 1 user error, 2 internal failure, 3 environment failure

## fun check

```mach
pub fun check(argv: **u8, inv: *args.ParsedInvocation) i64;
```

`mach check`: plan the same cells `mach build` would and run each through the
frontend only, load, resolve and sema, reporting the same diagnostics with the
same classification. no build step runs, nothing is generated, compiled, linked
or written, and a generated or embedded input that does not exist yet is an
error naming it rather than a reason to run the step

argv: the full process arguments
inv: the parsed invocation for this command
ret: 0 accepted, 1 rejected or user error, 2 internal failure, 3 environment failure

