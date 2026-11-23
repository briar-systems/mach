[mach](../../README.md) / [host](../README.md) / [output](README.md) / **layout**

# mach.host.output.layout

the layout a build owns beneath a cell's expanded `[project].work`: where its
objects, IR, assembly, tests, compiler state, step staging and dependency
artifacts go. the manifest names the work directory; everything under it is
placed here, and `trees` is what a build of a cell writes there

Source: [`src/host/output/layout.mach`](../../../../src/host/output/layout.mach)

Uses types from [`mach.base.fail`](../../base/fail.md).

| Functions | |
| :--- | :--- |
| [`under`](#under) | `tree` beneath the work directory `work` |
| [`within`](#within) | `name` within `tree` beneath `work`: a project's objects, a tested artifact's runs, a step's staging or a dependency's artifacts |

| Constants | |
| :--- | :--- |
| [`OBJ`](#constants) | each module's objects, one subdirectory per project id |
| [`IR`](#constants) | `-e ir` output |
| [`ASM`](#constants) | `-e asm` output |
| [`TEST`](#constants) | a test run's dispatcher and logs, one subdirectory per tested artifact |
| [`CACHE`](#constants) | compiler-only state: nothing but the compiler reads or writes it |
| [`STEPS`](#constants) | one fingerprint stamp per build step, under CACHE |
| [`STAGE`](#constants) | build step scratch space, one subdirectory per step, reset before the step runs |
| [`DEPENDENCY`](#constants) | a dependency's artifact outputs, one subdirectory per dependency id |
| [`TREE_COUNT`](#constants) | every tree beneath the work directory a build writes, STEPS within CACHE |
| [`TREES`](#constants) | |

---

## Functions

### under

```mach
pub fun under(a: *std.allocator.Allocator, work: str, tree: str) res[str, mach.base.fail.Fail]
```

`tree` beneath the work directory `work`

| Parameter | |
| :--- | :--- |
| `a` | owns the path |
| **returns** | the path; err when it cannot be formatted |

### within

```mach
pub fun within(a: *std.allocator.Allocator, work: str, tree: str, name: str) res[str, mach.base.fail.Fail]
```

`name` within `tree` beneath `work`: a project's objects, a tested artifact's
runs, a step's staging or a dependency's artifacts

## Constants

| Name | Type | Value | |
| :--- | :--- | :--- | :--- |
| `OBJ` | `str` | `"obj"` | each module's objects, one subdirectory per project id |
| `IR` | `str` | `"ir"` | `-e ir` output |
| `ASM` | `str` | `"asm"` | `-e asm` output |
| `TEST` | `str` | `"test"` | a test run's dispatcher and logs, one subdirectory per tested artifact |
| `CACHE` | `str` | `".cache"` | compiler-only state: nothing but the compiler reads or writes it |
| `STEPS` | `str` | `".cache/steps"` | one fingerprint stamp per build step, under CACHE |
| `STAGE` | `str` | `".stage"` | build step scratch space, one subdirectory per step, reset before the step runs |
| `DEPENDENCY` | `str` | `"dep"` | a dependency's artifact outputs, one subdirectory per dependency id |
| `TREE_COUNT` | `u32` | `7` | every tree beneath the work directory a build writes, STEPS within CACHE |
| `TREES` | `[7]str` | `[7]str{OBJ, IR, ASM, TEST, CACHE, STAGE, DEPENDENCY}` |  |

