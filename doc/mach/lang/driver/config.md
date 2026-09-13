# mach.lang.driver.config

## fun resolve_artifact_path

```mach
pub fun resolve_artifact_path(alloc: *A.Allocator, project_root: str, pick: *manifest.Selection) res[str, outcome.Fail];
```

## rec RunArtifact

```mach
pub rec RunArtifact;
```

## fun resolve_run_artifact

```mach
pub fun resolve_run_artifact(alloc: *A.Allocator, project_root: str, pick: *manifest.Selection,
output_override: str) res[RunArtifact, outcome.Fail];
```

## fun load_project_config

```mach
pub fun load_project_config(p: *project.Project, project_root: str, manifest_path: str, pick: *manifest.Selection, for_union: bool, is_test: bool) err[outcome.Fail];
```

## fun load_config_manifest

```mach
pub fun load_config_manifest(p: *project.Project, project_root: str, m: *manifest.Manifest, pick: *manifest.Selection, for_union: bool, is_test: bool) err[outcome.Fail];
```

## fun select_target

```mach
pub fun select_target(p: *project.Project, t: *project.TargetEntry) err[outcome.Fail];
```

## fun wildcard_match

```mach
pub fun wildcard_match(pat: str, name: str) bool;
```

## fun validate_contained_ancestors

```mach
pub fun validate_contained_ancestors(alloc: *A.Allocator, root: str, rel: str, label: str) err[outcome.Fail];
```

## fun glob_shape_ok

```mach
pub fun glob_shape_ok(pattern: str) bool;
```

## fun execute_steps

```mach
pub fun execute_steps(a: *A.Allocator, p: *project.Project, project_root: str) err[outcome.Fail];
```

## fun dep_out_home

```mach
pub fun dep_out_home(alloc: *A.Allocator, project_root: str, root_out: str) res[str, outcome.Fail];
```

## rec DependencyStep

```mach
pub rec DependencyStep;
```

one exported dependency step selected for a cell, by position in the closure

dependency: index into `p.config.deps`
step: index into that dependency manifest's `steps`

## fun plan_dep_steps

```mach
pub fun plan_dep_steps(p: *project.Project, isa: str, os: str, abi: str) res[Vector[DependencyStep], outcome.Fail];
```

the exported dependency steps a cell runs, in execution order: each realized
dependency's export step plan for the cell target, dependencies in closure
order. shared by `execute_dep_steps` and the build plan display

p: the configured project
isa, os, abi: the cell target tuple
ret: the ordered steps, owned by the caller; err from the export plan

## fun execute_dep_steps

```mach
pub fun execute_dep_steps(p: *project.Project, isa: str, os: str, abi: str) err[outcome.Fail];
```

