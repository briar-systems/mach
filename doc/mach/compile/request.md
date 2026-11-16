[mach](../README.md) / [compile](README.md) / **request**

# mach.compile.request

a build request: what one build is asked to do, from its profile and the command line

`compose` takes the selected profile's levers and sets the command line's overrides over
them, and `semantic_hash` digests the fields that decide what the build produces.

<sub>source: [`src/compile/request.mach`](../../../src/compile/request.mach) · imported by 14 modules</sub>

<sub>`A` is `std.allocator` · `intern` is [`mach.base.intern`](../base/intern.md) · `manifest_model` is [`mach.manifest.model`](../manifest/model.md) · `fail` is [`mach.base.fail`](../base/fail.md) · `relaxation` is `mirl.pass.relaxation` · `mink_subsystem` is `mink.catalog.subsystem` · `diagnostic_kind` is [`mach.base.diagnostic.kind`](../base/diagnostic/kind.md)</sub>

| Type | | |
| :--- | :--- | :--- |
| [`Lever`](#lever) | `def` | a binary profile lever as the command line sets it |
| [`Overrides`](#overrides) | `rec` | what a build-shaped command sets over the profile it builds, already typed; a lever or value left unset leaves the profile's |
| [`BuildGoal`](#buildgoal) | `def` |  |
| [`LinkToken`](#linktoken) | `rec` | reached only by mach-lsp |
| [`BuildRequest`](#buildrequest) | `rec` | owner: "" for a cell of the root manifest, or the id of the closure dependency        whose export library requires the cell; `target` and        `artifact` then name that dependency's declarations |

| On `Lever` | |
| :--- | :--- |
| [`LEVER_PROFILE`](#lever) | neither `--key` nor `--no-key` was given: the profile decides |
| [`LEVER_ON`](#lever) | `--key` |
| [`LEVER_OFF`](#lever) | `--no-key` |

| On `Overrides` | |
| :--- | :--- |
| [`overrides_init`](#overrides_init) | no override: the profile decides everything, with empty name lists and link inputs growable through `a` |
| [`compose`](#compose) | |

| On `BuildGoal` | |
| :--- | :--- |
| [`GOAL_ARTIFACT`](#buildgoal) | the finished artifact of the unit's kind: an executable, a static archive or a shared library |
| [`GOAL_OBJECTS`](#buildgoal) · [`GOAL_TESTS`](#buildgoal) · [`GOAL_TEST_LIST`](#buildgoal) | |
| [`GOAL_CHECK`](#buildgoal) | `mach check`: load, resolve and check the selected artifacts' reachable source through the frontend and stop; no step runs and no product is written |
| [`test_goal`](#test_goal) | |
| [`goal_name`](#goal_name) | the name a catalogued goal is displayed by |
| [`goal_verb`](#goal_verb) | the verb a catalogued goal's unit is announced with |

| On `BuildRequest` | |
| :--- | :--- |
| [`defaults`](#defaults) · [`validate`](#validate) · [`is_release`](#is_release) · [`for_cell`](#for_cell) · [`semantic_hash`](#semantic_hash) | |

| Functions | |
| :--- | :--- |
| [`work_apply`](#work_apply) | `work`, a `-w` value, in place of the root manifest `m`'s `[project].work`: a canonical path inside the project root, taken as written |
| [`workers`](#workers) | the workers a run uses: the `--threads` count, or the host's processor count when none was given |

| Constants | |
| :--- | :--- |
| [`OPT_DEBUG`](#constants) | a build's optimization lever: off, or the level mirl's schedule optimizes at |
| [`OPT_RELEASE`](#constants) · [`HASH_SIZE`](#constants) | |

---

## Lever

```mach
pub def Lever: u8
```

a binary profile lever as the command line sets it

| Name | Value | |
| :--- | :--- | :--- |
| `LEVER_PROFILE` | `0` | neither `--key` nor `--no-key` was given: the profile decides |
| `LEVER_ON` | `1` | `--key` |
| `LEVER_OFF` | `2` | `--no-key` |

## Overrides

```mach
pub rec Overrides {
    output:       *u8;
    work:         *u8;
    emit_asm:     bool;
    emit_ir:      bool;
    verify:       bool;
    optimize:     Lever;
    debug:        Lever;
    threads:      u32;
    include_deps: bool;
    no_cache:     bool;
    declassified: bool;
    link_tokens:  Vector[*u8];
    lib_dirs:     Vector[*u8];
}
```

what a build-shaped command sets over the profile it builds, already typed;
a lever or value left unset leaves the profile's

| Field | Type | |
| :--- | :--- | :--- |
| `output` | `*u8` | the `-o` path, nil when absent |
| `work` | `*u8` | the `-w` work directory, nil when absent |
| `optimize` | `Lever` | whether to optimize, or LEVER_PROFILE |
| `debug` | `Lever` | whether to emit debug information, or LEVER_PROFILE |
| `threads` | `u32` | the worker count, 0 for the host's |

### overrides_init

```mach
pub fun overrides_init(a: *A.Allocator) Overrides
```

no override: the profile decides everything, with empty name lists and link
inputs growable through `a`

### compose

```mach
pub fun compose(a: *A.Allocator, itn: *intern.Interner, m: *manifest_model.Manifest,
    o: *Overrides, root: str, goal: BuildGoal, profile: str) res[BuildRequest, fail.Fail]
```

## BuildGoal

```mach
pub def BuildGoal: u8
```

| Name | Value | |
| :--- | :--- | :--- |
| `GOAL_ARTIFACT` | `0` | the finished artifact of the unit's kind: an executable, a static archive or a shared library |
| `GOAL_OBJECTS` | `1` |  |
| `GOAL_TESTS` | `2` |  |
| `GOAL_TEST_LIST` | `3` |  |
| `GOAL_CHECK` | `4` | `mach check`: load, resolve and check the selected artifacts' reachable source through the frontend and stop; no step runs and no product is written |

### test_goal

```mach
pub fun test_goal(g: BuildGoal) bool
```

### goal_name

```mach
pub fun goal_name(goal: BuildGoal) str
```

the name a catalogued goal is displayed by

### goal_verb

```mach
pub fun goal_verb(goal: BuildGoal) str
```

the verb a catalogued goal's unit is announced with

## LinkToken

```mach
pub rec LinkToken {
    text: str;
}
```

reached only by mach-lsp

## BuildRequest

```mach
pub rec BuildRequest {
    project_root: str;
    owner:        str;

    target:   str;
    profile:  str;
    artifact: str;

    goal: BuildGoal;

    # the level mirl's schedule runs at
    opt: u8;
    # the command line changed a lever of the selected profile, so a build that
    # publishes artifacts must say where: presentation only, no part of any hash
    overrides: bool;
    debug:     bool;
    # whether mach and mirl run their verifiers: opt-in, no part of any hash
    verify: bool;
    # whether the linker may relax the code, which mirl's relax build option states
    linker_relax: bool;
    # the float relaxations the profile's `relax` permits, which mirl's floats build option states
    floats: relaxation.Relaxations;
    # the artifact's declared subsystem, none for the host's
    subsystem: opt[*mink_subsystem.Subsystem];
    simd:      manifest_model.SimdMode;
    # the warning kinds the profile silences: presentation only, so no part of the request hash
    allow:   diagnostic_kind.KindSet;
    threads: u32;

    emit_ir:  bool;
    emit_asm: bool;
    out:      str;
    # `-w`, the root manifest's work directory in place of its `[project].work`;
    # "" for none
    work: str;

    link_tokens: Vector[LinkToken];
    lib_dirs:    Vector[str];

    include_deps: bool;
    no_cache:     bool;
    object_cache: bool;
    # a test run's selection: the tests whose qualified name contains it, all when empty.
    # it shapes the dispatcher and never an object
    test_filter: str;
    # a check lists the source's declassifications: presentation only, so no part of the request hash
    declassified: bool;
}
```

owner: "" for a cell of the root manifest, or the id of the closure dependency
       whose export library requires the cell; `target` and
       `artifact` then name that dependency's declarations

### defaults

```mach
pub fun defaults() BuildRequest
```

### validate

```mach
pub fun validate(a: *A.Allocator, r: *BuildRequest) err[fail.Fail]
```

### is_release

```mach
pub fun is_release(r: *BuildRequest) bool
```

### for_cell

```mach
pub fun for_cell(base: *BuildRequest, owner: str, target: str, artifact: str,
    subsystem: opt[*mink_subsystem.Subsystem], goal: BuildGoal) BuildRequest
```

### semantic_hash

```mach
pub fun semantic_hash(a: *A.Allocator, r: *BuildRequest, digest: *u8) err[fail.Fail]
```

## Functions

### work_apply

```mach
pub fun work_apply(itn: *intern.Interner, work: str, m: *manifest_model.Manifest) err[fail.Fail]
```

`work`, a `-w` value, in place of the root manifest `m`'s `[project].work`: a
canonical path inside the project root, taken as written

### workers

```mach
pub fun workers(asked: u32) u32
```

the workers a run uses: the `--threads` count, or the host's processor count when none was given

| Parameter | |
| :--- | :--- |
| `asked` | the count, 0 when none was given |
| **returns** | at least 1 |

## Constants

| Name | Type | Value | |
| :--- | :--- | :--- | :--- |
| `OPT_DEBUG` | `u8` | `0` | a build's optimization lever: off, or the level mirl's schedule optimizes at |
| `OPT_RELEASE` | `u8` | `2` |  |
| `HASH_SIZE` | `usize` | `32` |  |

