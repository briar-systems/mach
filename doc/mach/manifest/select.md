[mach](../README.md) / [manifest](README.md) / **select**

# mach.manifest.select

the one selection model: the selection a command's `-a`, `-t` and `-p` options
and `--all` describe, resolved against a manifest into the (artifact, target,
profile) cells it covers, which the build plan takes. each axis takes exact
names and globs (`*` any run, `?` any one byte); an axis given no pattern is
`*` under `--all` and otherwise the manifest's default, which the
`ArtifactDefault` modes, `resolve_target` and `resolve_profile` decide. no cell
is chosen by table order

Source: [`src/manifest/select.mach`](../../../src/manifest/select.mach)

Uses types from [`mach.base.diagnostic.kind`](../base/diagnostic/kind.md), [`mach.base.fail`](../base/fail.md), [`mach.base.intern`](../base/intern.md).

| Type | | |
| :--- | :--- | :--- |
| [`ArtifactDefault`](#artifactdefault) | `def` | how an artifact axis given no pattern is filled when `--all` does not fill it |
| [`Selectors`](#selectors) | `rec` | the selector axes as a command gave them |
| [`Cell`](#cell) | `rec` | one resolved cell of a selection |
| [`ResolvedTarget`](#resolvedtarget) | `rec` | the outcome of target resolution for a selection |
| [`ResolvedProfile`](#resolvedprofile) | `rec` | the profile a selection resolves to, copied out of its `ProfileDef` |

| On `ArtifactDefault` | |
| :--- | :--- |
| [`ARTIFACT_DEFAULT_SET`](#artifactdefault) | every artifact the default selection holds for the target: build and check |
| [`ARTIFACT_DEFAULT_ONE`](#artifactdefault) | the one artifact the default selection holds, refused when it holds several: test and doc |
| [`ARTIFACT_DEFAULT_EXECUTABLE`](#artifactdefault) | the one `bin` the default selection holds, else the sole declared artifact: run |

| On `Selectors` | |
| :--- | :--- |
| [`selectors_init`](#selectors_init) | empty selectors: every axis takes the manifest's default |
| [`selectors_dnit`](#selectors_dnit) | release the pattern vectors of selectors; the patterns themselves are borrowed |
| [`selectors_of`](#selectors_of) | selectors naming at most one pattern per axis: each non-empty value is its axis's one pattern, and an empty value leaves the axis to the manifest's default |
| [`resolve_cells`](#resolve_cells) | resolve selectors against a manifest into the cells they cover, profiles outermost, then targets, then artifacts, each in declaration order |
| [`resolve_cell`](#resolve_cell) | resolve selectors a command needs one cell of: `resolve_cells`, refused as `single_selection_fail` refuses when the selection holds several |

| On `Cell` | |
| :--- | :--- |
| [`cell_of`](#cell_of) | a cell named outright, for a caller that already holds exact names: a dependency's requirement, or a cell carried from an earlier plan |
| [`single_selection_fail`](#single_selection_fail) | the refusal of an option or command that needs one (artifact, target, profile) when the selection resolved to several, naming each and the selectors of the axes that hold more than one value, the only ones narrowing can help |

| On `ResolvedTarget` | |
| :--- | :--- |
| [`resolve_target`](#resolve_target) | the target a selector names: a declared name, `native` for the declared target matching the host, or "" for the artifact's pinned target and otherwise `native` |

| On `ResolvedProfile` | |
| :--- | :--- |
| [`resolve_profile`](#resolve_profile) | choose the profile a selection names |

---

## ArtifactDefault

```mach
pub def ArtifactDefault: u8
```

how an artifact axis given no pattern is filled when `--all` does not fill it

| Name | Value | |
| :--- | :--- | :--- |
| `ARTIFACT_DEFAULT_SET` | `0` | every artifact the default selection holds for the target: build and check |
| `ARTIFACT_DEFAULT_ONE` | `1` | the one artifact the default selection holds, refused when it holds several: test and doc |
| `ARTIFACT_DEFAULT_EXECUTABLE` | `2` | the one `bin` the default selection holds, else the sole declared artifact: run |

## Selectors

```mach
pub rec Selectors {
    artifacts: Vector[str];
    targets:   Vector[str];
    profiles:  Vector[str];
    all:       bool;
}
```

the selector axes as a command gave them

| Field | Type | |
| :--- | :--- | :--- |
| `artifacts` | `Vector[str]` | `-a` patterns in the order given |
| `targets` | `Vector[str]` | `-t` patterns in the order given |
| `profiles` | `Vector[str]` | `-p` patterns in the order given |
| `all` | `bool` | `--all`: every axis given no pattern is `*` |

### selectors_init

```mach
pub fun selectors_init(alloc: *std.allocator.Allocator) Selectors
```

empty selectors: every axis takes the manifest's default

| Parameter | |
| :--- | :--- |
| `alloc` | backs the three pattern vectors |

### selectors_dnit

```mach
pub fun selectors_dnit(s: *Selectors)
```

release the pattern vectors of selectors; the patterns themselves are borrowed

### selectors_of

```mach
pub fun selectors_of(alloc: *std.allocator.Allocator, artifact: str, target: str, profile: str) res[Selectors, mach.base.fail.Fail]
```

selectors naming at most one pattern per axis: each non-empty value is its
axis's one pattern, and an empty value leaves the axis to the manifest's default

| Parameter | |
| :--- | :--- |
| `alloc` | backs the three pattern vectors |
| `artifact` | the `-a` pattern, or empty |
| `target` | the `-t` pattern, or empty |
| `profile` | the `-p` pattern, or empty |
| **returns** | the selectors; err when a vector cannot grow |

### resolve_cells

```mach
pub fun resolve_cells(alloc: *std.allocator.Allocator, itn: *mach.base.intern.Interner, m: *Manifest, s: *Selectors,
    mode: ArtifactDefault) res[Vector[Cell], mach.base.fail.Fail]
```

resolve selectors against a manifest into the cells they cover, profiles
outermost, then targets, then artifacts, each in declaration order. an
artifact is paired only with the targets its `targets` list supports: an
exact artifact with an exact target it does not support is refused, any
other unsupported pair is left out. an empty selection is refused

| Parameter | |
| :--- | :--- |
| `alloc` | owns the cells and every message |
| `itn` | resolves names |
| `m` | the root manifest |
| `s` | the selectors |
| `mode` | how an artifact axis given no pattern is filled without `--all` |
| **returns** | the cells, never empty; err for an unknown name, a glob matching nothing, an unsupported exact pair, an ambiguous default or an empty selection |

### resolve_cell

```mach
pub fun resolve_cell(alloc: *std.allocator.Allocator, itn: *mach.base.intern.Interner, m: *Manifest, s: *Selectors,
    mode: ArtifactDefault, what: str) res[Cell, mach.base.fail.Fail]
```

resolve selectors a command needs one cell of: `resolve_cells`, refused as
`single_selection_fail` refuses when the selection holds several

| Parameter | |
| :--- | :--- |
| `what` | what needs one cell, such as "mach doc renders one artifact" |
| **returns** | the cell; the `resolve_cells` errors, or the refusal of several cells |

## Cell

```mach
pub rec Cell {
    artifact:    str;
    target:      str;
    target_name: str;
    profile:     str;
    native:      bool;
}
```

one resolved cell of a selection

| Field | Type | |
| :--- | :--- | :--- |
| `artifact` | `str` | the declared artifact name |
| `target` | `str` | the target the cell is planned with: the declared name, or "" when the target axis took the manifest's default, so planning resolves it (and warns about a fallback) exactly as an unselected build does |
| `target_name` | `str` | the declared target the cell resolves to |
| `profile` | `str` | the declared profile name |
| `native` | `bool` | the target's `sys` and `arch` are the host's, so its programs run here |

### cell_of

```mach
pub fun cell_of(artifact: str, target: str, profile: str) Cell
```

a cell named outright, for a caller that already holds exact names: a
dependency's requirement, or a cell carried from an earlier plan

| Parameter | |
| :--- | :--- |
| `artifact` | the declared artifact name |
| `target` | the declared target name, or "" for the artifact's default target |
| `profile` | the declared profile name, or "" for the default profile |

### single_selection_fail

```mach
pub fun single_selection_fail(alloc: *std.allocator.Allocator, k: mach.base.diagnostic.kind.Kind, what: str, cells: *Vector[Cell]) mach.base.fail.Fail
```

the refusal of an option or command that needs one (artifact, target, profile)
when the selection resolved to several, naming each and the selectors of the axes
that hold more than one value, the only ones narrowing can help

| Parameter | |
| :--- | :--- |
| `alloc` | owns the message |
| `k` | the kind the refusal is reported as |
| `what` | what needs one, such as "-o names one output" |
| `cells` | the resolved cells |

## ResolvedTarget

```mach
pub rec ResolvedTarget {
    target: *TargetDef;
}
```

the outcome of target resolution for a selection

| Field | Type | |
| :--- | :--- | :--- |
| `target` | `*TargetDef` | the chosen target; points into the manifest, or for a manifest with no `[target.*]` at its synthesized host target, which the manifest owns and `dnit` frees target borrows its manifest-owned declaration until manifest destruction |

### resolve_target

```mach
pub fun resolve_target(alloc: *std.allocator.Allocator, itn: *mach.base.intern.Interner, m: *Manifest, selector: str,
    art: *ArtifactDef) res[ResolvedTarget, mach.base.fail.Fail]
```

the target a selector names: a declared name, `native` for the declared
target matching the host, or "" for the artifact's pinned target and
otherwise `native`

## ResolvedProfile

```mach
pub rec ResolvedProfile {
    name:         mach.base.intern.StrId;
    optimize:     bool;
    passes:       PassChoices;
    relax:        NameList;
    relax_named:  str;
    linker_relax: bool;
    symbols:      bool;
    sections:     bool;
    unwind:       bool;
    debug:        bool;
    simd:         SimdMode;
    allow:        mach.base.diagnostic.kind.KindSet;
    written:      bool;
}
```

the profile a selection resolves to, copied out of its `ProfileDef`

| Field | Type | |
| :--- | :--- | :--- |
| `name` | `mach.base.intern.StrId` | the profile's name |
| `optimize` | `bool` | as `ProfileDef` |
| `passes` | `PassChoices` | as `ProfileDef`, borrowed from the manifest |
| `relax` | `NameList` | as `ProfileDef`, borrowed from the manifest |
| `relax_named` | `str` | as `ProfileDef` |
| `linker_relax` | `bool` | as `ProfileDef` |
| `symbols` | `bool` | as `ProfileDef` |
| `sections` | `bool` | as `ProfileDef` |
| `unwind` | `bool` | as `ProfileDef` |
| `debug` | `bool` | as `ProfileDef` |
| `simd` | `SimdMode` | as `ProfileDef` |
| `allow` | `mach.base.diagnostic.kind.KindSet` | as `ProfileDef` |
| `written` | `bool` | as `ProfileDef` |

### resolve_profile

```mach
pub fun resolve_profile(alloc: *std.allocator.Allocator, itn: *mach.base.intern.Interner, m: *Manifest, pick: str) res[ResolvedProfile, mach.base.fail.Fail]
```

choose the profile a selection names. a named profile must be declared. an
empty name takes the sole declared profile, else the one with `default = true`,
of which parse admits at most one. several declared profiles with none marked
default are refused: no profile is ever selected by table order

| Parameter | |
| :--- | :--- |
| `alloc` | owns error text |
| `itn` | resolves the names |
| `m` | the manifest |
| `pick` | the profile name, or "" for the default |
| **returns** | the profile; err "mach.toml: no profile named '\<pick>'" or, with an empty name, an error when several profiles are declared and none is the default |

