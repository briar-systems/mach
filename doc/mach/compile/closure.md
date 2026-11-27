[mach](../README.md) / [compile](README.md) / **closure**

# mach.compile.closure

closure: how a build finds its dependency closure. every dependency is the directory its
key names under dep/, read through its manifest and refused unless its project id is that
key; nothing here fetches, pins or checks a dependency against its source, which is the
dependency manager's (package.mach). a stale
or missing dependency surfaces as a build error that `mach dep pull` fixes. the located
closure then scopes the build: a dependency's own closure, the link requirements each
artifact cascades, and the template variables a manifest reads

Source: [`src/compile/closure.mach`](../../../src/compile/closure.mach)

Uses types from [`mach.base.fail`](../base/fail.md), [`mach.base.intern`](../base/intern.md), [`mach.base.source`](../base/source.md), [`mach.compile.project`](project.md), [`mach.manifest.link`](../manifest/link.md), [`mach.manifest.load`](../manifest/load.md), [`mach.manifest.model`](../manifest/model.md), [`mach.manifest.template`](../manifest/template.md), [`mach.session`](../session.md).

| Type | | |
| :--- | :--- | :--- |
| [`Check`](#check) | `rec` | the checks a caller holds each located edge to, nil for a build |
| [`Unmet`](#unmet) | `rec` | a compiler range the running compiler is outside of, stated by one manifest of the closure |
| [`Unrealized`](#unrealized) | `def` | what locating makes of a dependency nothing is realized for under dep/ |

| On `Check` | |
| :--- | :--- |
| [`locate`](#locate) | locate a root manifest's dependency closure as a build reads it: every dependency is the directory its key names under dep/, read through its manifest |

| On `Unmet` | |
| :--- | :--- |
| [`unmet_names`](#unmet_names) | whether `unmet` already names the dependency `key` |
| [`admitted`](#admitted) | what a dependency's manifest is held to as read, before its schema: the identity rule, then the compiler range |
| [`unaccepted`](#unaccepted) | the refusal of a closure that does not accept the running compiler, naming every unmet range with who states it and pointing at the first |

| On `Unrealized` | |
| :--- | :--- |
| [`UNREALIZED_REFUSED`](#unrealized) | a build needs every dependency, so it is refused, naming `mach dep pull` |
| [`UNREALIZED_UNBUILT`](#unrealized) | a dependency never realized was never built: it is left out with what it would reach, and an edge to it names no project |

| Functions | |
| :--- | :--- |
| [`root_dependency`](#root_dependency) | the root manifest's own declaration of a dependency |
| [`compiler_admitted`](#compiler_admitted) | whether the running compiler satisfies `text`; a range that no longer parses counts as unmet |
| [`compiler_outside`](#compiler_outside) | whether `text` is a range the running compiler is outside of; a range that does not parse is not one, and is the schema's to refuse |
| [`running_mach_range_floor`](#running_mach_range_floor) | mach_range_floor for this compiler |
| [`check_dep_root`](#check_dep_root) | whether the dependency root exists; err when it is anything but a physical directory |
| [`adopt`](#adopt) | the cell `p` configures compiles against `c`: its `deps` borrow every entry |
| [`scope_closure_to_dependency`](#scope_closure_to_dependency) | make a resolved closure the closure of one of its dependencies: the dependency's entry becomes `p.config.owner` and `p.config.deps` keeps only what it reaches through its own edges, in closure order |
| [`resolve_requirement_scopes`](#resolve_requirement_scopes) | the requirement scope of each project a configured cell compiles: the declaring project's, holding the cell artifact's requirements, then one per closure dependency, holding what its export library artifact requires |
| [`resolve_cascade_libs`](#resolve_cascade_libs) | collect the exported `[link.*]` entries of the cell's dependencies that match its target each dependency is walked after the dependencies it reaches |
| [`check_realized`](#check_realized) | refuse a dependency checkout a build cannot read: nothing realized at `dep_dir` (absent, or the empty directory a pin leaves on a fresh clone), or a symlink or other entry in its place |
| [`diag_dep_manifest_error`](#diag_dep_manifest_error) | the message for a dependency whose manifest cannot be read |
| [`dep_manifest_refused`](#dep_manifest_refused) | `read_err`, a failure to read or parse the manifest of the dependency `alias`, worded as one |
| [`check_dep_identity`](#check_dep_identity) | the manifest key, the directory under dep/ and the project id are one name |
| [`build_dep_root`](#build_dep_root) | the directory a dependency is realized in, `<project_root>/<dir_dep>/<alias>` |
| [`cell_tmpl_vars`](#cell_tmpl_vars) | the template values a manifest of the cell expands in: its target, profile and artifact requirements |
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

the checks a caller holds each located edge to, nil for a build

| Field | Type | |
| :--- | :--- | :--- |
| `ctx` | `ptr` | the caller's state, passed to each check |
| `located` | `LocatedFn` | checks one located dependency, as LocatedFn states |
| `release` | `ReleaseFn` | holds a release-selecting edge to the release rule, as ReleaseFn states |
| `selector` | `SelectorFn` | checks a requirer's declaration against what it reaches, as SelectorFn states |

### locate

```mach
pub fun locate(s: *mach.session.Session, alloc: *std.allocator.Allocator, m: *mach.manifest.model.Manifest, project_root: str,
    check: *Check, unrealized: Unrealized) res[mach.compile.project.Closure, mach.base.fail.Fail]
```

locate a root manifest's dependency closure as a build reads it: every
dependency is the directory its key names under dep/, read through its
manifest. nothing is checked against git, a pin or a selector; `mach dep pull`
realizes what is missing and `mach dep verify` checks what exists. a root with no
dependency still has its compiler range checked

| Parameter | |
| :--- | :--- |
| `s` | interns the names the manifests carry |
| `alloc` | owns the closure |
| `m` | the root manifest |
| `project_root` | the root project's directory |
| `check` | what each located edge is also held to, or nil for a build, which locates only |
| `unrealized` | what a dependency nothing is realized for is |
| **returns** | the closure; err naming a missing dependency, an unreadable manifest, a cycle or an unaccepted compiler range, or the first edge that does not hold to `check` |

## Unmet

```mach
pub rec Unmet {
    key:   mach.base.intern.StrId;
    who:   mach.base.intern.StrId;
    range: mach.base.intern.StrId;
    at:    mach.base.source.Location;
}
```

a compiler range the running compiler is outside of, stated by one manifest of the closure

| Field | Type | |
| :--- | :--- | :--- |
| `key` | `mach.base.intern.StrId` | the dependency's key, nil for the root |
| `who` | `mach.base.intern.StrId` | who states it, as the refusal names it: the chain that reaches a dependency |
| `range` | `mach.base.intern.StrId` | the range as written |
| `at` | `mach.base.source.Location` | where the range is written |

### unmet_names

```mach
pub fun unmet_names(unmet: *Vector[Unmet], key: mach.base.intern.StrId) bool
```

whether `unmet` already names the dependency `key`

### admitted

```mach
pub fun admitted(s: *mach.session.Session, key: str, chain: str, doc: *mach.manifest.load.Doc) res[opt[Unmet], mach.base.fail.Fail]
```

what a dependency's manifest is held to as read, before its schema: the identity rule, then
the compiler range. a manifest written for another compiler may lack or misspell keys this
one requires, so a range the running compiler is outside of stops it before the schema
reads it, and the closure is refused once with every such range. an id or a range it does
not state, or a range that does not parse, is the schema's to refuse

| Parameter | |
| :--- | :--- |
| `s` | the session, which interns the unmet range |
| `key` | the dependency's key |
| `chain` | the chain that reaches it |
| `doc` | its manifest, read |
| **returns** | the range it states that the running compiler is outside of, none when the schema is to read it; err when it realizes another project |

### unaccepted

```mach
pub fun unaccepted(s: *mach.session.Session, alloc: *std.allocator.Allocator, unmet: *Vector[Unmet]) mach.base.fail.Fail
```

the refusal of a closure that does not accept the running compiler, naming every unmet range
with who states it and pointing at the first

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

the root manifest's own declaration of a dependency

| Parameter | |
| :--- | :--- |
| `root_m` | the root manifest |
| `name` | the dependency's key |
| **returns** | the declaration, borrowed from `root_m`, or nil when the root declares no such dependency |

### compiler_admitted

```mach
pub fun compiler_admitted(text: str) bool
```

whether the running compiler satisfies `text`; a range that no longer parses counts as unmet

### compiler_outside

```mach
pub fun compiler_outside(text: str) bool
```

whether `text` is a range the running compiler is outside of; a range that does not parse
is not one, and is the schema's to refuse

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
dependency, holding what its export library artifact requires

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

collect the exported `[link.*]` entries of the cell's dependencies that match its target
each dependency is walked after the dependencies it reaches

| Parameter | |
| :--- | :--- |
| `p` | configured project whose `cascade_libs` are replaced |
| `isa` | the target's architecture |
| `os` | the target's system |
| `abi` | the target's abi |
| `own_libs` | the cell's own link requirements, into which an entry naming the same library merges |
| `own_count` | length of `own_libs` |
| **returns** | ok, or a failure on a dependency cycle, an entry that cannot be expanded, a library stated with two versions, a framework stated in two directories, or an allocation or merge of link claims |

### check_realized

```mach
pub fun check_realized(s: *mach.session.Session, alias: str, dep_dir: str, root: str) err[mach.base.fail.Fail]
```

refuse a dependency checkout a build cannot read: nothing realized at `dep_dir` (absent, or the
empty directory a pin leaves on a fresh clone), or a symlink or other entry in its place

### diag_dep_manifest_error

```mach
pub fun diag_dep_manifest_error(s: *mach.session.Session, alias: str, read_err: str) res[str, mach.base.fail.Fail]
```

the message for a dependency whose manifest cannot be read

| Parameter | |
| :--- | :--- |
| `s` | session whose build allocator holds the message |
| `alias` | the dependency's key |
| `read_err` | why the read failed |
| **returns** | the message, or the failure of formatting it |

### dep_manifest_refused

```mach
pub fun dep_manifest_refused(s: *mach.session.Session, alias: str, read_err: mach.base.fail.Fail) mach.base.fail.Fail
```

`read_err`, a failure to read or parse the manifest of the dependency `alias`, worded as one

### check_dep_identity

```mach
pub fun check_dep_identity(s: *mach.session.Session, key: str, declared: str) err[mach.base.fail.Fail]
```

the manifest key, the directory under dep/ and the project id are one name

### build_dep_root

```mach
pub fun build_dep_root(alloc: *std.allocator.Allocator, project_root: str, dir_dep: str, alias: str) res[str, mach.base.fail.Fail]
```

the directory a dependency is realized in, `<project_root>/<dir_dep>/<alias>`

| Parameter | |
| :--- | :--- |
| `alloc` | allocator for the path |
| `project_root` | the project directory |
| `dir_dep` | the dependency root under it |
| `alias` | the dependency's key |
| **returns** | the path, owned by the caller in `alloc`, or the allocation failure |

### cell_tmpl_vars

```mach
pub fun cell_tmpl_vars(p: *mach.compile.project.Project) mach.manifest.template.Values
```

the template values a manifest of the cell expands in: its target, profile and artifact requirements

| Parameter | |
| :--- | :--- |
| `p` | configured project whose target entry, profile and requirements are read |
| **returns** | the values, borrowing from the project |

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

