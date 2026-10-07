# mach.lang.manifest.artifact

## fun parse_artifacts

```mach
pub fun parse_artifacts(alloc: *A.Allocator, itn: *intern.Interner, t: *toml.Table, m: *Manifest) err[fail.Fail];
```

## fun export_validate

```mach
pub fun export_validate(alloc: *A.Allocator, itn: *intern.Interner, m: *Manifest) err[fail.Fail];
```

more than one `export = true` artifact is refused, pointing at each `export` value

## fun export_artifact

```mach
pub fun export_artifact(m: *Manifest) *ArtifactDef;
```

the export library: the one artifact marked `export = true`, nil when none is

## fun find_artifact

```mach
pub fun find_artifact(itn: *intern.Interner, m: *Manifest, name: str) *ArtifactDef;
```

the artifact `m` declares under `name`, nil when none is

## fun find_artifact_by_name

```mach
pub fun find_artifact_by_name(m: *Manifest, name: intern.StrId) *ArtifactDef;
```

## fun artifact_needs_artifact

```mach
pub fun artifact_needs_artifact(itn: *intern.Interner, a: *ArtifactDef, other: *ArtifactDef) bool;
```

whether a `need` entry of `a` selects `other`, a different artifact

## fun expand_artifact_output

```mach
pub fun expand_artifact_output(alloc: *A.Allocator, itn: *intern.Interner, reg: *lang_target.TargetRegistry,
a: *ArtifactDef, project_work: str, vars: *template.Values) res[str, fail.Fail];
```

expand an artifact's `out` for the target `vars` names; a refusal points at the `out` value

## fun artifact_supports_target

```mach
pub fun artifact_supports_target(itn: *intern.Interner, a: *ArtifactDef, tname: intern.StrId) bool;
```

whether an artifact's `targets` lists a target name or "*"

itn: resolves the names
a: the artifact
tname: the target name
ret: true when listed

