# mach.lang.target.catalog.artifact

the artifact kind catalog: every kind of output a build produces, how a
manifest spells it, how a build plan labels it, and what producing it asks of
the linker. a kind says what an output is for, not how its object format
writes it: a spir-v module, a wasm module and a raw image are executables of
formats that finish each module or image themselves

## def Kind

```mach
pub def Kind: u8
```

## val EXECUTABLE

```mach
pub val EXECUTABLE: Kind = 0
```

## val OBJECTS

```mach
pub val OBJECTS:    Kind = 1
```

## val SHARED

```mach
pub val SHARED:     Kind = 2
```

## val STATIC

```mach
pub val STATIC:     Kind = 3
```

## fun from_name

```mach
pub fun from_name(name: str) opt[Kind];
```

the kind a manifest's `kind` value names; absent for a value no kind declares

## fun name_for

```mach
pub fun name_for(k: Kind) str;
```

the manifest spelling of a kind; empty for one a manifest cannot declare

## fun declarable_count

```mach
pub fun declarable_count() usize;
```

how many kinds a manifest can declare

## fun declarable_at

```mach
pub fun declarable_at(index: usize) str;
```

the manifest spelling of the declarable kind at `index`, in catalog order;
empty past the last

## fun label_for

```mach
pub fun label_for(k: Kind) str;
```

## fun output_for

```mach
pub fun output_for(k: Kind) str;
```

## fun is_library

```mach
pub fun is_library(k: Kind) bool;
```

whether the kind is a library; false for a kind outside the catalog

## fun is_linked

```mach
pub fun is_linked(k: Kind) bool;
```

whether the linker produces the kind; false for a kind outside the catalog

## fun allocates_commons

```mach
pub fun allocates_commons(k: Kind) opt[bool];
```

whether a link of the kind allocates common storage; absent for a kind the
linker does not produce

## fun is_loaded

```mach
pub fun is_loaded(k: Kind) opt[bool];
```

whether a loader maps the kind on its own account; absent for a kind the
linker does not produce

