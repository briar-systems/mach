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

## val PH_LOWER

```mach
pub val PH_LOWER: PhaseKind = 4
```

## val PH_CODEGEN

```mach
pub val PH_CODEGEN: PhaseKind = 5
```

## val PH_EMIT

```mach
pub val PH_EMIT: PhaseKind = 6
```

## val PH_LINK

```mach
pub val PH_LINK: PhaseKind = 7
```

## val PH_TEST

```mach
pub val PH_TEST: PhaseKind = 8
```

a test build's test objects, after the normal objects they are lowered against

## def LinkInputKind

```mach
pub def LinkInputKind: u8
```

## val LINK_STATIC

```mach
pub val LINK_STATIC: LinkInputKind = 0
```

## val LINK_DYNAMIC

```mach
pub val LINK_DYNAMIC: LinkInputKind = 1
```

## rec LinkInput

```mach
pub rec LinkInput;
```

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
              whose default library artifacts require the cell
owner_chain: the dependency chain that reaches `owner`, "" for the root
label: the unit's artifact as requirements name it: `<artifact>` for the
              root, `<owner>.<artifact>` for a dependency
dep_requires: the labels of the dependency requirement cells this cell waits on;
              empty until `plan_dependency_requirements`

## def BuildRequest

```mach
pub def BuildRequest: request.BuildRequest
```

## rec BuildPlan

```mach
pub rec BuildPlan;
```

project: the root manifest's `[project].id`

## fun plan

```mach
pub fun plan(a: *A.Allocator, itn: *intern.Interner, reg: *lang_target.TargetRegistry,
m: *manifest.Manifest, owned_req: request.BuildRequest) res[BuildPlan, outcome.Fail];
```

plan the request's own selection: `req.artifact` on `req.target`, each an
exact name or a glob, or empty for the manifest's default, as
`manifest.resolve_cells` reads them. a test goal takes the one artifact the
default selection holds, any other goal every one

a: owns the plan
itn: resolves names
reg: the target registry
m: the root manifest
owned_req: the request; its `profile` is the plan's
ret: the plan; err from request validation, selection or planning

## fun plan_cells

```mach
pub fun plan_cells(a: *A.Allocator, itn: *intern.Interner, reg: *lang_target.TargetRegistry,
m: *manifest.Manifest, owned_req: request.BuildRequest, cells: *Vector[manifest.Cell]) res[BuildPlan, outcome.Fail];
```

plan resolved cells for one profile: each cell's artifact on its target, and
before it every artifact it requires. the request's `profile` is the plan's;
a cell's own profile is its caller's grouping and is not read here

a: owns the plan
itn: resolves names
reg: the target registry
m: the root manifest
owned_req: the request the cells share
cells: the cells, from `manifest.resolve_cells`
ret: the plan; err from validation or a cell's planning, or when `-o`
           names one output and several cells are selected

## fun configure

```mach
pub fun configure(s: *session.Session, m: *manifest.Manifest, bp: *BuildPlan) err[outcome.Fail];
```

resolve every planned cell against the realized dependency closure, filling in
the exported dependency steps and export link requirements each cell consumes.
it configures projects through the same `driver.begin_build` a build runs, so a
missing realization, an invalid dependency manifest or a dependency cycle is
reported here exactly as the build would report it. nothing is fetched, no step
runs and no output is written

s: the session the plan was made in
m: the root manifest
bp: the plan, mutated in place; each unit becomes `configured`
ret: ok; the configuration failure of the first cell that has one

## fun plan_dependency_requirements

```mach
pub fun plan_dependency_requirements(s: *session.Session, m: *manifest.Manifest, bp: *BuildPlan) err[outcome.Fail];
```

add the cells a dependency's default library artifacts require to a plan made
from the root manifest. the root's closure is realized and verified exactly as
a build does; every cell compiled against a closure that holds a dependency
waits on that dependency's requirement cells, which are planned before it with
the root's profile, for every target they name in the dependency's manifest,
and against the dependency's own closure in turn. a requirement reached through
several consumers is planned once. two cells that would write one output path
are refused

s: the session the plan was made in
m: the root manifest
bp: the plan, mutated in place
ret: ok; err from closure realization, a dependency cell's planning, or an
     output collision

## fun check_plan_outputs

```mach
pub fun check_plan_outputs(a: *A.Allocator, plans: *Vector[BuildPlan]) err[outcome.Fail];
```

refuse two plans of one selection, one per profile, that would write one
file: a `[project].out` without `{profile.name}` puts every profile's
outputs at the same paths

a: owns the message
plans: the plans, one per profile
ret: ok; err naming the two cells and the path

## fun replan_unit

```mach
pub fun replan_unit(a: *A.Allocator, itn: *intern.Interner, reg: *lang_target.TargetRegistry,
s: *manifest.Scope, req: *request.BuildRequest,
prior: *BuildUnit) res[BuildUnit, outcome.Fail];
```

## fun finished_modules

```mach
pub fun finished_modules(tgt: *lang_target.Target) bool;
```

## fun resolve_token

```mach
pub fun resolve_token(a: *A.Allocator, tgt: *lang_target.Target, dirs: *Vector[str],
root: str, tok: *u8) res[LinkInput, outcome.Fail];
```

## fun resolve_requirement

```mach
pub fun resolve_requirement(a: *A.Allocator, itn: *intern.Interner, tgt: *lang_target.Target,
dirs: *Vector[str], root: str,
req: *manifest.LinkRequirement) res[LinkInput, outcome.Fail];
```

## fun join_msg

```mach
pub fun join_msg(al: *A.Allocator, va: ...) str;
```

## fun render

```mach
pub fun render(a: *A.Allocator, w: *io_writer.Writer, p: *BuildPlan, itn: *intern.Interner) err[outcome.Fail];
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
pub fun explain(a: *A.Allocator, p: *BuildPlan, itn: *intern.Interner) err[outcome.Fail];
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

the origin a failure of the phase is reported under: a build step is the
build's, a test build's test objects are generated code

## fun unknown_phase

```mach
pub fun unknown_phase(a: *A.Allocator, k: PhaseKind) outcome.Fail;
```

an internal catalog failure with a caller-owned message

