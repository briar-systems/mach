# mach.lang.build.request

## rec CliArgs

```mach
pub rec CliArgs;
```

## fun is_object_path

```mach
pub fun is_object_path(tok: *u8) bool;
```

## def SubsystemFlag

```mach
pub def SubsystemFlag: u8
```

## val SUBSYSTEM_FLAG_NONE

```mach
pub val SUBSYSTEM_FLAG_NONE:    SubsystemFlag = 0
```

## val SUBSYSTEM_FLAG_CONSOLE

```mach
pub val SUBSYSTEM_FLAG_CONSOLE: SubsystemFlag = 1
```

## val SUBSYSTEM_FLAG_GUI

```mach
pub val SUBSYSTEM_FLAG_GUI:     SubsystemFlag = 2
```

## fun subsystem_flag_from_name

```mach
pub fun subsystem_flag_from_name(name: str) opt[SubsystemFlag];
```

## fun subsystem_from_flag

```mach
pub fun subsystem_from_flag(f: SubsystemFlag) opt[of.Subsystem];
```

## def BuildGoal

```mach
pub def BuildGoal: u8
```

## val GOAL_EXECUTE

```mach
pub val GOAL_EXECUTE: BuildGoal = 0
```

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

## val EMIT_HELP

```mach
pub val EMIT_HELP: str = "obj | exe (default exe; obj stops at the objects)"
```

## val EMIT_ERROR

```mach
pub val EMIT_ERROR: str = "--emit expects 'obj' or 'exe'"
```

## rec LinkToken

```mach
pub rec LinkToken;
```

## rec BuildRequest

```mach
pub rec BuildRequest;
```

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

## fun goal_name

```mach
pub fun goal_name(goal: BuildGoal) str;
```

## fun validate

```mach
pub fun validate(a: *A.Allocator, r: *BuildRequest) err[outcome.Fail];
```

## fun release

```mach
pub fun release(r: *BuildRequest) bool;
```

## fun compose

```mach
pub fun compose(a: *A.Allocator, itn: *intern.Interner, m: *manifest.Manifest,
cli: *CliArgs, root: str, goal: BuildGoal,
pick: *manifest.Selection) res[BuildRequest, outcome.Fail];
```

## fun for_cell

```mach
pub fun for_cell(base: *BuildRequest, target: str, artifact: str, want_lib: bool,
subsystem: of.Subsystem, goal: BuildGoal) BuildRequest;
```

## val HASH_SIZE

```mach
pub val HASH_SIZE: usize = 32
```

## fun request_hash

```mach
pub fun request_hash(a: *A.Allocator, r: *BuildRequest, digest: *u8) err[outcome.Fail];
```

## fun semantic_hash

```mach
pub fun semantic_hash(a: *A.Allocator, r: *BuildRequest, digest: *u8) err[outcome.Fail];
```

