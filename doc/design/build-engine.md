# The Build Engine

**Status**: draft for owner markup
**Scope**: the layer between `main()` and the frontend — CLI, orchestration, outcome reporting

The build layer is a data pipeline. Intent is composed into a request, resolved into a
plan, executed by an engine, and reported as an outcome:

```
Manifest ──► compose(manifest, args) ──► BuildRequest ──► plan ──► BuildPlan ──► execute ──► BuildOutcome
```

Every arrow is a pure function of its inputs. The records are the contracts; the
functions own the logic between them.

---

## 1. Vocabulary

| Noun | Is |
|---|---|
| `CliArgs` | the parsed command line, nothing more |
| `Manifest` | the parsed `mach.toml` |
| `Project` | the loaded module graph |
| `BuildRequest` | composed intent: the decisions for one invocation, source-agnostic |
| `BuildPlan` | resolved reality: units × phases × artifacts, fully concrete |
| `BuildOutcome` | the structured result: artifacts, exit derivation |
| `Fail` | the orchestration layer's single error currency |

Two rules give the records their shapes:

- **A request carries decisions, not ingredients.** Composers own all precedence
  (manifest default < profile < invoker); a request field is the settled answer. No
  field records how it was decided.
- **A plan carries facts.** Anything that only exists per unit — an expanded output
  template, a link token resolved against a target's library directories — is produced
  by the planner. A request may hold a name; a plan holds the thing the name resolved to.

---

## 2. `BuildRequest`

```mach
# what the build produces
pub def BuildGoal: u8;
pub val GOAL_EXECUTE:   BuildGoal = 0;   # link the selected artifact
pub val GOAL_OBJECTS:   BuildGoal = 1;   # per-module objects only
pub val GOAL_TESTS:     BuildGoal = 2;   # test dispatcher + artifact list
pub val GOAL_TEST_LIST: BuildGoal = 3;   # collect tests, build nothing

# one link input as the invoker spelled it; the planner resolves it
pub rec LinkToken {
    text: str;
}

# the complete intent for one build invocation
#
# composed by the CLI, a matrix cell derivation, or an embedding consumer
# (LSP, tooling). every field is a settled decision. immutable once composed;
# owned by the request arena.
pub rec BuildRequest {
    project_root: str;

    # selection: names the planner resolves against the manifest
    target:       str;              # empty selects the manifest default
    profile:      str;
    artifact:     str;
    want_lib:     bool;
    all_targets:  bool;

    goal:         BuildGoal;

    # codegen decisions, precedence already applied
    opt:          u8;               # pipeline level
    debug:        bool;
    pie:          bool;
    simd:         manifest.SimdMode;
    verify_ir:    bool;
    jobs:         u32;              # concrete worker count, always >= 1

    # emissions and output
    emit_ir:      bool;
    emit_asm:     bool;
    out:          str;              # requested artifact path; empty selects the manifest template

    # link inputs in invocation order
    link_tokens:  Vector[LinkToken];
    lib_dirs:     Vector[str];

    # test scope
    include_deps: bool;

    verbosity:    u8;
    quiet:        bool;
}
```

**Composition.** `compose(manifest, args) -> R.Result[BuildRequest, Fail]` settles every
field: the profile's opt unless the invoker passed `-O`; the host CPU count unless
`--jobs`; the profile's simd posture. Embedding consumers construct requests directly or
through the same helper. The manifest is loaded once and handed forward to the planner.

**Identity.** The serialized request (`std.encoding.binary`) hashes to the build's cache
key. Because fields are settled values, the key reflects effective configuration: any
change that alters what the build would do — from either the manifest or the command
line — changes the key.

---

## 3. `BuildPlan`

```mach
# a phase in a unit's ordered execution list
pub def PhaseKind: u8;
pub val PH_DEP_STEPS: PhaseKind = 0;   # user-defined steps of dependencies
pub val PH_LOAD:      PhaseKind = 1;   # module-graph load + resolve
pub val PH_STEPS:     PhaseKind = 2;   # user-defined project steps
pub val PH_SEMA:      PhaseKind = 3;
pub val PH_LOWER:     PhaseKind = 4;
pub val PH_CODEGEN:   PhaseKind = 5;
pub val PH_EMIT:      PhaseKind = 6;   # object tree / ir / asm materialization
pub val PH_LINK:      PhaseKind = 7;   # executable, archive, shared object, or dispatcher per goal

# one fully resolved link input
pub def LinkInputKind: u8;
pub val LINK_STATIC:  LinkInputKind = 0;   # object or archive path on disk
pub val LINK_DYNAMIC: LinkInputKind = 1;   # soname recorded for the loader

pub rec LinkInput {
    kind: LinkInputKind;
    text: str;                      # absolute path (STATIC) or soname (DYNAMIC)
}

# one build unit: a (target, profile, artifact) cell, everything concrete
pub rec BuildUnit {
    target_entry: *project.TargetEntry;
    profile:      str;
    opt_level:    u8;
    goal:         BuildGoal;
    out_path:     str;                    # final artifact path, templates expanded
    obj_dir:      str;
    ir_dir:       str;                    # empty when ir emission is off
    asm_dir:      str;
    link_inputs:  Vector[LinkInput];
    phases:       Vector[PhaseKind];      # ordered, goal- and manifest-derived
}

# the resolved plan for one request
pub rec BuildPlan {
    request: *BuildRequest;               # provenance
    units:   Vector[BuildUnit];           # one normally; one per cell under all_targets
}
```

The planner resolves selection names to manifest entries, expands the matrix under
`all_targets`, expands output templates, resolves link tokens against the unit's target
(library directories, shared-object probing, format-specific dependency naming), and
derives each unit's phase list. User-defined manifest steps and compiler phases occupy
one ordered list: a unit's execution is its `phases` vector, nothing else.

The plan is inspectable data. `mach build --explain` prints it; `-v` narrates its
execution; both walk the same value, so neither can disagree with what the engine does.

---

## 4. `Fail` and `BuildOutcome`

```mach
# the orchestration layer's single error currency
#
# `reported` marks a failure already filed on the diagnostic sink as a located
# diagnostic; the renderer prints nothing further for it. `internal`
# distinguishes compiler faults from user errors for exit-code derivation.
pub rec Fail {
    reported: bool;
    message:  str;      # empty when reported
    internal: bool;
}

# one produced artifact
pub def ArtifactKind: u8;
pub val ART_BINARY:  ArtifactKind = 0;
pub val ART_OBJECTS: ArtifactKind = 1;
pub val ART_ARCHIVE: ArtifactKind = 2;
pub val ART_SHARED:  ArtifactKind = 3;
pub val ART_TESTS:   ArtifactKind = 4;

pub rec Artifact {
    kind: ArtifactKind;
    path: str;
}

# the structured result of executing a plan
pub rec BuildOutcome {
    ok:        bool;
    internal:  bool;
    artifacts: Vector[Artifact];
    tests:     Vector[TestArtifact];    # test goals only
}
```

Orchestration functions return `R.Result[T, Fail]`. Located diagnostics go to the
session sink; `Fail.reported` ties the two channels together. Rendering happens exactly
once, in the CLI's render step: flush the sink, print unreported failures, derive the
exit code (`ok` → 0, user error → 1, internal → 2). No printing occurs below the render
layer.

---

## 5. The engine

`execute(plan, backing, outcome_alloc, progress, render) -> R.Result[BuildOutcome, Fail]`
walks each unit's phase list in order, dispatching on `PhaseKind`. Each unit runs in its
own fresh session over `backing` (torn down between cells), so the engine takes
allocators and sinks rather than a session; the planning session only outlives the
plan's borrowed strings. `PhaseKind` values are tags — a unit's execution order is its
`phases` vector, nothing else. The engine owns:

- **incrementality** — each pass registers its own query computes at entry; the request
  hash keys the build-level entries. Query computes reach the request through
  `Project.req`, the query engine's context channel;
- **parallelism** — the codegen worker pool is an executor property, sized by
  `request.jobs`;
- **progress** — the engine emits phase/item events through a ctx + fn-pointer pair;
  the readout renders them. Consumers that want silence pass none.

The `Session` provides infrastructure to the engine: allocator, interner, source map,
diagnostic sink, query database. It carries no build intent; the request and plan are
the engine's only sources of decision.

---

## 6. Module map

```
src/lang/build.mach            facade: compose, plan, execute, explain
src/lang/build/request.mach    BuildRequest, LinkToken, composition, request hashing
src/lang/build/plan.mach       BuildPlan, BuildUnit, PhaseKind, the planner
src/lang/build/engine.mach     the executor: phase dispatch, worker pool,
                               query-pipeline binding, progress events
src/lang/build/outcome.mach    Fail, Artifact, BuildOutcome
src/lang/build/emit.mach       object-tree / ir / asm materialization; executable,
                               archive, and shared-object emission
src/lang/build/testing.mach    dispatcher synthesis and TestArtifact collection
```

Dependency direction: `outcome ← request ← plan ← engine ← facade`. The engine consumes
`driver/*` (manifest loading, dependency resolution, the module-graph load walk),
`manifest`, `session`, `query`, and the backend as services. The `driver` subtree is the
graph layer: it loads and resolves; the engine orchestrates.

Consumers:

- `cmd/build.mach`, `cmd/run.mach`, `cmd/testing.mach`: parse → compose → plan →
  execute → render. Each command is a request composition and an outcome rendering.
- `editor.mach` (LSP): composes requests directly for whole-project operations; its
  per-file micro-pipeline is unchanged.

---

## 7. The CLI layer

Each command declares its flags as data:

```mach
# one command-line flag: spelling, arity, one-line help text
pub rec FlagSpec {
    name:      str;
    has_value: bool;
    doc:       str;
}
```

One shared parser walks a command's table and produces `CliArgs`, consumption tracking,
and unknown-flag rejection. `mach help <cmd>` and usage lines render from the same
table, so help and behavior cannot disagree.

`argv` is read in exactly one module. It does not cross the compose boundary. This is a
grep-checkable invariant: `argc` appears nowhere outside `src/cli/args.mach`.

---

## 8. Memory policy

Three named arenas with declared lifetimes:

| Arena | Lives for | Holds |
|---|---|---|
| request | one invocation | `CliArgs`, `BuildRequest`, composed strings |
| unit | one `BuildUnit` execution | `Project`, compile state, plan-unit scratch |
| outcome | until the caller is done | `BuildOutcome`, artifact paths, test artifacts |

Results always land in the outcome arena. Phase-local transients use `fin`-scoped
teardown. The unit arena tears down between matrix cells. Ownership is a property of
the arena a value lives in, never per-function prose.

---

## 9. Adoption

Eight stages, in build order. Each stage is complete when it lands — the area it covers
is fully on the new model, green against the full gate (seed build, self-build timing
baseline, unit suite, int leg) in the same commit. Rendered diagnostic and readout text
is byte-stable through stages 1 and 4 (the int golden suite enforces this); any wording
changes ship separately.

| # | Stage | Builds |
|---|---|---|
| 0 | Vocabulary | the nouns of §1 applied throughout the layer |
| 1 | Outcome | `Fail`, `BuildOutcome`, the single render point, derived exit codes |
| 2 | Request | `BuildRequest` + composition; intent has one carrier |
| 3 | Plan | the planner, `BuildUnit`, link resolution at plan time, `--explain` |
| 4 | Engine | the executor, `build/emit`, `build/testing`, progress events |
| 5 | Flags | declarative tables, generated help |
| 6 | Arenas | the named-arena policy, `fin` discipline |
| 7 | Foundations | path segment primitives in `std.types.path`, `MANIFEST_FILE` in `lang/manifest`, the full cleanup pass over `driver/` |

Stages 0–2 are sequential; 3–4 depend on 2; 5–7 are mutually independent after 4.

Constraints held throughout: the LSP's vendored pin sees only additive API until it
advances; no closures — phase dispatch switches on `PhaseKind`, events use the ctx +
fn-pointer idiom; the timing gate runs at every stage.
