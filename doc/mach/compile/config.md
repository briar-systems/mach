[mach](../README.md) / [compile](README.md) / **config**

# mach.compile.config

a project configured for one build cell

configuring resolves the cell's build unit in its scope, the artifacts it requires, its
steps and their outputs, its local links and its dependency closure. `select_target`
readies the cell's target and mirl's back end for it.

Source: [`src/compile/config.mach`](../../../src/compile/config.mach)

Uses types from [`mach.base.fail`](../base/fail.md), [`mach.compile.project`](project.md), [`mach.manifest.model`](../manifest/model.md), [`mach.manifest.select`](../manifest/select.md).

| Functions | |
| :--- | :--- |
| [`load_config_manifest`](#load_config_manifest) | configure a project for one build cell of `m`, or of one of its dependencies |
| [`module_obj_tree`](#module_obj_tree) | the module object tree of project `id` under the build's work directory |
| [`path_in_module_obj_tree`](#path_in_module_obj_tree) | whether `path` is the directory `tree` or lies under it |
| [`step_out_reserved_error`](#step_out_reserved_error) | the refusal of a step output written into a directory the compiler owns |
| [`select_target`](#select_target) | resolve the cell's target and open mirl's back end for it |

---

## Functions

### load_config_manifest

```mach
pub fun load_config_manifest(p: *mach.compile.project.Project, project_root: str, m: *mach.manifest.model.Manifest, closure: *mach.compile.project.Closure, c: *mach.manifest.select.Cell,
    owner: str, for_union: bool) err[mach.base.fail.Fail]
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
pub fun module_obj_tree(a: *std.allocator.Allocator, proj_out: str, id: str) res[str, mach.base.fail.Fail]
```

the module object tree of project `id` under the build's work directory

| Parameter | |
| :--- | :--- |
| `a` | allocator for the returned path |
| `proj_out` | the project's work directory |
| `id` | the project id the tree is named for |
| **returns** | the path, owned by the caller in `a`, or the failure of building it |

### path_in_module_obj_tree

```mach
pub fun path_in_module_obj_tree(path: str, tree: str) bool
```

whether `path` is the directory `tree` or lies under it

| Parameter | |
| :--- | :--- |
| `path` | path to test, spelled as `tree` is |
| `tree` | directory path with no trailing separator |
| **returns** | true when `path` equals `tree` or continues it past a separator |

### step_out_reserved_error

```mach
pub fun step_out_reserved_error(a: *std.allocator.Allocator, what: str, name: str, path: str, tree: str, owner_desc: str) mach.base.fail.Fail
```

the refusal of a step output written into a directory the compiler owns

| Parameter | |
| :--- | :--- |
| `a` | allocator for the message |
| `what` | what declares the output, leading the message |
| `name` | the step that writes it |
| `path` | the output's expanded path |
| `tree` | the compiler-owned directory it lies in |
| `owner_desc` | what that directory is, in words |
| **returns** | the user refusal, of kind STEP_OUTPUT_RESERVED |

### select_target

```mach
pub fun select_target(p: *mach.compile.project.Project, t: *mach.compile.project.TargetEntry) err[mach.base.fail.Fail]
```

resolve the cell's target and open mirl's back end for it

| Parameter | |
| :--- | :--- |
| `p` | project whose target, back end and compiler name and version are set |
| `t` | the target entry to resolve, an executable's linkage narrowing it |
| **returns** | ok, or the failure of resolving the target, its linkage, opening the back end or interning a name |

