# mach.cli.cmd.build

## fun plan_invocation

```mach
pub fun plan_invocation(a: *A.Allocator, root: str, cli: *args.CliArgs, goal: request.BuildGoal,
configure_deps: bool, ps: *session.Session) res[plan.BuildPlan, outcome.Fail];
```

initialise a session, load the manifest, and plan a build for parsed CLI arguments

a: allocator for the request
root: the project root directory
cli: the parsed arguments
goal: what to produce, objects or an executable
configure_deps: also resolve every cell against the realized dependency closure,
                filling in its exported dependency steps and export link requirements. a build
                that goes on to execute resolves each cell as it runs it, so only a caller that
                reports the plan instead of running it asks for this
ps: out; the session the plan refers to, initialised here; on an error it has already been
                torn down and the returned failure message is owned by `a`
ret: the plan, configured against the realized dependency closure, or a Fail:
                internal for session or registry setup, user for manifest, selection, request
                and dependency-configuration errors, and whatever plan.plan returns

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

