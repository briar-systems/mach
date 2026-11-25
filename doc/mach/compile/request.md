[mach](../README.md) / [compile](README.md) / **request**

# mach.compile.request

a build request: what one build is asked to do, from its profile and the command line

`compose` takes the selected profile's levers and sets the command line's overrides over
them, and `semantic_hash` digests the fields that decide what the build produces.

Source: [`src/compile/request.mach`](../../../src/compile/request.mach)

Uses types from [`mach.base.diagnostic.kind`](../base/diagnostic/kind.md), [`mach.base.fail`](../base/fail.md), [`mach.base.intern`](../base/intern.md), [`mach.compile.view`](view.md), [`mach.manifest.model`](../manifest/model.md).

| Type | | |
| :--- | :--- | :--- |
| [`Lever`](#lever) | `def` | a binary profile lever as the command line sets it |
| [`Overrides`](#overrides) | `rec` | what a build-shaped command sets over the profile it builds, already typed; a lever or value left unset leaves the profile's |
| [`BuildGoal`](#buildgoal) | `def` |  |
| [`LinkToken`](#linktoken) | `rec` | a `-l` value or path operand the build plan links |
| [`BuildRequest`](#buildrequest) | `rec` | owner: "" for a cell of the root manifest, or the id of the closure dependency        whose export library requires the cell; `target` and        `artifact` then name that dependency's declarations |

| On `Lever` | |
| :--- | :--- |
| [`LEVER_PROFILE`](#lever) | neither `--key` nor `--no-key` was given: the profile decides |
| [`LEVER_ON`](#lever) | `--key` |
| [`LEVER_OFF`](#lever) | `--no-key` |

| On `Overrides` | |
| :--- | :--- |
| [`overrides_init`](#overrides_init) | no override: the profile decides everything, with empty name lists and link inputs growable through `a` |
| [`compose`](#compose) | the request for a build of `m`: the selected profile's levers with the command line's over them |

| On `BuildGoal` | |
| :--- | :--- |
| [`GOAL_ARTIFACT`](#buildgoal) | the finished artifact of the unit's kind: an executable, a static archive or a shared library |
| [`GOAL_OBJECTS`](#buildgoal), [`GOAL_TESTS`](#buildgoal), [`GOAL_TEST_LIST`](#buildgoal) | |
| [`GOAL_CHECK`](#buildgoal) | `mach check`: load, resolve and check the selected artifacts' reachable source through the frontend and stop; no step runs and no product is written |
| [`test_goal`](#test_goal) | whether a goal compiles the tests, to run them or to list them |
| [`goal_name`](#goal_name) | the name a catalogued goal is displayed by |
| [`goal_verb`](#goal_verb) | the verb a catalogued goal's unit is announced with |

| On `BuildRequest` | |
| :--- | :--- |
| [`defaults`](#defaults) | a request with every field at its built-in default: an unoptimized artifact build of no cell |
| [`validate`](#validate) | refuse a request whose goal or simd mode is outside its catalog |
| [`shows_warnings`](#shows_warnings) | whether a build shows the warnings of a package, the project being built where `root` holds and a dependency otherwise: its own always, a dependency's only under `--dep-warnings` |
| [`is_release`](#is_release) | whether the request optimizes |
| [`for_cell`](#for_cell) | a copy of `base` aimed at one build cell |
| [`semantic_hash`](#semantic_hash) | the sha-256 of the request fields that decide what the build produces |

| Functions | |
| :--- | :--- |
| [`work_apply`](#work_apply) | `work`, a `-w` value, in place of the root manifest `m`'s `[project].work`: a canonical path inside the project root, taken as written |
| [`workers`](#workers) | the workers a run uses: the `--threads` count, or the host's processor count when none was given |

| Constants | |
| :--- | :--- |
| [`HASH_SIZE`](#constants) | |

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
    views:        mach.compile.view.Set;
    verify:       bool;
    passes:       bool;
    dep_warnings: bool;
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
| `views` | `mach.compile.view.Set` | the views of each unit `--emit` names, written beside its object |
| `optimize` | `Lever` | whether to optimize, or LEVER_PROFILE |
| `debug` | `Lever` | whether to emit debug information, or LEVER_PROFILE |
| `threads` | `u32` | the worker count, 0 for the host's |

### overrides_init

```mach
pub fun overrides_init(a: *std.allocator.Allocator) Overrides
```

no override: the profile decides everything, with empty name lists and link
inputs growable through `a`

### compose

```mach
pub fun compose(a: *std.allocator.Allocator, itn: *mach.base.intern.Interner, m: *mach.manifest.model.Manifest,
    o: *Overrides, root: str, goal: BuildGoal, profile: str) res[BuildRequest, mach.base.fail.Fail]
```

the request for a build of `m`: the selected profile's levers with the command line's over them

| Parameter | |
| :--- | :--- |
| `a` | allocator for refusals and the request's link token and library directory lists |
| `itn` | interner holding the manifest's names |
| `m` | the root manifest, every profile of which is checked |
| `o` | what the command line sets over the profile |
| `root` | the project root |
| `goal` | what the build is asked to produce |
| `profile` | the profile to select, "" for the manifest's default |
| **returns** | the request, its lists owned through `a` and its strings borrowed from `o`, or the refusal of the goal, the profile, a profile's `relax` list or an allocation |

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

whether a goal compiles the tests, to run them or to list them

| Parameter | |
| :--- | :--- |
| `g` | the goal |
| **returns** | true for GOAL_TESTS and GOAL_TEST_LIST |

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

a `-l` value or path operand the build plan links

## BuildRequest

```mach
pub rec BuildRequest {
    project_root: str;
    owner:        str;

    target:   str;
    profile:  str;
    artifact: str;

    goal: BuildGoal;

    # whether mirl optimises, and the passes the profile turns on or off one by one
    optimise: Optimise;
    # the command line changed a lever of the selected profile, or a `--set` wrote
    # a key of it, so a build that publishes artifacts must say where:
    # presentation only, no part of any hash
    overrides: bool;
    debug:     bool;
    # whether mach and mirl run their verifiers: opt-in, no part of any hash
    verify: bool;
    # whether mirl reports each optimisation pass that changed each function: presentation only, no part of any
    # hash, and a build that reports compiles every module, as a cached object holds no report
    passes: bool;
    # whether the linker may relax the code, which mirl's relax build option states
    linker_relax: bool;
    # whether the image keeps its symbol table, an executable its section header table, and
    # the image its unwind tables, which the link states
    symbols:  bool;
    sections: bool;
    unwind:   bool;
    # the float relaxations the profile's `relax` permits, which mirl's floats build option states
    floats: mirl.pass.relaxation.Relaxations;
    # the artifact's declared subsystem, none for the host's
    subsystem: opt[*mink.catalog.subsystem.Subsystem];
    simd:      mach.manifest.model.SimdMode;
    # the warning kinds the profile silences: presentation only, so no part of the request hash
    allow: mach.base.diagnostic.kind.KindSet;
    # the build shows its dependencies' warnings too, each under that dependency's own `allow`:
    # presentation only, so no part of the request hash
    dep_warnings: bool;
    threads:      u32;

    # the views of each unit written beside its object (compile/view)
    views: mach.compile.view.Set;
    out:   str;
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

a request with every field at its built-in default: an unoptimized artifact build of no cell

| Parameter | |
| :--- | :--- |
| **returns** | the request, its link token and library directory lists empty with no allocator |

### validate

```mach
pub fun validate(a: *std.allocator.Allocator, r: *BuildRequest) err[mach.base.fail.Fail]
```

refuse a request whose goal or simd mode is outside its catalog

| Parameter | |
| :--- | :--- |
| `a` | allocator for the refusal's message |
| `r` | the request to check |
| **returns** | ok, or the refusal naming the catalog and the value |

### shows_warnings

```mach
pub fun shows_warnings(r: *BuildRequest, root: bool) bool
```

whether a build shows the warnings of a package, the project being built where `root` holds
and a dependency otherwise: its own always, a dependency's only under `--dep-warnings`. the one
statement of the rule, which the store's governance and every pass whose only product is a
warning read through the module registry

### is_release

```mach
pub fun is_release(r: *BuildRequest) bool
```

whether the request optimizes

| Parameter | |
| :--- | :--- |
| `r` | the request |
| **returns** | true when its optimisation setting is on |

### for_cell

```mach
pub fun for_cell(base: *BuildRequest, owner: str, target: str, artifact: str,
    subsystem: opt[*mink.catalog.subsystem.Subsystem], goal: BuildGoal) BuildRequest
```

a copy of `base` aimed at one build cell

| Parameter | |
| :--- | :--- |
| `base` | the composed request |
| `owner` | "" for a cell of the root manifest, or the dependency whose export library requires it |
| `target` | the cell's target name |
| `artifact` | the cell's artifact name |
| `subsystem` | the artifact's declared subsystem, none for the host's |
| `goal` | the cell's goal |
| **returns** | the copy, sharing every list and string with `base` |

### semantic_hash

```mach
pub fun semantic_hash(a: *std.allocator.Allocator, r: *BuildRequest, digest: *u8) err[mach.base.fail.Fail]
```

the sha-256 of the request fields that decide what the build produces

| Parameter | |
| :--- | :--- |
| `a` | allocator for the hashed bytes and any refusal |
| `r` | the request, validated first |
| `digest` | where the HASH_SIZE bytes of the digest go |
| **returns** | ok, or the refusal of validation or of writing the fields |

## Functions

### work_apply

```mach
pub fun work_apply(itn: *mach.base.intern.Interner, work: str, m: *mach.manifest.model.Manifest) err[mach.base.fail.Fail]
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
| `HASH_SIZE` | `usize` | `32` |  |

