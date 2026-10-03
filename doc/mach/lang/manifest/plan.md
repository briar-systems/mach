# mach.lang.manifest.plan

## def LibKind

```mach
pub def LibKind: u8
```

the library shape of a build unit, decoded from `[artifact.<name>].kind`

## val LIBKIND_STATIC

```mach
pub val LIBKIND_STATIC: LibKind = 0
```

`kind = "static"`; also the value a `bin` unit carries, so check `is_lib` first

## val LIBKIND_SHARED

```mach
pub val LIBKIND_SHARED: LibKind = 1
```

`kind = "shared"`

## rec BuildUnit

```mach
pub rec BuildUnit;
```

one resolved build cell: a target, a profile, and an artifact with every path
expanded. produced by `resolve_build_unit`; `libs` is owned by the unit

target: the resolved target; points into the manifest, except for the host
               target synthesized when the manifest declares no targets
artifact: the resolved artifact, pointing into the manifest
target_name: `target.name`
profile_name: the resolved profile's name
entry: the artifact's `entry`
bin_path: the artifact's output: the expanded `[project].out` joined with the
               expanded artifact `out`
out_root: `<expanded project out>`, whose layout beneath it the build owns
obj_root: `<expanded project out>/obj`
ir_root: `<expanded project out>/ir`
asm_root: `<expanded project out>/asm`
test_root: `<expanded project out>/test/{name}`, with `{name}` left literal for
               the test runner to fill
cache_root: `<expanded project out>/.cache`, the compiler-only state
stage_root: `<expanded project out>/.stage`, the build step scratch space
opt: from the resolved profile
debug: from the resolved profile
simd: from the resolved profile
vectorize: from the resolved profile
float_reassoc: from the resolved profile
is_lib: the artifact's `is_lib`
kind: LIBKIND_SHARED for `kind = "shared"`, LIBKIND_STATIC otherwise, including for a `bin`
libs: the artifact's `link` entries that match the target, resolved in `link`
               order and sized exactly; a `link` name matching no `[link.*]` table is skipped
lib_count: length of `libs`

## rec Scope

```mach
pub rec Scope;
```

the manifest that declares a build cell, and the root project the cell is built
for. a root cell has `owner == root`. a dependency's cell is one of the
requirements its export library carries: it resolves its artifact, targets and
links in `owner`, its profile in `root`, expands the root's `[project].out`, and
reads `{project.out}` in its artifact outputs as `dep/<owner id>` of that out,
so identically named artifacts of two dependencies never share a path

root: the manifest of the project being built
owner: the manifest declaring the cell's artifact; `root` for the root's own

## val DEPENDENCY_ARTIFACT_DIR

```mach
pub val DEPENDENCY_ARTIFACT_DIR: str = "dep"
```

the directory under the expanded root `[project].out` that holds a dependency's
artifact outputs, one subdirectory per dependency id

## val CACHE_DIR

```mach
pub val CACHE_DIR: str = ".cache"
```

the directory under the expanded `[project].out` that holds compiler-only state:
nothing but the compiler reads or writes it, and a step output may not name it

## val STEP_STAMP_DIR

```mach
pub val STEP_STAMP_DIR: str = ".cache/steps"
```

the directory under `CACHE_DIR` that holds one fingerprint stamp per build step

## val STAGE_DIR

```mach
pub val STAGE_DIR: str = ".stage"
```

the directory under the expanded `[project].out` that holds build step scratch
space, one subdirectory per step, reset before the step runs

## fun root_scope

```mach
pub fun root_scope(m: *Manifest) Scope;
```

## fun dependency_scope

```mach
pub fun dependency_scope(root: *Manifest, owner: *Manifest) Scope;
```

## fun scope_is_dependency

```mach
pub fun scope_is_dependency(s: *Scope) bool;
```

## fun local_path_demanded_by_step

```mach
pub fun local_path_demanded_by_step(alloc: *A.Allocator, itn: *intern.Interner, m: *Manifest,
expanded_path: str, proj_out: str, v: *template.Values) res[bool, fail.Fail];
```

whether some build step produces a path; `step_producing_out` as a bool

## fun required_artifact_uses_target

```mach
pub fun required_artifact_uses_target(itn: *intern.Interner, req: *ArtifactDef,
consumer_target: intern.StrId, t: *TargetDef) bool;
```

whether a required artifact is built for a target when its consumer builds for
`consumer_target`: only that target when `req` supports it, otherwise every
target `req` supports

itn: resolves the names
req: the required artifact
consumer_target: the consumer's target name, or STR_NIL for a consumer outside
                 `req`'s manifest, which admits every target `req` supports
t: the target asked about
ret: true when `req` is built for `t` on behalf of that consumer

## fun export_requires

```mach
pub fun export_requires(itn: *intern.Interner, m: *Manifest, ra: *ArtifactDef) bool;
```

whether the export library selects `ra` in its `need`: the requirements a
dependency's export library carries to every consumer

itn: resolves the names
m: the manifest
ra: the candidate requirement
ret: true when the artifact marked `export = true` needs it

## fun resolve_artifact_reqs

```mach
pub fun resolve_artifact_reqs(alloc: *A.Allocator, itn: *intern.Interner, reg: *lang_target.TargetRegistry, s: *Scope,
consumer: *ArtifactDef, consumer_target: intern.StrId, profile: str,
whole_project: bool,
out_items: **template.Requirement, out_count: *u32) err[fail.Fail];
```

list the artifacts a consumer requires, each with its output path when that
path is unique, for `{artifact.<id>.out}` expansion

alloc: owns the returned array and each non-empty `out` string
itn: resolves names
s: the scope; the requirements are declared by `s.owner`
consumer: the requiring artifact; nil with `whole_project` false yields nothing
consumer_target: the consumer's target name
profile: the profile name the outputs expand with
whole_project: select every artifact some artifact needs instead of `consumer`'s
out_items: receives the array, or nil when nothing is required
out_count: receives its length
ret: ok; err from the output path expansion, with the array freed

## fun resolve_export_reqs

```mach
pub fun resolve_export_reqs(alloc: *A.Allocator, itn: *intern.Interner, reg: *lang_target.TargetRegistry,
s: *Scope, profile: str, out_items: **template.Requirement, out_count: *u32) err[fail.Fail];
```

list the artifacts a dependency's export library requires, the scope
`{artifact.<id>.out}` resolves in for that dependency's modules. each requirement
builds for every target it names in the dependency's manifest

alloc: owns the returned array and each non-empty `out` string
itn: resolves names
s: a dependency scope
profile: the root's profile name the outputs expand with
out_items: receives the array, or nil when nothing is required
out_count: receives its length
ret: ok; err from the output path expansion, with the array freed

## fun plan_steps

```mach
pub fun plan_steps(alloc: *A.Allocator, itn: *intern.Interner, m: *Manifest,
art_names: *intern.StrId, art_count: u32, t: *TargetDef,
proj_out: str, v: *template.Values,
out_order: **u32, out_count: *u32) err[fail.Fail];
```

order the build steps the named artifacts demand: a step producing a local
link path that matches the target, every step a `need` names (globs match
step names), and transitively each step's own `need`, dependencies first

alloc: owns the returned order
itn: resolves names
m: the manifest
art_names: the artifacts to plan for; unknown names are skipped
art_count: length of `art_names`
t: the target
proj_out: the expanded `[project].out`
v: the template values
out_order: receives the step indices in run order, sized exactly; nil when none
out_count: receives the length
ret: ok; err on a step `need` cycle or a local link path that fails to expand

## fun plan_export_steps

```mach
pub fun plan_export_steps(alloc: *A.Allocator, itn: *intern.Interner, m: *Manifest,
t: *TargetDef, proj_out: str, v: *template.Values,
out_order: **u32, out_count: *u32) err[fail.Fail];
```

order the build steps that produce an exported local link path matching the
target and the steps the export library's `need` names, with their
transitive step `need`s; the steps a consumer of this manifest must run

alloc: owns the returned order
itn: resolves names
m: the manifest
t: the target
proj_out: the expanded `[project].out`
v: the template values
out_order: receives the step indices in run order, sized exactly; nil when none
out_count: receives the length
ret: ok; the `plan_steps` errors

## fun resolve_build_unit

```mach
pub fun resolve_build_unit(alloc: *A.Allocator, itn: *intern.Interner, reg: *lang_target.TargetRegistry, m: *Manifest, c: *Cell) res[BuildUnit, fail.Fail];
```

resolve one cell into a `BuildUnit`. the cell names its artifact; with no
target named, an artifact that supports exactly one declared target, or exactly
one host-matching target, pins it, except that a sole hosted target the host
cannot run is refused as `native` refuses it; otherwise `native` resolution
applies. the profile comes from `resolve_profile`, and every path is expanded
through the template engine

alloc: owns `libs` and temporary strings
itn: interns the expanded paths
m: the manifest
c: the cell, as `resolve_cells` resolves it or as `cell_of` names it
ret: the unit; err from target or profile resolution, for an artifact the
       manifest does not declare, when the artifact does not support the target
       (naming the artifact's targets when none was selected), or from path expansion

## fun resolve_scoped_build_unit

```mach
pub fun resolve_scoped_build_unit(alloc: *A.Allocator, itn: *intern.Interner, reg: *lang_target.TargetRegistry, s: *Scope, c: *Cell) res[BuildUnit, fail.Fail];
```

`resolve_build_unit` for a cell of `s`: target, artifact and links resolve in
`s.owner`, the profile and `[project].out` in `s.root`, and `{project.out}` in
the artifact output names the scope's artifact root

alloc: owns `libs` and temporary strings
itn: interns the expanded paths
s: the scope
c: the cell, naming a target and an artifact of `s.owner` and a profile of `s.root`
ret: as `resolve_build_unit`

## fun check_collisions

```mach
pub fun check_collisions(alloc: *A.Allocator, itn: *intern.Interner, reg: *lang_target.TargetRegistry, m: *Manifest,
v: *template.Values) err[fail.Fail];
```

reject two artifacts whose `out` expands to the same path under one set of
template values

alloc: owns temporary strings
itn: resolves names
m: the manifest; fewer than two artifacts always pass
v: the template values
ret: ok; err "mach.toml: artifacts 'a' and 'b' collide on output path '<p>'",
       or an expansion error

