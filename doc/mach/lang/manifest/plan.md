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

## rec Selection

```mach
pub rec Selection;
```

what the caller asked to build: a target, a profile, and optionally an
artifact. empty strings mean "not selected"

target: a declared target name, or "" to resolve `native` (or the artifact's pinned target)
profile: a declared profile name, or "" for the default profile
artifact: an artifact name, read only when `has_artifact`
want_lib: with `has_artifact`, whether the named artifact must be a library
              (true) or a `bin` (false); a name with the wrong shape does not match
has_artifact: whether `artifact` and `want_lib` are set

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
obj_root: `<expanded project out>/obj`
ir_root: `<expanded project out>/ir`
asm_root: `<expanded project out>/asm`
test_root: `<expanded project out>/test/{name}`, with `{name}` left literal for
               the test runner to fill
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
warned: `native` matched no declared target for the host and a declared target
               was chosen anyway; the driver prints the warning

## rec Scope

```mach
pub rec Scope;
```

the manifest that declares a build cell, and the root project the cell is built
for. a root cell has `owner == root`. a dependency's cell is one of the
requirements its default library artifacts carry: it resolves its artifact,
targets and links in `owner`, its profile in `root`, expands the root's
`[project].out`, and homes its artifact outputs under `dep/<owner id>` of that
out, so identically named artifacts of two dependencies never share a path

root: the manifest of the project being built
owner: the manifest declaring the cell's artifact; `root` for the root's own

## val DEPENDENCY_ARTIFACT_DIR

```mach
pub val DEPENDENCY_ARTIFACT_DIR: str = "dep"
```

the directory under the expanded root `[project].out` that holds a dependency's
artifact outputs, one subdirectory per dependency id

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

## rec ResolvedTarget

```mach
pub rec ResolvedTarget;
```

the outcome of target resolution for a selection

target: the chosen target; points into the manifest unless synthesized for a
        manifest with no `[target.*]`, in which case it is allocated and never freed by `dnit`
warned: `native` matched no host target and a declared target was chosen anyway
target borrows its manifest-owned declaration until manifest destruction

## rec ResolvedProfile

```mach
pub rec ResolvedProfile;
```

the profile a selection resolves to, copied out of its `ProfileDef`

name: the profile's name
opt: as `ProfileDef`
debug: as `ProfileDef`
simd: as `ProfileDef`
vectorize: as `ProfileDef`
float_reassoc: as `ProfileDef`

## fun resolve_profile

```mach
pub fun resolve_profile(alloc: *A.Allocator, itn: *intern.Interner, m: *Manifest, pick: str) res[ResolvedProfile, outcome.Fail];
```

choose the profile a selection names. a named profile must be declared, except
that a dependency manifest with no profiles at all answers "debug" and
"release" with the synthesized ones. an empty name takes the sole declared
profile, else the one with `default = true`, else the synthesized debug
profile of a dependency manifest. several declared profiles with none marked
default are refused: no profile is ever selected by table order

alloc: owns error text
itn: interns the name
m: the manifest
pick: the profile name, or "" for the default
ret: the profile; err "mach.toml: no profile named '<pick>'" or, with an empty
       name, an error when more than one profile is marked default

## fun resolve_artifact

```mach
pub fun resolve_artifact(itn: *intern.Interner, m: *Manifest, pick: Selection) *ArtifactDef;
```

the artifact a selection names: with `has_artifact`, the one whose name and
`is_lib` both match `pick`; otherwise the sole declared artifact

itn: interns the name
m: the manifest
pick: the selection
ret: the artifact, or nil when none is declared, the name has no match of
      the requested shape, or several artifacts are declared and none was named

## fun local_path_demanded_by_step

```mach
pub fun local_path_demanded_by_step(alloc: *A.Allocator, itn: *intern.Interner, m: *Manifest,
expanded_path: str, proj_out: str, v: *TmplVars) bool;
```

whether some build step produces a path; `step_producing_out` as a bool

alloc: owns temporary expansions
itn: resolves step strings
m: the manifest
expanded_path: the path to match, already expanded
proj_out: the expanded `[project].out`
v: the template values
ret: true when a step lists the path in `out`

## fun required_artifact_uses_target

```mach
pub fun required_artifact_uses_target(itn: *intern.Interner, req: *ArtifactDef,
consumer_target: intern.StrId, t: *TargetDef) bool;
```

whether a required artifact is built for a target when its consumer builds for
`consumer_target`: only that target when `req` supports it, otherwise every
target `req` supports

itn: interns "*"
req: the required artifact
consumer_target: the consumer's target name, or STR_NIL for a consumer outside
                 `req`'s manifest, which admits every target `req` supports
t: the target asked about
ret: true when `req` is built for `t` on behalf of that consumer

## fun artifact_required_by_any

```mach
pub fun artifact_required_by_any(itn: *intern.Interner, m: *Manifest, ra: *ArtifactDef) bool;
```

whether any artifact's `need` selects `ra`

itn: resolves the names
m: the manifest
ra: the candidate requirement
ret: true when some other artifact needs it

## fun default_library_requires

```mach
pub fun default_library_requires(itn: *intern.Interner, m: *Manifest, ra: *ArtifactDef) bool;
```

whether a library artifact marked `default = true` selects `ra` in its `need`:
the requirements a dependency's default library carries to every consumer

itn: resolves the names
m: the manifest
ra: the candidate requirement
ret: true when a default library artifact needs it

## fun resolve_artifact_reqs

```mach
pub fun resolve_artifact_reqs(alloc: *A.Allocator, itn: *intern.Interner, reg: *tgt.TargetRegistry, s: *Scope,
consumer: *ArtifactDef, consumer_target: intern.StrId, profile: str,
whole_project: bool,
out_items: **ArtifactReq, out_count: *u32) err[outcome.Fail];
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

## fun resolve_default_library_reqs

```mach
pub fun resolve_default_library_reqs(alloc: *A.Allocator, itn: *intern.Interner, reg: *tgt.TargetRegistry,
s: *Scope, profile: str, out_items: **ArtifactReq, out_count: *u32) err[outcome.Fail];
```

list the artifacts a dependency's default library artifacts require, the scope
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
proj_out: str, v: *TmplVars,
out_order: **u32, out_count: *u32) err[outcome.Fail];
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
ret: ok; err on a step `need` cycle, a step `need` naming an unknown step, or
           a local link path that fails to expand

## fun plan_export_steps

```mach
pub fun plan_export_steps(alloc: *A.Allocator, itn: *intern.Interner, m: *Manifest,
t: *TargetDef, proj_out: str, v: *TmplVars,
out_order: **u32, out_count: *u32) err[outcome.Fail];
```

order the build steps that produce an exported local link path matching the
target and the steps a default library artifact's `need` names, with their
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

## fun cell_supported

```mach
pub fun cell_supported(alloc: *A.Allocator, itn: *intern.Interner, m: *Manifest, pick: Selection) res[bool, outcome.Fail];
```

whether the selection's artifact builds for the selection's target. the target
is resolved as `native` would be, never pinned by the artifact

alloc: owns error text
itn: resolves names
m: the manifest
pick: the selection
ret: the support bit; err from target resolution, or when `resolve_artifact`
       finds nothing

## val AMBIGUOUS_TARGET_MSG

```mach
pub val AMBIGUOUS_TARGET_MSG:   str = "mach.toml: several targets are declared, none matches the host and none is marked `default = true`
```

a selection several candidates could satisfy is refused where a command
must pick one; table order carries no meaning (#3222, #3226)

## val AMBIGUOUS_PROFILE_MSG

```mach
pub val AMBIGUOUS_PROFILE_MSG:  str = "mach.toml: several profiles are declared and none is marked `default = true`
```

## val AMBIGUOUS_ARTIFACT_MSG

```mach
pub val AMBIGUOUS_ARTIFACT_MSG: str = "mach.toml: several artifacts support the selected target and none is marked `default = true`
```

## fun default_selection_includes

```mach
pub fun default_selection_includes(itn: *intern.Interner, m: *Manifest, a: *ArtifactDef,
target: intern.StrId, executables_only: bool) bool;
```

the default selection: what a command takes for a target when no `--bin`/`--lib`
names an artifact. of the artifacts the target builds, those marked
`default = true` when any is, otherwise every one. build and check take the whole
selection, and a command that needs one artifact takes it only when it holds one

itn: resolves names
m: the manifest
a: the artifact asked about
target: the resolved target's name
executables_only: consider `bin` artifacts only, so a library beside them is never taken
ret: true when the default selection holds `a`

## fun default_selection_takes

```mach
pub fun default_selection_takes(alloc: *A.Allocator, itn: *intern.Interner, m: *Manifest,
target: str, a: *ArtifactDef) res[bool, outcome.Fail];
```

`default_selection_includes` for a target selector not yet resolved

alloc: owns error text
itn: resolves names
m: the manifest
target: the target selector, "" for the default target
a: the artifact asked about
ret: whether the default selection holds `a`; err from target resolution

## fun select_primary_artifact

```mach
pub fun select_primary_artifact(alloc: *A.Allocator, itn: *intern.Interner,
m: *Manifest, pick: *Selection) err[outcome.Fail];
```

fill in `pick.artifact` when none was named and several artifacts are declared:
the default selection when it holds one artifact, the sole artifact supporting
the selected target or the one marked `default = true`; a selection of several
is refused, never narrowed by table order

alloc: owns error text
itn: resolves names
m: the manifest
pick: updated in place; untouched when it already names an artifact or fewer
       than two are declared
ret: ok; err from target resolution, when no artifact supports the target,
       when more than one candidate is marked default, or when several are and none is

## fun select_sole_target_artifact

```mach
pub fun select_sole_target_artifact(alloc: *A.Allocator, itn: *intern.Interner,
m: *Manifest, pick: *Selection) err[outcome.Fail];
```

`select_primary_artifact` without the refusal: fill in `pick.artifact` only
when the default selection holds one artifact, and stay silent otherwise

alloc: owns error text
itn: resolves names
m: the manifest
pick: updated in place when a choice is clear
ret: ok unless the artifact name cannot be looked up

## fun select_sole_executable_artifact

```mach
pub fun select_sole_executable_artifact(alloc: *A.Allocator, itn: *intern.Interner,
m: *Manifest, pick: *Selection) err[outcome.Fail];
```

`select_sole_target_artifact` over `bin` artifacts only, so a bin beside a
library is chosen for `run` and `test`

alloc: owns error text
itn: resolves names
m: the manifest
pick: updated in place when a choice is clear
ret: ok unless the artifact name cannot be looked up

## fun resolve_build_unit

```mach
pub fun resolve_build_unit(alloc: *A.Allocator, itn: *intern.Interner, reg: *tgt.TargetRegistry, m: *Manifest, pick: Selection) res[BuildUnit, outcome.Fail];
```

resolve a selection into one `BuildUnit`. with no target named, an artifact
that supports exactly one declared target, or exactly one host-matching
target, pins it; otherwise `native` resolution applies. the profile comes
from `resolve_profile`, the artifact from `resolve_artifact`, and every path
is expanded with `expand_project_path`

alloc: owns `libs` and temporary strings
itn: interns the expanded paths
m: the manifest
pick: the selection
ret: the unit; err from target, profile or artifact resolution, when the
       artifact does not support the target (naming the artifact's targets when
       none was selected), or from path expansion

## fun resolve_scoped_build_unit

```mach
pub fun resolve_scoped_build_unit(alloc: *A.Allocator, itn: *intern.Interner, reg: *tgt.TargetRegistry, s: *Scope, pick: Selection) res[BuildUnit, outcome.Fail];
```

`resolve_build_unit` for a cell of `s`: target, artifact and links resolve in
`s.owner`, the profile and `[project].out` in `s.root`, and the artifact output
is homed under the scope's artifact root

alloc: owns `libs` and temporary strings
itn: interns the expanded paths
s: the scope
pick: the selection, naming targets and artifacts of `s.owner` and a profile of `s.root`
ret: as `resolve_build_unit`

## fun check_collisions

```mach
pub fun check_collisions(alloc: *A.Allocator, itn: *intern.Interner, reg: *tgt.TargetRegistry, m: *Manifest,
v: *TmplVars) err[outcome.Fail];
```

reject two artifacts whose `out` expands to the same path under one set of
template values

alloc: owns temporary strings
itn: resolves names
m: the manifest; fewer than two artifacts always pass
v: the template values
ret: ok; err "mach.toml: artifacts 'a' and 'b' collide on output path '<p>'",
       or an expansion error

