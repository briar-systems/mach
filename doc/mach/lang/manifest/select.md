# mach.lang.manifest.select

the selection a command's `-a`, `-t` and `-p` options and `--all` describe,
resolved against a manifest into the (artifact, target, profile) cells it
covers. each axis takes exact names and globs (`*` any run, `?` any one
byte); an axis given no pattern is `*` under `--all` and otherwise the
manifest's default. no cell is chosen by table order

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

## fun selectors_init

```mach
pub fun selectors_init(alloc: *std_allocator.Allocator) Selectors;
```

empty selectors: every axis takes the manifest's default

alloc: backs the three pattern vectors

## fun host_label

```mach
pub fun host_label(alloc: *std_allocator.Allocator) str;
```

the host as `mach info` names it, `<os>-<isa>`, which a cell is native to when
its target's os and isa are these

## fun cell_label

```mach
pub fun cell_label(alloc: *std_allocator.Allocator, c: *Cell) str;
```

the cell as a person names it: `<artifact> on <target> (<profile>)`

## fun cell_labels

```mach
pub fun cell_labels(alloc: *std_allocator.Allocator, cells: *Vector[Cell]) str;
```

every cell's label, comma separated, for a refusal that names the selection

## fun single_selection_msg

```mach
pub fun single_selection_msg(alloc: *std_allocator.Allocator, what: str, cells: *Vector[Cell]) str;
```

the refusal of an option or command that needs one (artifact, target, profile)
when the selection resolved to several, naming each

alloc: owns the message
what: what needs one, such as "-o names one output"
cells: the resolved cells

## fun resolve_cells

```mach
pub fun resolve_cells(alloc: *std_allocator.Allocator, itn: *intern.Interner, m: *Manifest, s: *Selectors,
mode: ArtifactDefault) res[Vector[Cell], outcome.Fail];
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

