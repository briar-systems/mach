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

steps: the root manifest's prerequisite steps in execution order, by name
dep_steps: exported dependency steps in execution order as `<dep id>.<step>`;
              empty until `configure`
export_links: the dependency closure's exported link requirements for this
              cell, in link order; empty until `configure`
configured: `configure` ran, so `dep_steps` and `export_links` are effective

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
pub fun plan(a: *A.Allocator, itn: *intern.Interner, reg: *target.TargetRegistry,
m: *manifest.Manifest, owned_req: request.BuildRequest) res[BuildPlan, outcome.Fail];
```

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

## fun replan_unit

```mach
pub fun replan_unit(a: *A.Allocator, itn: *intern.Interner, reg: *target.TargetRegistry,
m: *manifest.Manifest, req: *request.BuildRequest,
prior: *BuildUnit) res[BuildUnit, outcome.Fail];
```

## fun finished_modules

```mach
pub fun finished_modules(tgt: *target.Target) bool;
```

## fun resolve_token

```mach
pub fun resolve_token(a: *A.Allocator, tgt: *target.Target, dirs: *Vector[str],
root: str, tok: *u8) res[LinkInput, outcome.Fail];
```

## fun resolve_requirement

```mach
pub fun resolve_requirement(a: *A.Allocator, itn: *intern.Interner, tgt: *target.Target,
dirs: *Vector[str], root: str,
req: *manifest.LinkRequirement) res[LinkInput, outcome.Fail];
```

## fun join_msg

```mach
pub fun join_msg(al: *A.Allocator, va: ...) str;
```

## fun render

```mach
pub fun render(a: *A.Allocator, w: *writer.Writer, p: *BuildPlan, itn: *intern.Interner) err[outcome.Fail];
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

## fun unknown_phase

```mach
pub fun unknown_phase(a: *A.Allocator, k: PhaseKind) outcome.Fail;
```

an internal catalog failure with a caller-owned message

