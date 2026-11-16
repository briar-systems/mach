[mach](../README.md) / [compile](README.md) / **closure**

# mach.compile.closure

closure: how a build finds its dependency closure. every dependency is the directory its
key names under dep/, read through its manifest; nothing here fetches, pins or checks a
dependency against its source, which is the dependency manager's (package.mach). a stale
or missing dependency surfaces as a build error that `mach dep pull` fixes. the located
closure then scopes the build: a dependency's own closure, the link requirements each
artifact cascades, and the template variables a manifest reads

Source: [`src/compile/closure.mach`](../../../src/compile/closure.mach)

Uses types from [`mach.base.fail`](../base/fail.md), [`mach.base.intern`](../base/intern.md), [`mach.compile.project`](project.md), [`mach.manifest.link`](../manifest/link.md), [`mach.manifest.model`](../manifest/model.md), [`mach.manifest.template`](../manifest/template.md), [`mach.session`](../session.md).

| Type | | |
| :--- | :--- | :--- |
| [`Check`](#check) | `rec` |  |
| [`Unrealized`](#unrealized) | `def` | what locating makes of a dependency nothing is realized for under dep/ |

| On `Check` | |
| :--- | :--- |
| [`locate`](#locate) | locate a root manifest's dependency closure as a build reads it: every dependency is the directory its key names under dep/, read through its manifest |

| On `Unrealized` | |
| :--- | :--- |
| [`UNREALIZED_REFUSED`](#unrealized) | a build needs every dependency, so it is refused, naming `mach dep pull` |
| [`UNREALIZED_UNBUILT`](#unrealized) | a dependency never realized was never built: it is left out with what it would reach, and an edge to it names no project |

| Functions | |
| :--- | :--- |
| [`root_dependency`](#root_dependency) | |
| [`running_mach_range_floor`](#running_mach_range_floor) | mach_range_floor for this compiler |
| [`check_dep_root`](#check_dep_root) | whether the dependency root exists; err when it is anything but a physical directory |
| [`adopt`](#adopt) | the cell `p` configures compiles against `c`: its `deps` borrow every entry |
| [`scope_closure_to_dependency`](#scope_closure_to_dependency) | make a resolved closure the closure of one of its dependencies: the dependency's entry becomes `p.config.owner` and `p.config.deps` keeps only what it reaches through its own edges, in closure order |
| [`resolve_requirement_scopes`](#resolve_requirement_scopes) | the requirement scope of each project a configured cell compiles: the declaring project's, holding the cell artifact's requirements, then one per closure dependency, holding what its default library artifacts require |
| [`resolve_cascade_libs`](#resolve_cascade_libs), [`build_dep_root`](#build_dep_root), [`cell_tmpl_vars`](#cell_tmpl_vars) | |
| [`owner_tmpl_vars`](#owner_tmpl_vars) | `cell_tmpl_vars` whose `{artifact.<id>.out}` resolves in the requirement scope of the project `owner` names, empty when the cell binds no scope for it |

| Constants | |
| :--- | :--- |
| [`DEP_ROOT_REFUSED`](#constants) | `dep` is the project's reserved dependency root: absent, or a physical directory |
| [`DEP_DIR`](#constants) | |

---

## Check

```mach
pub rec Check {
    ctx:      ptr;
    located:  LocatedFn;
    release:  ReleaseFn;
    selector: SelectorFn;
}
```

### locate

```mach
pub fun locate(s: *mach.session.Session, alloc: *std.allocator.Allocator, m: *mach.manifest.model.Manifest, project_root: str,
    check: *Check, unrealized: Unrealized) res[mach.compile.project.Closure, mach.base.fail.Fail]
```

locate a root manifest's dependency closure as a build reads it: every
dependency is the directory its key names under dep/, read through its
manifest. nothing is checked against git, a pin, a selector or the identity
rule; `mach dep pull` realizes what is missing and `mach dep verify` checks
what exists. a root with no dependency still has its compiler range checked

| Parameter | |
| :--- | :--- |
| `s` | interns the names the manifests carry |
| `alloc` | owns the closure |
| `m` | the root manifest |
| `project_root` | the root project's directory |
| `check` | what each located edge is also held to, or nil for a build, which locates only |
| `unrealized` | what a dependency nothing is realized for is |
| **returns** | the closure; err naming a missing dependency, an unreadable manifest, a cycle or an unaccepted compiler range, or the first edge that does not hold to `check` |

## Unrealized

```mach
pub def Unrealized: u8
```

what locating makes of a dependency nothing is realized for under dep/

| Name | Value | |
| :--- | :--- | :--- |
| `UNREALIZED_REFUSED` | `0` | a build needs every dependency, so it is refused, naming `mach dep pull` |
| `UNREALIZED_UNBUILT` | `1` | a dependency never realized was never built: it is left out with what it would reach, and an edge to it names no project |

## Functions

### root_dependency

```mach
pub fun root_dependency(root_m: *mach.manifest.model.Manifest, name: mach.base.intern.StrId) *mach.manifest.model.DepDef
```

### running_mach_range_floor

```mach
pub fun running_mach_range_floor(a: *std.allocator.Allocator) res[str, std.format.FormatError]
```

mach_range_floor for this compiler

### check_dep_root

```mach
pub fun check_dep_root(a: *std.allocator.Allocator, project_root: str) res[bool, mach.base.fail.Fail]
```

whether the dependency root exists; err when it is anything but a physical directory. a
refusal's text is allocated in `a`

### adopt

```mach
pub fun adopt(p: *mach.compile.project.Project, c: *mach.compile.project.Closure) err[mach.base.fail.Fail]
```

the cell `p` configures compiles against `c`: its `deps` borrow every entry

| Parameter | |
| :--- | :--- |
| `c` | outlives the project |

### scope_closure_to_dependency

```mach
pub fun scope_closure_to_dependency(p: *mach.compile.project.Project, owner: str) err[mach.base.fail.Fail]
```

make a resolved closure the closure of one of its dependencies: the dependency's
entry becomes `p.config.owner` and `p.config.deps` keeps only what it reaches
through its own edges, in closure order. the entries stay the closure's

| Parameter | |
| :--- | :--- |
| `p` | a project whose `p.config.deps` holds the root's resolved closure |
| `owner` | the dependency's project id |
| **returns** | ok; err when the closure holds no such dependency |

### resolve_requirement_scopes

```mach
pub fun resolve_requirement_scopes(p: *mach.compile.project.Project, root: *mach.manifest.model.Manifest, profile: str) err[mach.base.fail.Fail]
```

the requirement scope of each project a configured cell compiles: the declaring
project's, holding the cell artifact's requirements, then one per closure
dependency, holding what its default library artifacts require

| Parameter | |
| :--- | :--- |
| `p` | a configured project whose `art_reqs` and `deps` are resolved |
| `root` | the root manifest, whose out and profile the dependencies' outputs expand in |
| `profile` | the resolved profile name |
| **returns** | ok with `p.config.req_scopes` set; err from output expansion |

### resolve_cascade_libs

```mach
pub fun resolve_cascade_libs(p: *mach.compile.project.Project, isa: mach.base.intern.StrId, os: mach.base.intern.StrId, abi: mach.base.intern.StrId,
    own_libs: *mach.manifest.link.LinkRequirement, own_count: u32) err[mach.base.fail.Fail]
```

### build_dep_root

```mach
pub fun build_dep_root(alloc: *std.allocator.Allocator, project_root: str, dir_dep: str, alias: str) res[str, mach.base.fail.Fail]
```

### cell_tmpl_vars

```mach
pub fun cell_tmpl_vars(p: *mach.compile.project.Project) mach.manifest.template.Values
```

### owner_tmpl_vars

```mach
pub fun owner_tmpl_vars(p: *mach.compile.project.Project, owner: str) mach.manifest.template.Values
```

`cell_tmpl_vars` whose `{artifact.<id>.out}` resolves in the requirement scope
of the project `owner` names, empty when the cell binds no scope for it

## Constants

| Name | Type | Value | |
| :--- | :--- | :--- | :--- |
| `DEP_ROOT_REFUSED` | `str` | `"dependency root 'dep' must be a physical directory, not a symlink or another kind of entry"` | `dep` is the project's reserved dependency root: absent, or a physical directory |
| `DEP_DIR` | `str` | `"dep"` |  |

