[mach](../README.md) / [compile](README.md) / **config**

# mach.compile.config

a project configured for one build cell, and the executable `mach run` launches

configuring resolves the cell's build unit in its scope, the artifacts it requires, its
steps and their outputs, its local links and its dependency closure. `select_target`
readies the cell's target and mirl's back end for it.

<sub>source: [`src/compile/config.mach`](../../../src/compile/config.mach) · imported by 3 modules</sub>

<sub>`runner` is [`mach.manifest.runner`](../manifest/runner.md) · `A` is `std.allocator` · `manifest_select` is [`mach.manifest.select`](../manifest/select.md) · `base_source` is [`mach.base.source`](../base/source.md) · `fail` is [`mach.base.fail`](../base/fail.md) · `project` is [`mach.compile.project`](project.md) · `manifest_model` is [`mach.manifest.model`](../manifest/model.md)</sub>

| Type | | |
| :--- | :--- | :--- |
| [`RunArtifact`](#runartifact) | `rec` | the program `mach run` launches |

| On `RunArtifact` | |
| :--- | :--- |
| [`resolve_run_artifact`](#resolve_run_artifact) | the built executable `mach run` launches: the one (artifact, target, profile) the selectors name, which must be a `bin` |

| Functions | |
| :--- | :--- |
| [`load_config_manifest`](#load_config_manifest) | configure a project for one build cell of `m`, or of one of its dependencies |
| [`module_obj_tree`](#module_obj_tree) · [`path_in_module_obj_tree`](#path_in_module_obj_tree) · [`step_out_reserved_error`](#step_out_reserved_error) · [`select_target`](#select_target) | |

---

## RunArtifact

```mach
pub rec RunArtifact {
    path:          str;
    host_runnable: bool;
    target_name:   str;
    runner:        runner.Runner;
}
```

the program `mach run` launches

| Field | Type | |
| :--- | :--- | :--- |
| `path` | `str` | the built executable, or `-o` |
| `host_runnable` | `bool` | the target's os and isa are the host's |
| `target_name` | `str` | the target's declared name |
| `runner` | `runner.Runner` | the runner the target declares, none when it declares none |

### resolve_run_artifact

```mach
pub fun resolve_run_artifact(alloc: *A.Allocator, project_root: str, selectors: *manifest_select.Selectors,
    output_override: str, sources: *base_source.SourceMap) res[RunArtifact, fail.Fail]
```

the built executable `mach run` launches: the one (artifact, target, profile)
the selectors name, which must be a `bin`

| Parameter | |
| :--- | :--- |
| `alloc` | owns the result and every message |
| `project_root` | the project root |
| `selectors` | the `-a`, `-t` and `-p` patterns; an artifact axis given none takes the sole `bin` |
| `output_override` | `-o`, the path to run instead of the artifact's output, or nil |
| `sources` | receives the manifest's text, which a refusal points into |
| **returns** | the executable; err from the manifest or selection, when the selection names several cells or a library, or for an unusable `-o` |

## Functions

### load_config_manifest

```mach
pub fun load_config_manifest(p: *project.Project, project_root: str, m: *manifest_model.Manifest, closure: *project.Closure, c: *manifest_select.Cell,
    owner: str, for_union: bool) err[fail.Fail]
```

configure a project for one build cell of `m`, or of one of its dependencies

| Parameter | |
| :--- | :--- |
| `p` | the project being configured |
| `project_root` | the root project's directory |
| `m` | the root manifest |
| `closure` | `m`'s located closure, which the project borrows; nil to locate one the project owns |
| `c` | the cell; with `owner`, a target and artifact of that dependency and a profile of `m` |
| `owner` | "" for a cell of `m`, or the id of the closure dependency whose export library requires the cell |
| `for_union` | configure the editor's union of every artifact: every artifact's entry loads, and `{artifact.<id>.out}` resolves over every artifact some artifact needs |
| **returns** | ok; err from selection, template, step, link or dependency resolution |

### module_obj_tree

```mach
pub fun module_obj_tree(a: *A.Allocator, proj_out: str, id: str) res[str, fail.Fail]
```

### path_in_module_obj_tree

```mach
pub fun path_in_module_obj_tree(path: str, tree: str) bool
```

### step_out_reserved_error

```mach
pub fun step_out_reserved_error(a: *A.Allocator, what: str, name: str, path: str, tree: str, owner_desc: str) fail.Fail
```

### select_target

```mach
pub fun select_target(p: *project.Project, t: *project.TargetEntry) err[fail.Fail]
```

