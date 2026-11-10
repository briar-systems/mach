# mach.lang.driver.closure

closure: how a build finds its dependency closure. every dependency is the directory its
key names under dep/, read through its manifest; nothing here fetches, pins or checks a
dependency against its source, which is the dependency manager's (package.mach). a stale
or missing dependency surfaces as a build error that `mach dep pull` fixes. the located
closure then scopes the build: a dependency's own closure, the link requirements each
artifact cascades, and the template variables a manifest reads

## fun root_dependency

```mach
pub fun root_dependency(root_m: *manifest_model.Manifest, name: intern.StrId) *manifest_model.DepDef;
```

## rec Check

```mach
pub rec Check;
```

## fun running_mach_range_floor

```mach
pub fun running_mach_range_floor(a: *A.Allocator) res[str, std_format.FormatError];
```

mach_range_floor for this compiler

## val DEP_ROOT_REFUSED

```mach
pub val DEP_ROOT_REFUSED: str = "dependency root 'dep' must be a physical directory, not a symlink or another kind of entry"
```

`dep` is the project's reserved dependency root: absent, or a physical directory. a
symlink there would resolve the closure from another tree, so every command that reads
or realizes the closure refuses it through this one check

## fun check_dep_root

```mach
pub fun check_dep_root(project_root: str) res[bool, fail.Fail];
```

whether the dependency root exists; err when it is anything but a physical directory

## def Unrealized

```mach
pub def Unrealized: u8
```

what locating makes of a dependency nothing is realized for under dep/

## val UNREALIZED_REFUSED

```mach
pub val UNREALIZED_REFUSED: Unrealized = 0
```

a build needs every dependency, so it is refused, naming `mach dep pull`

## val UNREALIZED_UNBUILT

```mach
pub val UNREALIZED_UNBUILT: Unrealized = 1
```

a dependency never realized was never built: it is left out with what it
would reach, and an edge to it names no project. only a caller asking what
was built, as clean does, locates this way; such a closure never configures a build

## fun locate

```mach
pub fun locate(s: *session.Session, alloc: *A.Allocator, m: *manifest_model.Manifest, project_root: str,
check: *Check, unrealized: Unrealized) res[project.Closure, fail.Fail];
```

locate a root manifest's dependency closure as a build reads it: every
dependency is the directory its key names under dep/, read through its
manifest. nothing is checked against git, a pin, a selector or the identity
rule; `mach dep pull` realizes what is missing and `mach dep verify` checks
what exists. a root with no dependency still has its compiler range checked

s: interns the names the manifests carry
alloc: owns the closure
m: the root manifest
project_root: the root project's directory
check: what each located edge is also held to, or nil for a build, which locates only
unrealized: what a dependency nothing is realized for is
ret: the closure; err naming a missing dependency, an unreadable manifest, a
              cycle or an unaccepted compiler range, or the first edge that does not hold
              to `check`

## fun adopt

```mach
pub fun adopt(p: *project.Project, c: *project.Closure) err[fail.Fail];
```

the cell `p` configures compiles against `c`: its `deps` borrow every entry

c: outlives the project

## fun scope_closure_to_dependency

```mach
pub fun scope_closure_to_dependency(p: *project.Project, owner: str) err[fail.Fail];
```

make a resolved closure the closure of one of its dependencies: the dependency's
entry becomes `p.config.owner` and `p.config.deps` keeps only what it reaches
through its own edges, in closure order. the entries stay the closure's

p: a project whose `p.config.deps` holds the root's resolved closure
owner: the dependency's project id
ret: ok; err when the closure holds no such dependency

## fun resolve_requirement_scopes

```mach
pub fun resolve_requirement_scopes(p: *project.Project, root: *manifest_model.Manifest, profile: str) err[fail.Fail];
```

the requirement scope of each project a configured cell compiles: the declaring
project's, holding the cell artifact's requirements, then one per closure
dependency, holding what its default library artifacts require

p: a configured project whose `art_reqs` and `deps` are resolved
root: the root manifest, whose out and profile the dependencies' outputs expand in
profile: the resolved profile name
ret: ok with `p.config.req_scopes` set; err from output expansion

## fun resolve_cascade_libs

```mach
pub fun resolve_cascade_libs(p: *project.Project, isa: intern.StrId, os: intern.StrId, abi: intern.StrId,
own_libs: *manifest_link.LinkRequirement, own_count: u32) err[fail.Fail];
```

## fun build_dep_root

```mach
pub fun build_dep_root(alloc: *A.Allocator, project_root: str, dir_dep: str, alias: str) res[str, fail.Fail];
```

## fun cell_tmpl_vars

```mach
pub fun cell_tmpl_vars(p: *project.Project) template.Values;
```

## fun owner_tmpl_vars

```mach
pub fun owner_tmpl_vars(p: *project.Project, owner: str) template.Values;
```

`cell_tmpl_vars` whose `{artifact.<id>.out}` resolves in the requirement scope
of the project `owner` names, empty when the cell binds no scope for it

## val DEP_DIR

```mach
pub val DEP_DIR: str = "dep"
```

