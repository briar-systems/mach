[mach](../README.md) / [build](README.md) / **step**

# mach.build.step

the build steps a manifest declares: each step's inputs are collected, its
invocation expanded, its outputs staged and published, and its stamp kept, so
a step runs again only when what it reads changed. the build engine runs a
project's own steps and its dependencies' steps as phases of a unit

Source: [`src/build/step.mach`](../../../src/build/step.mach)

Uses types from [`mach.base.fail`](../base/fail.md), [`mach.base.intern`](../base/intern.md), [`mach.compile.project`](../compile/project.md).

| Type | | |
| :--- | :--- | :--- |
| [`Dependency`](#dependency) | `rec` | one exported dependency step selected for a cell, by position in the closure |

| Functions | |
| :--- | :--- |
| [`run`](#run) | run the declaring project's prerequisite steps for the configured cell, in its own directory |
| [`dependency_plan`](#dependency_plan) | the exported dependency steps a cell runs, in execution order: each realized dependency's export step plan for the cell target, dependencies in closure order |
| [`dependency_run`](#dependency_run) | |

---

## Dependency

```mach
pub rec Dependency {
    dependency: u32;
    step:       u32;
}
```

one exported dependency step selected for a cell, by position in the closure

| Field | Type | |
| :--- | :--- | :--- |
| `dependency` | `u32` | index into `p.config.deps` |
| `step` | `u32` | index into that dependency manifest's `steps` |

## Functions

### run

```mach
pub fun run(a: *std.allocator.Allocator, p: *mach.compile.project.Project) err[mach.base.fail.Fail]
```

run the declaring project's prerequisite steps for the configured cell, in its
own directory. a dependency's cell runs them as a dependency step runs, homed
in the root's output tree through an absolute `{project.work}`

### dependency_plan

```mach
pub fun dependency_plan(p: *mach.compile.project.Project, isa: mach.base.intern.StrId, os: mach.base.intern.StrId, abi: mach.base.intern.StrId) res[Vector[Dependency], mach.base.fail.Fail]
```

the exported dependency steps a cell runs, in execution order: each realized
dependency's export step plan for the cell target, dependencies in closure
order. shared by `dependency_run` and the build plan display

| Parameter | |
| :--- | :--- |
| `p` | the configured project isa, os, abi: the cell target tuple |
| **returns** | the ordered steps, owned by the caller; err from the export plan |

### dependency_run

```mach
pub fun dependency_run(p: *mach.compile.project.Project, isa: mach.base.intern.StrId, os: mach.base.intern.StrId, abi: mach.base.intern.StrId) err[mach.base.fail.Fail]
```

