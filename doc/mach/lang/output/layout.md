# mach.lang.output.layout

the layout a build owns beneath a cell's expanded `[project].work`: where its
objects, IR, assembly, tests, compiler state, step staging and dependency
artifacts go. the manifest names the work directory; everything under it is
placed here, and `trees` is what a build of a cell writes there

## val OBJ

```mach
pub val OBJ: str = "obj"
```

each module's objects, one subdirectory per project id

## val IR

```mach
pub val IR: str = "ir"
```

`-e ir` output

## val ASM

```mach
pub val ASM: str = "asm"
```

`-e asm` output

## val TEST

```mach
pub val TEST: str = "test"
```

a test run's dispatcher and logs, one subdirectory per tested artifact

## val CACHE

```mach
pub val CACHE: str = ".cache"
```

compiler-only state: nothing but the compiler reads or writes it

## val STEPS

```mach
pub val STEPS: str = ".cache/steps"
```

one fingerprint stamp per build step, under CACHE

## val STAGE

```mach
pub val STAGE: str = ".stage"
```

build step scratch space, one subdirectory per step, reset before the step runs

## val DEPENDENCY

```mach
pub val DEPENDENCY: str = "dep"
```

a dependency's artifact outputs, one subdirectory per dependency id

## val TREE_COUNT

```mach
pub val TREE_COUNT: u32    = 7
```

every tree beneath the work directory a build writes, STEPS within CACHE

## val TREES

```mach
pub val TREES:      [7]str = [7]str;
```

## fun under

```mach
pub fun under(a: *A.Allocator, work: str, tree: str) res[str, fail.Fail];
```

`tree` beneath the work directory `work`

a: owns the path
ret: the path; err when it cannot be formatted

## fun within

```mach
pub fun within(a: *A.Allocator, work: str, tree: str, name: str) res[str, fail.Fail];
```

`name` within `tree` beneath `work`: a project's objects, a tested artifact's
runs, a step's staging or a dependency's artifacts

