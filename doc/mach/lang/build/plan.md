# mach.lang.build.plan

## def PhaseKind

```mach
pub def PhaseKind: u8
```

## val PH_DEP_STEPS

```mach
pub val PH_DEP_STEPS: PhaseKind = 0
```

## val PH_LOAD

```mach
pub val PH_LOAD: PhaseKind = 1
```

## val PH_STEPS

```mach
pub val PH_STEPS: PhaseKind = 2
```

## val PH_SEMA

```mach
pub val PH_SEMA: PhaseKind = 3
```

## val PH_CODEGEN

```mach
pub val PH_CODEGEN: PhaseKind = 4
```

each unit lowered and compiled, or its object restored

## val PH_EMIT

```mach
pub val PH_EMIT: PhaseKind = 5
```

## val PH_LINK

```mach
pub val PH_LINK: PhaseKind = 6
```

## val PH_TEST

```mach
pub val PH_TEST: PhaseKind = 7
```

a test build's test objects, after the normal objects they are lowered against

## rec BuildUnit

```mach
pub rec BuildUnit;
```

one planned cell: an artifact on a target for a profile, with every decision
the engine executes. `plan` fills everything the root manifest decides; `configure`
adds what the realized dependency closure decides

steps: the declaring manifest's prerequisite steps in execution order, by name
dep_steps: exported dependency steps in execution order as `<dep id>.<step>`;
              empty until `configure`
export_links: the dependency closure's exported link requirements for this
              cell, in link order; empty until `configure`
configured: `configure` ran, so `dep_steps` and `export_links` are effective
requires: the declaring manifest's artifacts this cell requires, by name
owner: "" for a cell of the root manifest, or the id of the dependency
              whose export library requires the cell
owner_chain: the dependency chain that reaches `owner`, "" for the root
label: the unit's artifact as requirements name it: `<artifact>` for the
              root, `<owner>.<artifact>` for a dependency
dep_requires: the labels of the dependency requirement cells this cell waits on;
              empty until `plan_dependency_requirements`

## rec BuildPlan

```mach
pub rec BuildPlan;
```

project:  the root manifest's `[project].id`
names:    the interner the plan's names live in; a session runs the plan only
          when it holds them
manifest: the root manifest the plan was made from, borrowed; it outlives the plan
closure:  the manifest's dependency closure, located once by
          `plan_dependency_requirements`; every cell borrows it
located:  whether `closure` was located as a build needs it, every dependency
          realized, which running the plan requires
sources:  the texts the manifest and closure were read into, the map of the session
          that located the closure; every session running the plan shares it

## fun plan

```mach
pub fun plan(a: *A.Allocator, itn: *intern.Interner, m: *manifest_model.Manifest, owned_req: request.BuildRequest) res[BuildPlan, fail.Fail];
```

plan the request's own selection: `req.artifact` on `req.target`, each an
exact name or a glob, or empty for the manifest's default, as
`manifest_select.resolve_cells` reads them. a test goal takes the one artifact the
default selection holds, any other goal every one

a: owns the plan
itn: resolves names
m: the root manifest
owned_req: the request; its `profile` is the plan's
ret: the plan; err from request validation, selection or planning

## fun plan_to_run

```mach
pub fun plan_to_run(a: *A.Allocator, s: *session.Session, m: *manifest_model.Manifest, owned_req: request.BuildRequest) res[BuildPlan, fail.Fail];
```

`plan`, with the plan's dependency closure located in `s` as
`plan_dependency_requirements` locates it, so the plan is ready to run there

s: the session the plan's names live in; `itn` of `plan` is its interner
ret: the plan; err from planning or from locating the closure

## fun plan_cells

```mach
pub fun plan_cells(a: *A.Allocator, itn: *intern.Interner, m: *manifest_model.Manifest, owned_req: request.BuildRequest, cells: *Vector[manifest_select.Cell]) res[BuildPlan, fail.Fail];
```

plan resolved cells for one profile: each cell's artifact on its target, and
before it every artifact it requires. the request's `profile` is the plan's;
a cell's own profile is its caller's grouping and is not read here

a: owns the plan
itn: resolves names
m: the root manifest
owned_req: the request the cells share
cells: the cells, from `manifest_select.resolve_cells`
ret: the plan; err from validation or a cell's planning, or when `-o`
           names one output and several cells are selected

## fun configure

```mach
pub fun configure(s: *session.Session, bp: *BuildPlan) err[fail.Fail];
```

resolve every planned cell against the plan's dependency closure, filling in
the exported dependency steps and export link requirements each cell consumes.
it configures projects through the same `driver.begin_build` a build runs, so
what configuring a cell refuses is reported here exactly as the build would
report it. nothing is fetched, no step runs and no output is written

s: the session the plan was made in
bp: the plan, its closure located; mutated in place, each unit becomes `configured`
ret: ok; the configuration failure of the first cell that has one

## fun plan_dependency_requirements

```mach
pub fun plan_dependency_requirements(s: *session.Session, bp: *BuildPlan, unrealized: driver_closure.Unrealized) err[fail.Fail];
```

locate the root's dependency closure, once, into the plan, and add the cells a
dependency's export library requires to it. a plan runs only once this has
located its closure; every cell compiled against a closure that holds a dependency
waits on that dependency's requirement cells, which are planned before it with
the root's profile, for every target they name in the dependency's manifest,
and against the dependency's own closure in turn. a requirement reached through
several consumers is planned once. two cells that would write one output path
are refused

s: the session the plan was made in
bp: the plan, mutated in place
unrealized: what a dependency nothing is realized for is; a plan that leaves
            one out lists what was built and never runs
ret: ok; err from locating the closure, a dependency cell's planning, or an
            output collision

## fun check_plan_outputs

```mach
pub fun check_plan_outputs(a: *A.Allocator, plans: *Vector[BuildPlan]) err[fail.Fail];
```

refuse two plans of one selection, one per profile, that would write one
file: a `[project].work` without `{profile.name}` puts every profile's
outputs at the same paths

a: owns the message
plans: the plans, one per profile
ret: ok; err naming the two cells and the path

## fun under_dir

```mach
pub fun under_dir(path: str, dir: str) bool;
```

whether `path` lies under the directory `dir`

## fun requirement_input

```mach
pub fun requirement_input(a: *A.Allocator, itn: *intern.Interner, root: str,
req: *manifest_link.LinkRequirement) res[input.Input, fail.Fail];
```

the input a manifest link requirement names: a framework or a library by
name, or a file located as written or under `root`

## fun render

```mach
pub fun render(a: *A.Allocator, w: *io_writer.Writer, p: *BuildPlan, itn: *intern.Interner) err[fail.Fail];
```

render the effective plan: one block per selected cell, naming the project,
target, profile, artifact, entry, output paths, ordered prerequisite steps and
link requirements. a plan the caller never configured says so instead of
claiming an empty dependency closure. a phase outside the catalog is an
internal failure whose message `a` owns

w: the sink; `explain` passes stdout
p: the plan
itn: resolves interned names

## fun explain

```mach
pub fun explain(a: *A.Allocator, p: *BuildPlan, itn: *intern.Interner) err[fail.Fail];
```

## fun phase_name

```mach
pub fun phase_name(k: PhaseKind) opt[str];
```

the name of a declared build phase, absent for an unknown catalog member

## fun phase_origin

```mach
pub fun phase_origin(k: PhaseKind) diagnostic.Origin;
```

the origin a failure of the phase is reported under when no product raised it, the phase's
own work outside every computation: a step, a file it reads or writes, the engine itself. a
build step is the build's, and a test build's test objects are generated code

## fun unknown_phase

```mach
pub fun unknown_phase(a: *A.Allocator, k: PhaseKind) fail.Fail;
```

an internal catalog failure with a caller-owned message

