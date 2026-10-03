# mach.lang.manifest.select

the one selection model: the selection a command's `-a`, `-t` and `-p` options
and `--all` describe, resolved against a manifest into the (artifact, target,
profile) cells it covers, which the build plan takes. each axis takes exact
names and globs (`*` any run, `?` any one byte); an axis given no pattern is
`*` under `--all` and otherwise the manifest's default, which the
`ArtifactDefault` modes, `resolve_target` and `resolve_profile` decide. no cell
is chosen by table order

## def ArtifactDefault

```mach
pub def ArtifactDefault: u8
```

how an artifact axis given no pattern is filled when `--all` does not fill it

## val ARTIFACT_DEFAULT_SET

```mach
pub val ARTIFACT_DEFAULT_SET: ArtifactDefault = 0
```

every artifact the default selection holds for the target: build and check

## val ARTIFACT_DEFAULT_ONE

```mach
pub val ARTIFACT_DEFAULT_ONE: ArtifactDefault = 1
```

the one artifact the default selection holds, refused when it holds several: test and doc

## val ARTIFACT_DEFAULT_EXECUTABLE

```mach
pub val ARTIFACT_DEFAULT_EXECUTABLE: ArtifactDefault = 2
```

the one `bin` the default selection holds, else the sole declared artifact: run

## rec Selectors

```mach
pub rec Selectors;
```

the selector axes as a command gave them

artifacts: `-a` patterns in the order given
targets: `-t` patterns in the order given
profiles: `-p` patterns in the order given
all: `--all`: every axis given no pattern is `*`

## rec Cell

```mach
pub rec Cell;
```

one resolved cell of a selection

artifact: the declared artifact name
target: the target the cell is planned with: the declared name, or "" when
             the target axis took the manifest's default, so planning resolves it
             (and warns about a fallback) exactly as an unselected build does
target_name: the declared target the cell resolves to
profile: the declared profile name
native: the target's `os` and `isa` are the host's, so its programs run here

## fun cell_of

```mach
pub fun cell_of(artifact: str, target: str, profile: str) Cell;
```

a cell named outright, for a caller that already holds exact names: a
dependency's requirement, or a cell carried from an earlier plan

artifact: the declared artifact name
target: the declared target name, or "" for the artifact's default target
profile: the declared profile name, or "" for the default profile

## rec ResolvedTarget

```mach
pub rec ResolvedTarget;
```

the outcome of target resolution for a selection

target: the chosen target; points into the manifest unless synthesized for a
        manifest with no `[target.*]`, in which case it is allocated and never freed by `dnit`
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
allow: as `ProfileDef`

## fun resolve_target

```mach
pub fun resolve_target(alloc: *A.Allocator, itn: *intern.Interner, m: *Manifest, selector: str,
art: *ArtifactDef) res[ResolvedTarget, fail.Fail];
```

the target a selector names: a declared name, `native` for the declared
target matching the host, or "" for the artifact's pinned target and
otherwise `native`

## fun resolve_profile

```mach
pub fun resolve_profile(alloc: *A.Allocator, itn: *intern.Interner, m: *Manifest, pick: str) res[ResolvedProfile, fail.Fail];
```

choose the profile a selection names. a named profile must be declared. an
empty name takes the sole declared profile, else the one with `default = true`,
of which parse admits at most one. several declared profiles with none marked
default are refused: no profile is ever selected by table order

alloc: owns error text
itn: resolves the names
m: the manifest
pick: the profile name, or "" for the default
ret: the profile; err "mach.toml: no profile named '<pick>'" or, with an empty
       name, an error when several profiles are declared and none is the default

## fun default_selection_includes

```mach
pub fun default_selection_includes(itn: *intern.Interner, m: *Manifest, a: *ArtifactDef,
target: intern.StrId, executables_only: bool) bool;
```

the default selection: what a command takes for a target when no `-a`
names an artifact. of the artifacts the target builds, those marked
`default = true` when any is, otherwise every one. build and check take the whole
selection, and a command that needs one artifact takes it only when it holds one

itn: resolves names
m: the manifest
a: the artifact asked about
target: the resolved target's name
executables_only: consider `bin` artifacts only, so a library beside them is never taken
ret: true when the default selection holds `a`

## fun selectors_init

```mach
pub fun selectors_init(alloc: *A.Allocator) Selectors;
```

empty selectors: every axis takes the manifest's default

alloc: backs the three pattern vectors

## fun selectors_dnit

```mach
pub fun selectors_dnit(s: *Selectors);
```

release the pattern vectors of selectors; the patterns themselves are borrowed

## fun selectors_of

```mach
pub fun selectors_of(alloc: *A.Allocator, artifact: str, target: str, profile: str) res[Selectors, fail.Fail];
```

selectors naming at most one pattern per axis: each non-empty value is its
axis's one pattern, and an empty value leaves the axis to the manifest's default

alloc: backs the three pattern vectors
artifact: the `-a` pattern, or empty
target: the `-t` pattern, or empty
profile: the `-p` pattern, or empty
ret: the selectors; err when a vector cannot grow

## fun single_selection_fail

```mach
pub fun single_selection_fail(alloc: *A.Allocator, k: diagnostic_kind.Kind, what: str, cells: *Vector[Cell]) fail.Fail;
```

the refusal of an option or command that needs one (artifact, target, profile)
when the selection resolved to several, naming each

alloc: owns the message
k: the kind the refusal is reported as
what: what needs one, such as "-o names one output"
cells: the resolved cells

## fun resolve_cells

```mach
pub fun resolve_cells(alloc: *A.Allocator, itn: *intern.Interner, m: *Manifest, s: *Selectors,
mode: ArtifactDefault) res[Vector[Cell], fail.Fail];
```

resolve selectors against a manifest into the cells they cover, profiles
outermost, then targets, then artifacts, each in declaration order. an
artifact is paired only with the targets its `targets` list supports: an
exact artifact with an exact target it does not support is refused, any
other unsupported pair is left out. an empty selection is refused

alloc: owns the cells and every message
itn: resolves names
m: the root manifest
s: the selectors
mode: how an artifact axis given no pattern is filled without `--all`
ret: the cells, never empty; err for an unknown name, a glob matching
       nothing, an unsupported exact pair, an ambiguous default or an empty selection

## fun resolve_cell

```mach
pub fun resolve_cell(alloc: *A.Allocator, itn: *intern.Interner, m: *Manifest, s: *Selectors,
mode: ArtifactDefault, what: str) res[Cell, fail.Fail];
```

resolve selectors a command needs one cell of: `resolve_cells`, refused as
`single_selection_fail` refuses when the selection holds several

what: what needs one cell, such as "mach doc renders one artifact"
ret: the cell; the `resolve_cells` errors, or the refusal of several cells

