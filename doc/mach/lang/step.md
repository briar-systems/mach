# mach.lang.step

the build steps a manifest declares: each step's inputs are collected, its
invocation expanded, its outputs staged and published, and its stamp kept, so
a step runs again only when what it reads changed. the build engine runs a
project's own steps and its dependencies' steps as phases of a unit

## fun run

```mach
pub fun run(a: *A.Allocator, p: *project.Project) err[fail.Fail];
```

run the declaring project's prerequisite steps for the configured cell, in its
own directory. a dependency's cell runs them as a dependency step runs, homed
in the root's output tree through an absolute `{project.work}`

## fun dependency_out_home

```mach
pub fun dependency_out_home(alloc: *A.Allocator, project_root: str, root_out: str) res[str, fail.Fail];
```

## rec Dependency

```mach
pub rec Dependency;
```

one exported dependency step selected for a cell, by position in the closure

dependency: index into `p.config.deps`
step: index into that dependency manifest's `steps`

## fun dependency_plan

```mach
pub fun dependency_plan(p: *project.Project, isa: intern.StrId, os: intern.StrId, abi: intern.StrId) res[Vector[Dependency], fail.Fail];
```

the exported dependency steps a cell runs, in execution order: each realized
dependency's export step plan for the cell target, dependencies in closure
order. shared by `dependency_run` and the build plan display

p: the configured project
isa, os, abi: the cell target tuple
ret: the ordered steps, owned by the caller; err from the export plan

## fun dependency_run

```mach
pub fun dependency_run(p: *project.Project, isa: intern.StrId, os: intern.StrId, abi: intern.StrId) err[fail.Fail];
```

