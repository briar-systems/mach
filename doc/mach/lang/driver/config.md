# mach.lang.driver.config

## rec RunArtifact

```mach
pub rec RunArtifact;
```

## fun resolve_run_artifact

```mach
pub fun resolve_run_artifact(alloc: *A.Allocator, reg: *lang_target.TargetRegistry, project_root: str, selectors: *manifest.Selectors,
output_override: str) res[RunArtifact, fail.Fail];
```

the built executable `mach run` launches: the one (artifact, target, profile)
the selectors name, which must be a `bin`

alloc: owns the result and every message
reg: the composed target registry the cell's target resolves against
project_root: the project root
selectors: the `-a`, `-t` and `-p` patterns; an artifact axis given none takes the sole `bin`
output_override: `-o`, the path to run instead of the artifact's output, or nil
ret: the executable; err from the manifest or selection, when the selection
                 names several cells or a library, or for an unusable `-o`

## fun load_config_manifest

```mach
pub fun load_config_manifest(p: *project.Project, project_root: str, m: *manifest.Manifest, c: *manifest.Cell,
owner: str, for_union: bool) err[fail.Fail];
```

configure a project for one build cell of `m`, or of one of its dependencies

p: the project being configured
project_root: the root project's directory
m: the root manifest
c: the cell; with `owner`, a target and artifact of that dependency and
              a profile of `m`
owner: "" for a cell of `m`, or the id of the closure dependency whose export
              library requires the cell
for_union: configure the editor's union of every artifact: every artifact's
              entry loads, and `{artifact.<id>.out}` resolves over every artifact
              some artifact needs
ret: ok; err from selection, template, step, link or dependency resolution

## fun module_obj_tree

```mach
pub fun module_obj_tree(a: *A.Allocator, proj_out: str, id: str) res[str, fail.Fail];
```

## fun path_in_module_obj_tree

```mach
pub fun path_in_module_obj_tree(path: str, tree: str) bool;
```

## fun step_out_reserved_error

```mach
pub fun step_out_reserved_error(a: *A.Allocator, what: str, name: str, path: str, tree: str, owner_desc: str) fail.Fail;
```

## fun select_target

```mach
pub fun select_target(p: *project.Project, t: *project.TargetEntry) err[fail.Fail];
```

