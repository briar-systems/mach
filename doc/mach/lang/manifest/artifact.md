# mach.lang.manifest.artifact

## fun artifact_set

```mach
pub fun artifact_set(alloc: *A.Allocator, itn: *intern.Interner, lead: str, sub: *toml.Table, d: *ArtifactDef) err[fail.Fail];
```

every key `--set` writes over an artifact that the table `sub` holds, decoded over `d` with
where it was written; a key `sub` does not hold leaves `d` as it is

alloc: owns a refusal's text
itn: holds the name a refusal of the value leads with
lead: the artifact table's label in mach.toml, nil for a `--set` value
sub: the table
d: the artifact
ret: ok, or the refusal of a value the key does not accept

## fun subsystem_named

```mach
pub fun subsystem_named(a: *A.Allocator, name: str) res[*mink_subsystem.Subsystem, str];
```

the PE subsystem mink spells `name`, or why it names none: the spelling a previous major took
for the graphical subsystem is refused naming mink's, and any other unknown name is refused
listing mink's

a: owns the refusal's text
ret: mink's row, or the refusal's text

## fun parse_artifacts

```mach
pub fun parse_artifacts(alloc: *A.Allocator, itn: *intern.Interner, t: *toml.Table, m: *Manifest) err[fail.Fail];
```

## fun product_choices

```mach
pub fun product_choices(alloc: *A.Allocator) res[str, fail.Fail];
```

mink's products, quoted and joined as a list a refusal of an unknown product offers

alloc: owns the list
ret: the list; err a refused allocation

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
pub fun expand_artifact_output(alloc: *A.Allocator, itn: *intern.Interner, a: *ArtifactDef, project_work: str, vars: *template.Values) res[str, fail.Fail];
```

expand an artifact's `out` for the target `vars` names; a refusal points at the `out` value

## fun linkage_resolved

```mach
pub fun linkage_resolved(a: *A.Allocator, t: *resolved.Target, named: opt[*Linkage], from: *Origin) res[*Linkage, fail.Fail];
```

the linkage of the executable an artifact stating `named`, none for the target's default,
builds for `t`, as mink resolves it against the host. a refusal of a linkage the artifact
names says where it was written and points at it

a: owns the refusal's text
t: the resolved target
named: the artifact's `linkage`
from: where it was written

## fun artifact_supports_target

```mach
pub fun artifact_supports_target(itn: *intern.Interner, a: *ArtifactDef, tname: intern.StrId) bool;
```

whether an artifact's `targets` lists a target name or "*"

itn: resolves the names
a: the artifact
tname: the target name
ret: true when listed

