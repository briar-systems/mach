# mach.lang.driver.request

## def Lever

```mach
pub def Lever: u8
```

a binary profile lever as the command line sets it

## val LEVER_PROFILE

```mach
pub val LEVER_PROFILE: Lever = 0
```

neither `--key` nor `--no-key` was given: the profile decides

## val LEVER_ON

```mach
pub val LEVER_ON: Lever = 1
```

`--key`

## val LEVER_OFF

```mach
pub val LEVER_OFF: Lever = 2
```

`--no-key`

## rec Overrides

```mach
pub rec Overrides;
```

what a build-shaped command sets over the profile it builds, already typed;
a lever or value left unset leaves the profile's

output: the `-o` path, nil when absent
work: the `-w` work directory, nil when absent
subsystem: the declared subsystem, SUBSYSTEM_FLAG_NONE for the artifact's own
optimize: whether to optimize, or LEVER_PROFILE
debug: whether to emit debug information, or LEVER_PROFILE
simd: the vector lowering mode, none for the profile's
jobs: the worker count, 0 for the host's

## fun overrides_init

```mach
pub fun overrides_init(a: *A.Allocator) Overrides;
```

no override: the profile decides everything, with empty name lists and link
inputs growable through `a`

## def SubsystemFlag

```mach
pub def SubsystemFlag: u8
```

## val SUBSYSTEM_FLAG_CONSOLE

```mach
pub val SUBSYSTEM_FLAG_CONSOLE: SubsystemFlag = 1
```

## fun subsystem_flag_from_name

```mach
pub fun subsystem_flag_from_name(name: str) opt[SubsystemFlag];
```

the flag a subsystem name sets, none for a name no flag sets

## fun subsystem_from_flag

```mach
pub fun subsystem_from_flag(f: SubsystemFlag) opt[catalog_subsystem.Subsystem];
```

## val OPT_DEBUG

```mach
pub val OPT_DEBUG:   u8 = 0
```

a build's optimization lever: off, or the level mirl's schedule optimizes at

## val OPT_RELEASE

```mach
pub val OPT_RELEASE: u8 = 2
```

## def BuildGoal

```mach
pub def BuildGoal: u8
```

## val GOAL_ARTIFACT

```mach
pub val GOAL_ARTIFACT: BuildGoal = 0
```

the finished artifact of the unit's kind: an executable, a static archive or a shared library

## val GOAL_OBJECTS

```mach
pub val GOAL_OBJECTS: BuildGoal = 1
```

## val GOAL_TESTS

```mach
pub val GOAL_TESTS: BuildGoal = 2
```

## val GOAL_TEST_LIST

```mach
pub val GOAL_TEST_LIST: BuildGoal = 3
```

## val GOAL_CHECK

```mach
pub val GOAL_CHECK: BuildGoal = 4
```

`mach check`: load, resolve and check the selected artifacts' reachable source
through the frontend and stop; no step runs and no product is written

## rec LinkToken

```mach
pub rec LinkToken;
```

reached only by mach-lsp

## rec BuildRequest

```mach
pub rec BuildRequest;
```

owner: "" for a cell of the root manifest, or the id of the closure dependency
       whose export library requires the cell; `target` and
       `artifact` then name that dependency's declarations

## fun defaults

```mach
pub fun defaults() BuildRequest;
```

## fun test_goal

```mach
pub fun test_goal(g: BuildGoal) bool;
```

## fun goal_from_emit_name

```mach
pub fun goal_from_emit_name(name: str) opt[BuildGoal];
```

## fun goal_emit_name

```mach
pub fun goal_emit_name(goal: BuildGoal) str;
```

the name `--emit` gives a catalogued goal, "" for one it does not name

## fun goal_name

```mach
pub fun goal_name(goal: BuildGoal) str;
```

the name a catalogued goal is displayed by

## fun goal_verb

```mach
pub fun goal_verb(goal: BuildGoal) str;
```

the verb a catalogued goal's unit is announced with

## fun validate

```mach
pub fun validate(a: *A.Allocator, r: *BuildRequest) err[fail.Fail];
```

## fun is_release

```mach
pub fun is_release(r: *BuildRequest) bool;
```

## fun compose

```mach
pub fun compose(a: *A.Allocator, itn: *intern.Interner, m: *manifest.Manifest,
o: *Overrides, root: str, goal: BuildGoal, profile: str) res[BuildRequest, fail.Fail];
```

## fun work_apply

```mach
pub fun work_apply(itn: *intern.Interner, work: str, m: *manifest.Manifest) err[fail.Fail];
```

`work`, a `-w` value, in place of the root manifest `m`'s `[project].work`: a
canonical path inside the project root, taken as written

## fun for_cell

```mach
pub fun for_cell(base: *BuildRequest, owner: str, target: str, artifact: str,
subsystem: catalog_subsystem.Subsystem, goal: BuildGoal) BuildRequest;
```

## fun workers

```mach
pub fun workers(asked: u32) u32;
```

the workers a run uses: the `--jobs` count, or the host's processor count when none was given

asked: the count, 0 when none was given
ret: at least 1

## val HASH_SIZE

```mach
pub val HASH_SIZE: usize = 32
```

## fun semantic_hash

```mach
pub fun semantic_hash(a: *A.Allocator, r: *BuildRequest, digest: *u8) err[fail.Fail];
```

