[mach](../README.md) / [build](README.md) / **plan**

# mach.build.plan

the build plan: the units a request selects, with every decision the engine executes

`plan` decides what the root manifest decides. `plan_dependency_requirements` locates the
dependency closure and adds the cells a dependency's export library requires, and
`configure` fills in what the closure decides. `render` prints a plan for `mach build --plan`.

Source: [`src/build/plan.mach`](../../../src/build/plan.mach)

Uses types from [`mach.base.diagnostic`](../base/diagnostic.md), [`mach.base.fail`](../base/fail.md), [`mach.base.intern`](../base/intern.md), [`mach.base.source`](../base/source.md), [`mach.compile.closure`](../compile/closure.md), [`mach.compile.project`](../compile/project.md), [`mach.compile.request`](../compile/request.md), [`mach.manifest.link`](../manifest/link.md), [`mach.manifest.model`](../manifest/model.md), [`mach.manifest.select`](../manifest/select.md), [`mach.session`](../session.md), [`mach.target.input`](../target/input.md).

| Type | | |
| :--- | :--- | :--- |
| [`PhaseKind`](#phasekind) | `def` |  |
| [`BuildUnit`](#buildunit) | `rec` | one planned cell: an artifact on a target for a profile, with every decision the engine executes |
| [`BuildPlan`](#buildplan) | `rec` | a build's plan: the cells it runs in execution order, with the names, manifest and dependency closure they are read against |

| On `PhaseKind` | |
| :--- | :--- |
| [`PH_DEP_STEPS`](#phasekind), [`PH_LOAD`](#phasekind), [`PH_STEPS`](#phasekind), [`PH_SEMA`](#phasekind) | |
| [`PH_CODEGEN`](#phasekind) | each unit lowered and compiled, or its object restored |
| [`PH_EMIT`](#phasekind), [`PH_LINK`](#phasekind) | |
| [`PH_TEST`](#phasekind) | a test build's test objects, after the normal objects they are lowered against |
| [`phase_name`](#phase_name) | the name of a declared build phase, absent for an unknown catalog member |
| [`phase_origin`](#phase_origin) | the origin a failure of the phase is reported under when no product raised it, the phase's own work outside every computation: a step, a file it reads or writes, the engine itself |
| [`unknown_phase`](#unknown_phase) | an internal catalog failure with a caller-owned message |

| On `BuildPlan` | |
| :--- | :--- |
| [`plan`](#plan) | plan the request's own selection: `req.artifact` on `req.target`, each an exact name or a glob, or empty for the manifest's default, as `manifest_select.resolve_cells` reads them |
| [`plan_to_run`](#plan_to_run) | `plan`, with the plan's dependency closure located in `s` as `plan_dependency_requirements` locates it, so the plan is ready to run there |
| [`plan_cells`](#plan_cells) | plan resolved cells for one profile: each cell's artifact on its target, and before it every artifact it requires |
| [`configure`](#configure) | resolve every planned cell against the plan's dependency closure, filling in the exported dependency steps and export link requirements each cell consumes |
| [`cell_request`](#cell_request) | the request cell `u` of `bp` builds for |
| [`plan_dependency_requirements`](#plan_dependency_requirements) | locate the root's dependency closure, once, into the plan, and add the cells a dependency's export library requires to it |
| [`check_plan_outputs`](#check_plan_outputs) | refuse two plans of one selection, one per profile, that would write one file: a `[project].work` without `{profile.name}` puts every profile's outputs at the same paths |
| [`render`](#render) | render the effective plan: one block per selected cell, naming the project, target, profile, artifact, entry, output paths, ordered prerequisite steps and link requirements |

| Functions | |
| :--- | :--- |
| [`under_dir`](#under_dir) | whether `path` lies under the directory `dir` |
| [`requirement_input`](#requirement_input) | the input a manifest link requirement names: a framework or a library by name, or a file located as written, a local entry's path anchored already |

---

## PhaseKind

```mach
pub def PhaseKind: u8
```

| Name | Value | |
| :--- | :--- | :--- |
| `PH_DEP_STEPS` | `0` |  |
| `PH_LOAD` | `1` |  |
| `PH_STEPS` | `2` |  |
| `PH_SEMA` | `3` |  |
| `PH_CODEGEN` | `4` | each unit lowered and compiled, or its object restored |
| `PH_EMIT` | `5` |  |
| `PH_LINK` | `6` |  |
| `PH_TEST` | `7` | a test build's test objects, after the normal objects they are lowered against |

### phase_name

```mach
pub fun phase_name(k: PhaseKind) opt[str]
```

the name of a declared build phase, absent for an unknown catalog member

### phase_origin

```mach
pub fun phase_origin(k: PhaseKind) mach.base.diagnostic.Origin
```

the origin a failure of the phase is reported under when no product raised it, the phase's
own work outside every computation: a step, a file it reads or writes, the engine itself. a
build step is the build's, and a test build's test objects are generated code

### unknown_phase

```mach
pub fun unknown_phase(a: *std.allocator.Allocator, k: PhaseKind) mach.base.fail.Fail
```

an internal catalog failure with a caller-owned message

## BuildUnit

```mach
pub rec BuildUnit {
    target_entry: *mach.compile.project.TargetEntry;
    profile:      str;
    optimise:     mirl.pass.setting.Optimise;
    goal:         mach.compile.request.BuildGoal;
    out_path:     str;
    # the expanded `[project].work`: the build owns the layout beneath it
    out_dir: str;
    obj_dir: str;
    # the compiler-only state of the cell's output directory, `<out>/.cache`
    cache_dir: str;
    # the tree of each view of compile/view the build writes, "" for one it does not
    view_dirs:    [compile_view.COUNT]str;
    test_dir:     str;
    link_inputs:  Vector[mach.target.input.Input];
    phases:       Vector[PhaseKind];
    steps:        Vector[str];
    dep_steps:    Vector[str];
    export_links: Vector[mach.manifest.link.LinkRequirement];
    configured:   bool;

    sel_target:   str;
    sel_artifact: str;
    is_required:  bool;
    built_only:   bool;
    requires:     Vector[str];
    subsystem:    opt[*mink.catalog.subsystem.Subsystem];

    owner:        str;
    owner_chain:  str;
    label:        str;
    dep_requires: Vector[str];
}
```

one planned cell: an artifact on a target for a profile, with every decision
the engine executes. `plan` fills everything the root manifest decides; `configure`
adds what the realized dependency closure decides

| Field | Type | |
| :--- | :--- | :--- |
| `steps` | `Vector[str]` | the declaring manifest's prerequisite steps in execution order, by name |
| `dep_steps` | `Vector[str]` | exported dependency steps in execution order as `<dep id>.<step>`; empty until `configure` |
| `export_links` | `Vector[mach.manifest.link.LinkRequirement]` | the dependency closure's exported link requirements for this cell, in link order; empty until `configure` |
| `configured` | `bool` | `configure` ran, so `dep_steps` and `export_links` are effective |
| `built_only` | `bool` | a test run's cell on a finished-module target, which links no test executable: built as its artifact, or checked for a listing, and not run |
| `requires` | `Vector[str]` | the declaring manifest's artifacts this cell requires, by name |
| `owner` | `str` | "" for a cell of the root manifest, or the id of the dependency whose export library requires the cell |
| `owner_chain` | `str` | the dependency chain that reaches `owner`, "" for the root |
| `label` | `str` | the unit's artifact as requirements name it: `<artifact>` for the root, `<owner>.<artifact>` for a dependency |
| `dep_requires` | `Vector[str]` | the labels of the dependency requirement cells this cell waits on; empty until `plan_dependency_requirements` |

## BuildPlan

```mach
pub rec BuildPlan {
    request:  mach.compile.request.BuildRequest;
    project:  str;
    units:    Vector[BuildUnit];
    names:    *mach.base.intern.Interner;
    manifest: *mach.manifest.model.Manifest;
    closure:  mach.compile.project.Closure;
    located:  bool;
    sources:  *mach.base.source.SourceMap;
}
```

a build's plan: the cells it runs in execution order, with the names, manifest and
dependency closure they are read against

| Field | Type | |
| :--- | :--- | :--- |
| `request` | `mach.compile.request.BuildRequest` | the request the plan was made for, each cell's request derived from it |
| `project` | `str` | the root manifest's `[project].id` |
| `units` | `Vector[BuildUnit]` | the planned cells in execution order, every requirement before the cells that need it |
| `names` | `*mach.base.intern.Interner` | the interner the plan's names live in; a session runs the plan only when it holds them |
| `manifest` | `*mach.manifest.model.Manifest` | the root manifest the plan was made from, borrowed; it outlives the plan |
| `closure` | `mach.compile.project.Closure` | the manifest's dependency closure, located once by `plan_dependency_requirements`; every cell borrows it |
| `located` | `bool` | whether `closure` was located as a build needs it, every dependency realized, which running the plan requires |
| `sources` | `*mach.base.source.SourceMap` | the texts the manifest and closure were read into, the map of the session that located the closure; every session running the plan shares it |

### plan

```mach
pub fun plan(a: *std.allocator.Allocator, itn: *mach.base.intern.Interner, m: *mach.manifest.model.Manifest, owned_req: mach.compile.request.BuildRequest) res[BuildPlan, mach.base.fail.Fail]
```

plan the request's own selection: `req.artifact` on `req.target`, each an
exact name or a glob, or empty for the manifest's default, as
`manifest_select.resolve_cells` reads them. a test goal takes the one artifact the
default selection holds, any other goal every one

| Parameter | |
| :--- | :--- |
| `a` | owns the plan |
| `itn` | resolves names |
| `m` | the root manifest |
| `owned_req` | the request; its `profile` is the plan's |
| **returns** | the plan; err from request validation, selection or planning |

### plan_to_run

```mach
pub fun plan_to_run(a: *std.allocator.Allocator, s: *mach.session.Session, m: *mach.manifest.model.Manifest, owned_req: mach.compile.request.BuildRequest) res[BuildPlan, mach.base.fail.Fail]
```

`plan`, with the plan's dependency closure located in `s` as
`plan_dependency_requirements` locates it, so the plan is ready to run there

| Parameter | |
| :--- | :--- |
| `s` | the session the plan's names live in; `itn` of `plan` is its interner |
| **returns** | the plan; err from planning or from locating the closure |

### plan_cells

```mach
pub fun plan_cells(a: *std.allocator.Allocator, itn: *mach.base.intern.Interner, m: *mach.manifest.model.Manifest, owned_req: mach.compile.request.BuildRequest, cells: *Vector[mach.manifest.select.Cell]) res[BuildPlan, mach.base.fail.Fail]
```

plan resolved cells for one profile: each cell's artifact on its target, and
before it every artifact it requires. the request's `profile` is the plan's;
a cell's own profile is its caller's grouping and is not read here

| Parameter | |
| :--- | :--- |
| `a` | owns the plan |
| `itn` | resolves names |
| `m` | the root manifest |
| `owned_req` | the request the cells share |
| `cells` | the cells, from `manifest_select.resolve_cells` |
| **returns** | the plan; err from validation or a cell's planning, or when `-o` names one output and several cells are selected |

### configure

```mach
pub fun configure(s: *mach.session.Session, bp: *BuildPlan) err[mach.base.fail.Fail]
```

resolve every planned cell against the plan's dependency closure, filling in
the exported dependency steps and export link requirements each cell consumes.
it configures projects through the same `compile.begin_build` a build runs, so
what configuring a cell refuses is reported here exactly as the build would
report it. nothing is fetched, no step runs and no output is written

| Parameter | |
| :--- | :--- |
| `s` | the session the plan was made in |
| `bp` | the plan, its closure located; mutated in place, each unit becomes `configured` |
| **returns** | ok; the configuration failure of the first cell that has one |

### cell_request

```mach
pub fun cell_request(bp: *BuildPlan, u: *BuildUnit) mach.compile.request.BuildRequest
```

the request cell `u` of `bp` builds for. `-o` names a selected cell's output,
so a required cell keeps its declared one

### plan_dependency_requirements

```mach
pub fun plan_dependency_requirements(s: *mach.session.Session, bp: *BuildPlan, unrealized: mach.compile.closure.Unrealized) err[mach.base.fail.Fail]
```

locate the root's dependency closure, once, into the plan, and add the cells a
dependency's export library requires to it. a plan runs only once this has
located its closure; every cell compiled against a closure that holds a dependency
waits on that dependency's requirement cells, which are planned before it with
the root's profile, for every target they name in the dependency's manifest,
and against the dependency's own closure in turn. a requirement reached through
several consumers is planned once. two cells that would write one output path
are refused

| Parameter | |
| :--- | :--- |
| `s` | the session the plan was made in |
| `bp` | the plan, mutated in place |
| `unrealized` | what a dependency nothing is realized for is; a plan that leaves one out lists what was built and never runs |
| **returns** | ok; err from locating the closure, a dependency cell's planning, or an output collision |

### check_plan_outputs

```mach
pub fun check_plan_outputs(a: *std.allocator.Allocator, plans: *Vector[BuildPlan]) err[mach.base.fail.Fail]
```

refuse two plans of one selection, one per profile, that would write one
file: a `[project].work` without `{profile.name}` puts every profile's
outputs at the same paths

| Parameter | |
| :--- | :--- |
| `a` | owns the message |
| `plans` | the plans, one per profile |
| **returns** | ok; err naming the two cells and the path |

### render

```mach
pub fun render(a: *std.allocator.Allocator, w: *std.io.writer.Writer, p: *BuildPlan, itn: *mach.base.intern.Interner) err[mach.base.fail.Fail]
```

render the effective plan: one block per selected cell, naming the project,
target, profile, artifact, entry, output paths, ordered prerequisite steps and
link requirements. a plan the caller never configured says so instead of
claiming an empty dependency closure. a phase outside the catalog is an
internal failure whose message `a` owns

| Parameter | |
| :--- | :--- |
| `w` | the sink; `mach build --plan` passes the command's stdout |
| `p` | the plan |
| `itn` | resolves interned names |

## Functions

### under_dir

```mach
pub fun under_dir(path: str, dir: str) bool
```

whether `path` lies under the directory `dir`

### requirement_input

```mach
pub fun requirement_input(a: *std.allocator.Allocator, itn: *mach.base.intern.Interner, req: *mach.manifest.link.LinkRequirement) res[mach.target.input.Input, mach.base.fail.Fail]
```

the input a manifest link requirement names: a framework or a library by
name, or a file located as written, a local entry's path anchored already

