[mach](../README.md) / [manifest](README.md) / **artifact**

# mach.manifest.artifact

the `[artifact.<name>]` tables: their decoding, `--set` over them, and lookups

at most one artifact is `export = true`, an artifact's `out` expands through the
template engine, and its PE subsystem is named as mink spells it.

Source: [`src/manifest/artifact.mach`](../../../src/manifest/artifact.mach)

Uses types from [`mach.base.fail`](../base/fail.md), [`mach.base.intern`](../base/intern.md), [`mach.manifest.template`](template.md).

| Functions | |
| :--- | :--- |
| [`artifact_set`](#artifact_set) | every key `--set` writes over an artifact that the table `sub` holds, decoded over `d` with where it was written; a key `sub` does not hold leaves `d` as it is |
| [`subsystem_named`](#subsystem_named) | the PE subsystem mink spells `name`, or why it names none: the spelling a previous major took for the graphical subsystem is refused naming mink's, and any other unknown name is refused listing mink's |
| [`parse_artifacts`](#parse_artifacts) | |
| [`export_validate`](#export_validate) | more than one `export = true` artifact is refused, pointing at each `export` value |
| [`export_artifact`](#export_artifact) | the export library: the one artifact marked `export = true`, nil when none is |
| [`find_artifact`](#find_artifact) | the artifact `m` declares under `name`, nil when none is |
| [`find_artifact_by_name`](#find_artifact_by_name) | |
| [`artifact_needs_artifact`](#artifact_needs_artifact) | whether a `need` entry of `a` selects `other`, a different artifact |
| [`expand_artifact_output`](#expand_artifact_output) | expand an artifact's `out` for the target `vars` names; a refusal points at the `out` value |
| [`artifact_supports_target`](#artifact_supports_target) | whether an artifact's `targets` lists a target name or "*" |

---

## Functions

### artifact_set

```mach
pub fun artifact_set(alloc: *std.allocator.Allocator, itn: *mach.base.intern.Interner, lead: str, sub: *toml.value.Table, d: *ArtifactDef) err[mach.base.fail.Fail]
```

every key `--set` writes over an artifact that the table `sub` holds, decoded over `d` with
where it was written; a key `sub` does not hold leaves `d` as it is

| Parameter | |
| :--- | :--- |
| `alloc` | owns a refusal's text |
| `itn` | holds the name a refusal of the value leads with |
| `lead` | the artifact table's label in mach.toml, nil for a `--set` value |
| `sub` | the table |
| `d` | the artifact |
| **returns** | ok, or the refusal of a value the key does not accept |

### subsystem_named

```mach
pub fun subsystem_named(a: *std.allocator.Allocator, name: str) res[*mink.catalog.subsystem.Subsystem, str]
```

the PE subsystem mink spells `name`, or why it names none: the spelling a previous major took
for the graphical subsystem is refused naming mink's, and any other unknown name is refused
listing mink's

| Parameter | |
| :--- | :--- |
| `a` | owns the refusal's text |
| **returns** | mink's row, or the refusal's text |

### parse_artifacts

```mach
pub fun parse_artifacts(alloc: *std.allocator.Allocator, itn: *mach.base.intern.Interner, t: *toml.value.Table, m: *Manifest) err[mach.base.fail.Fail]
```

### export_validate

```mach
pub fun export_validate(alloc: *std.allocator.Allocator, itn: *mach.base.intern.Interner, m: *Manifest) err[mach.base.fail.Fail]
```

more than one `export = true` artifact is refused, pointing at each `export` value

### export_artifact

```mach
pub fun export_artifact(m: *Manifest) *ArtifactDef
```

the export library: the one artifact marked `export = true`, nil when none is

### find_artifact

```mach
pub fun find_artifact(itn: *mach.base.intern.Interner, m: *Manifest, name: str) *ArtifactDef
```

the artifact `m` declares under `name`, nil when none is

### find_artifact_by_name

```mach
pub fun find_artifact_by_name(m: *Manifest, name: mach.base.intern.StrId) *ArtifactDef
```

### artifact_needs_artifact

```mach
pub fun artifact_needs_artifact(itn: *mach.base.intern.Interner, a: *ArtifactDef, other: *ArtifactDef) bool
```

whether a `need` entry of `a` selects `other`, a different artifact

### expand_artifact_output

```mach
pub fun expand_artifact_output(alloc: *std.allocator.Allocator, itn: *mach.base.intern.Interner, a: *ArtifactDef, project_work: str, vars: *mach.manifest.template.Values) res[str, mach.base.fail.Fail]
```

expand an artifact's `out` for the target `vars` names; a refusal points at the `out` value

### artifact_supports_target

```mach
pub fun artifact_supports_target(itn: *mach.base.intern.Interner, a: *ArtifactDef, tname: mach.base.intern.StrId) bool
```

whether an artifact's `targets` lists a target name or "*"

| Parameter | |
| :--- | :--- |
| `itn` | resolves the names |
| `a` | the artifact |
| `tname` | the target name |
| **returns** | true when listed |

