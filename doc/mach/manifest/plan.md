[mach](../README.md) / [manifest](README.md) / **plan**

# mach.manifest.plan

a build cell resolved into a build unit: target, profile, artifact, paths and links

a cell resolves in a scope: the root's own, or a dependency's, whose artifact, targets
and links are the dependency's and whose profile and work directory are the root's. the
steps a cell runs first and the artifacts it requires are ordered here too.

Source: [`src/manifest/plan.mach`](../../../src/manifest/plan.mach)

Uses types from [`mach.base.fail`](../base/fail.md), [`mach.base.intern`](../base/intern.md), [`mach.manifest.template`](template.md).

| Type | | |
| :--- | :--- | :--- |
| [`BuildUnit`](#buildunit) | `rec` | one resolved build cell: a target, a profile, and an artifact with every path expanded |
| [`Scope`](#scope) | `rec` | the manifest that declares a build cell, and the root project the cell is built for |

| On `BuildUnit` | |
| :--- | :--- |
| [`resolve_build_unit`](#resolve_build_unit) | resolve one cell into a `BuildUnit` |

| On `Scope` | |
| :--- | :--- |
| [`root_scope`](#root_scope) | the scope of the root project's own cells, `m` both root and owner and `dir` both directories |
| [`dependency_scope`](#dependency_scope) | the scope of a cell `owner` declares, built for the project `root` |
| [`scope_is_dependency`](#scope_is_dependency) | whether the scope's cells belong to a dependency rather than the root |
| [`resolve_artifact_reqs`](#resolve_artifact_reqs) | list the artifacts a consumer requires, each with its output path when that path is unique, for `{artifact.<id>.out}` expansion |
| [`resolve_export_reqs`](#resolve_export_reqs) | list the artifacts a dependency's export library requires, the scope `{artifact.<id>.out}` resolves in for that dependency's modules |
| [`resolve_scoped_build_unit`](#resolve_scoped_build_unit) | `resolve_build_unit` for a cell of `s`: target, artifact and links resolve in `s.owner`, the profile and `[project].work` in `s.root`, and `{project.work}` in the artifact output names the scope's artifact root |

| Functions | |
| :--- | :--- |
| [`local_path_demanded`](#local_path_demanded) | whether the build makes a path for target `t`: some build step's output, or the output of an artifact built for `t`, as a shared library an executable of the project links |
| [`required_artifact_uses_target`](#required_artifact_uses_target) | whether a required artifact is built for a target when its consumer builds for `consumer_target`: only that target when `req` supports it, otherwise every target `req` supports |
| [`export_requires`](#export_requires) | whether the export library selects `ra` in its `need`: the requirements a dependency's export library carries to every consumer |
| [`plan_steps`](#plan_steps) | order the build steps the named artifacts demand: a step producing a local link path that matches the target, every step a `need` names (globs match step names), and transitively each step's own `need`, dependencies first |
| [`plan_export_steps`](#plan_export_steps) | order the build steps that produce an exported local link path matching the target and the steps the export library's `need` names, with their transitive step `need`s; the steps a consumer of this manifest must run |
| [`check_collisions`](#check_collisions) | reject two artifacts whose `out` expands to the same path under one set of template values |

---

## BuildUnit

```mach
pub rec BuildUnit {
    target:       *TargetDef;
    artifact:     *ArtifactDef;
    target_name:  mach.base.intern.StrId;
    profile_name: mach.base.intern.StrId;
    entry:        mach.base.intern.StrId;
    bin_path:     mach.base.intern.StrId;
    out_root:     mach.base.intern.StrId;
    optimize:     bool;
    debug:        bool;
    simd:         SimdMode;
    libs:         *LinkRequirement;
    lib_count:    u32;
}
```

one resolved build cell: a target, a profile, and an artifact with every path
expanded. produced by `resolve_build_unit`; `libs` is owned by the unit

| Field | Type | |
| :--- | :--- | :--- |
| `target` | `*TargetDef` | the resolved target; points into the manifest, except for the host target synthesized when the manifest declares no targets |
| `artifact` | `*ArtifactDef` | the resolved artifact, pointing into the manifest |
| `target_name` | `mach.base.intern.StrId` | `target.name` |
| `profile_name` | `mach.base.intern.StrId` | the resolved profile's name |
| `entry` | `mach.base.intern.StrId` | the artifact's `entry` |
| `bin_path` | `mach.base.intern.StrId` | the artifact's output: its expanded `out`, with `{project.work}` naming the scope's artifact root, STR_NIL for an object artifact |
| `out_root` | `mach.base.intern.StrId` | `<expanded project work>`, whose layout beneath it the build owns (mach.host.output.layout) |
| `optimize` | `bool` | from the resolved profile |
| `debug` | `bool` | from the resolved profile |
| `simd` | `SimdMode` | from the resolved profile |
| `libs` | `*LinkRequirement` | the artifact's `link` entries that match the target, resolved in `link` order and sized exactly; a `link` name matching no `[link.*]` table is skipped |
| `lib_count` | `u32` | length of `libs` |

### resolve_build_unit

```mach
pub fun resolve_build_unit(alloc: *std.allocator.Allocator, itn: *mach.base.intern.Interner, m: *Manifest, dir: str, c: *Cell) res[BuildUnit, mach.base.fail.Fail]
```

resolve one cell into a `BuildUnit`. the cell names its artifact; with no
target named, an artifact that supports exactly one declared target, or exactly
one host-matching target, pins it, except that a sole hosted target the host
cannot run is refused as `native` refuses it; otherwise `native` resolution
applies. the profile comes from `resolve_profile`, and every path is expanded
through the template engine

| Parameter | |
| :--- | :--- |
| `alloc` | owns `libs` and temporary strings |
| `itn` | interns the expanded paths |
| `m` | the manifest |
| `dir` | the project's directory |
| `c` | the cell, as `resolve_cells` resolves it or as `cell_of` names it |
| **returns** | the unit; err from target or profile resolution, for an artifact the manifest does not declare, when the artifact does not support the target (naming the artifact's targets when none was selected), or from path expansion |

## Scope

```mach
pub rec Scope {
    root:      *Manifest;
    owner:     *Manifest;
    root_dir:  str;
    owner_dir: str;
}
```

the manifest that declares a build cell, and the root project the cell is built
for. a root cell has `owner == root`. a dependency's cell is one of the
requirements its export library carries: it resolves its artifact, targets and
links in `owner`, its profile in `root`, expands the root's `[project].work`, and
reads `{project.work}` in its artifact outputs as `dep/<owner id>` of that out,
so identically named artifacts of two dependencies never share a path

| Field | Type | |
| :--- | :--- | :--- |
| `root` | `*Manifest` | the manifest of the project being built |
| `owner` | `*Manifest` | the manifest declaring the cell's artifact; `root` for the root's own |
| `root_dir` | `str` | the root project's directory, which a local link path naming `{project.work}` lies under |
| `owner_dir` | `str` | the owner's directory, which its other local link paths lie under |

### root_scope

```mach
pub fun root_scope(m: *Manifest, dir: str) Scope
```

the scope of the root project's own cells, `m` both root and owner and `dir` both directories

| Parameter | |
| :--- | :--- |
| `m` | the root project's manifest |
| `dir` | its directory |
| **returns** | the scope |

### dependency_scope

```mach
pub fun dependency_scope(root: *Manifest, root_dir: str, owner: *Manifest, owner_dir: str) Scope
```

the scope of a cell `owner` declares, built for the project `root`

| Parameter | |
| :--- | :--- |
| `root` | the manifest of the project being built |
| `root_dir` | its directory |
| `owner` | the manifest declaring the cell |
| `owner_dir` | its directory |
| **returns** | the scope |

### scope_is_dependency

```mach
pub fun scope_is_dependency(s: *Scope) bool
```

whether the scope's cells belong to a dependency rather than the root

### resolve_artifact_reqs

```mach
pub fun resolve_artifact_reqs(alloc: *std.allocator.Allocator, itn: *mach.base.intern.Interner, s: *Scope,
    consumer: *ArtifactDef, consumer_target: mach.base.intern.StrId, profile: str,
    whole_project: bool,
    out_items: **mach.manifest.template.Requirement, out_count: *u32) err[mach.base.fail.Fail]
```

list the artifacts a consumer requires, each with its output path when that
path is unique, for `{artifact.<id>.out}` expansion

| Parameter | |
| :--- | :--- |
| `alloc` | owns the returned array and each non-empty `out` string |
| `itn` | resolves names |
| `s` | the scope; the requirements are declared by `s.owner` |
| `consumer` | the requiring artifact; nil with `whole_project` false yields nothing |
| `consumer_target` | the consumer's target name |
| `profile` | the profile name the outputs expand with |
| `whole_project` | select every artifact some artifact needs instead of `consumer`'s |
| `out_items` | receives the array, or nil when nothing is required |
| `out_count` | receives its length |
| **returns** | ok; err from the output path expansion, with the array freed |

### resolve_export_reqs

```mach
pub fun resolve_export_reqs(alloc: *std.allocator.Allocator, itn: *mach.base.intern.Interner, s: *Scope, profile: str, out_items: **mach.manifest.template.Requirement, out_count: *u32) err[mach.base.fail.Fail]
```

list the artifacts a dependency's export library requires, the scope
`{artifact.<id>.out}` resolves in for that dependency's modules. each requirement
builds for every target it names in the dependency's manifest

| Parameter | |
| :--- | :--- |
| `alloc` | owns the returned array and each non-empty `out` string |
| `itn` | resolves names |
| `s` | a dependency scope |
| `profile` | the root's profile name the outputs expand with |
| `out_items` | receives the array, or nil when nothing is required |
| `out_count` | receives its length |
| **returns** | ok; err from the output path expansion, with the array freed |

### resolve_scoped_build_unit

```mach
pub fun resolve_scoped_build_unit(alloc: *std.allocator.Allocator, itn: *mach.base.intern.Interner, s: *Scope, c: *Cell) res[BuildUnit, mach.base.fail.Fail]
```

`resolve_build_unit` for a cell of `s`: target, artifact and links resolve in
`s.owner`, the profile and `[project].work` in `s.root`, and `{project.work}` in
the artifact output names the scope's artifact root

| Parameter | |
| :--- | :--- |
| `alloc` | owns `libs` and temporary strings |
| `itn` | interns the expanded paths |
| `s` | the scope |
| `c` | the cell, naming a target and an artifact of `s.owner` and a profile of `s.root` |
| **returns** | as `resolve_build_unit` |

## Functions

### local_path_demanded

```mach
pub fun local_path_demanded(alloc: *std.allocator.Allocator, itn: *mach.base.intern.Interner, m: *Manifest, t: *TargetDef,
    expanded_path: str, proj_out: str, v: *mach.manifest.template.Values) res[bool, mach.base.fail.Fail]
```

whether the build makes a path for target `t`: some build step's output, or the output of
an artifact built for `t`, as a shared library an executable of the project links

### required_artifact_uses_target

```mach
pub fun required_artifact_uses_target(itn: *mach.base.intern.Interner, req: *ArtifactDef,
    consumer_target: mach.base.intern.StrId, t: *TargetDef) bool
```

whether a required artifact is built for a target when its consumer builds for
`consumer_target`: only that target when `req` supports it, otherwise every
target `req` supports

| Parameter | |
| :--- | :--- |
| `itn` | resolves the names |
| `req` | the required artifact |
| `consumer_target` | the consumer's target name, or STR_NIL for a consumer outside `req`'s manifest, which admits every target `req` supports |
| `t` | the target asked about |
| **returns** | true when `req` is built for `t` on behalf of that consumer |

### export_requires

```mach
pub fun export_requires(itn: *mach.base.intern.Interner, m: *Manifest, ra: *ArtifactDef) bool
```

whether the export library selects `ra` in its `need`: the requirements a
dependency's export library carries to every consumer

| Parameter | |
| :--- | :--- |
| `itn` | resolves the names |
| `m` | the manifest |
| `ra` | the candidate requirement |
| **returns** | true when the artifact marked `export = true` needs it |

### plan_steps

```mach
pub fun plan_steps(alloc: *std.allocator.Allocator, itn: *mach.base.intern.Interner, m: *Manifest,
    art_names: *mach.base.intern.StrId, art_count: u32, t: *TargetDef,
    proj_out: str, v: *mach.manifest.template.Values,
    out_order: **u32, out_count: *u32) err[mach.base.fail.Fail]
```

order the build steps the named artifacts demand: a step producing a local
link path that matches the target, every step a `need` names (globs match
step names), and transitively each step's own `need`, dependencies first

| Parameter | |
| :--- | :--- |
| `alloc` | owns the returned order |
| `itn` | resolves names |
| `m` | the manifest |
| `art_names` | the artifacts to plan for; unknown names are skipped |
| `art_count` | length of `art_names` |
| `t` | the target |
| `proj_out` | the expanded `[project].work` |
| `v` | the template values |
| `out_order` | receives the step indices in run order, sized exactly; nil when none |
| `out_count` | receives the length |
| **returns** | ok; err on a step `need` cycle or a local link path that fails to expand |

### plan_export_steps

```mach
pub fun plan_export_steps(alloc: *std.allocator.Allocator, itn: *mach.base.intern.Interner, m: *Manifest,
    t: *TargetDef, proj_out: str, v: *mach.manifest.template.Values,
    out_order: **u32, out_count: *u32) err[mach.base.fail.Fail]
```

order the build steps that produce an exported local link path matching the
target and the steps the export library's `need` names, with their
transitive step `need`s; the steps a consumer of this manifest must run

| Parameter | |
| :--- | :--- |
| `alloc` | owns the returned order |
| `itn` | resolves names |
| `m` | the manifest |
| `t` | the target |
| `proj_out` | the expanded `[project].work` |
| `v` | the template values |
| `out_order` | receives the step indices in run order, sized exactly; nil when none |
| `out_count` | receives the length |
| **returns** | ok; the `plan_steps` errors |

### check_collisions

```mach
pub fun check_collisions(alloc: *std.allocator.Allocator, itn: *mach.base.intern.Interner, m: *Manifest,
    v: *mach.manifest.template.Values) err[mach.base.fail.Fail]
```

reject two artifacts whose `out` expands to the same path under one set of
template values

| Parameter | |
| :--- | :--- |
| `alloc` | owns temporary strings |
| `itn` | resolves names |
| `m` | the manifest; fewer than two artifacts always pass |
| `v` | the template values |
| **returns** | ok; err "mach.toml: artifacts 'a' and 'b' collide on output path '\<p>'", or an expansion error |

