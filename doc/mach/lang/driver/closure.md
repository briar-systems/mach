# mach.lang.driver.closure

closure: how a build finds its dependency closure. every dependency is the directory its
key names under dep/, read through its manifest; nothing here fetches, pins or checks a
dependency against its source, which is the dependency manager's (package.mach). a stale
or missing dependency surfaces as a build error that `mach dep pull` fixes. the located
closure then scopes the build: a dependency's own closure, the link requirements each
artifact cascades, and the template variables a manifest reads

## fun root_dependency

```mach
pub fun root_dependency(root_m: *manifest.Manifest, name: intern.StrId) *manifest.DepDef;
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

## val CLOSURE_MAX

```mach
pub val CLOSURE_MAX: usize = 128
```

## fun locate

```mach
pub fun locate(p: *project.Project, m: *manifest.Manifest, project_root: str, check: *Check) err[fail.Fail];
```

locate a root manifest's dependency closure into `p.config.deps`, as a build reads it:
every dependency is the directory its key names under dep/, read through its manifest.
nothing is checked against git, a pin, a selector or the identity rule; `mach dep pull`
realizes what is missing and `mach dep verify` checks what exists

p: the project receiving the closure
m: the root manifest
project_root: the root project's directory
check: what each located edge is also held to, or nil for a build, which locates only
ret: err naming a missing dependency, an unreadable manifest, a cycle or an
              unaccepted compiler range, or the first edge that does not hold to `check`

## fun scope_closure_to_dependency

```mach
pub fun scope_closure_to_dependency(p: *project.Project, owner: str) err[fail.Fail];
```

make a resolved closure the closure of one of its dependencies: the dependency's
entry moves to `p.config.owner` and `p.config.deps` keeps only what it reaches
through its own edges, in closure order

p: a project whose `p.config.deps` holds the root's resolved closure
owner: the dependency's project id
ret: ok; err when the closure holds no such dependency

## fun resolve_requirement_scopes

```mach
pub fun resolve_requirement_scopes(p: *project.Project, root: *manifest.Manifest, profile: str) err[fail.Fail];
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
pub fun resolve_cascade_libs(p: *project.Project, isa: str, os: str, abi: str,
own_libs: *manifest.LinkRequirement, own_count: u32) err[fail.Fail];
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

