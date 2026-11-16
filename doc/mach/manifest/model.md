[mach](../README.md) / [manifest](README.md) / **model**

# mach.manifest.model

the manifest's model: a parsed `mach.toml` as records of interned strings and arrays

every place a definition records is resolved in the file it was read from, so a
refusal made from the model points at the entry that caused it. an entry build makes
its model directly, with source roots in place of dependencies.

<sub>source: [`src/manifest/model.mach`](../../../src/manifest/model.mach) · imported by 43 modules</sub>

<sub>`intern` is [`mach.base.intern`](../base/intern.md) · `base_source` is [`mach.base.source`](../base/source.md) · `A` is `std.allocator` · `mink_subsystem` is `mink.catalog.subsystem` · `diagnostic_kind` is [`mach.base.diagnostic.kind`](../base/diagnostic/kind.md) · `input` is [`mach.target.input`](../target/input.md) · `chrono_duration` is `std.chrono.duration` · `query` is [`mach.session.query`](../session/query.md) · `fail` is [`mach.base.fail`](../base/fail.md)</sub>

| Type | | |
| :--- | :--- | :--- |
| [`NameList`](#namelist) | `rec` | the names one of a profile's set keys lists, as written: the build checks each against the catalog it names into |
| [`SimdMode`](#simdmode) | `def` | what happens when a vector operator has no packed instruction on the target, decoded from `[profile.<name>].simd` |
| [`TargetDef`](#targetdef) | `rec` | one `[target.<name>]` table as parsed; the string fields hold interned copies of the manifest text, unvalidated against the target registry |
| [`Origin`](#origin) | `rec` | where a value a refusal names was written: in a manifest table, or by `--set` for one invocation |
| [`ArtifactDef`](#artifactdef) | `rec` | one `[artifact.<name>]` table as parsed |
| [`RegionDef`](#regiondef) | `rec` | one `[artifact.X.region.<name>]` memory region as parsed, laid out as a linker script's MEMORY lays one |
| [`ProfileDef`](#profiledef) | `rec` | one `[profile.<name>]` compilation policy as parsed |
| [`DepDef`](#depdef) | `rec` | one `[dep.<name>]` table as parsed |
| [`LinkSource`](#linksource) | `def` | where a `[link.<name>]` entry is resolved from, decoded from its `source` key |
| [`LinkDef`](#linkdef) | `rec` | one `[link.<name>]` table as parsed |
| [`LinkVersions`](#linkversions) | `rec` | the versions a shared library is stated at, as a Mach-O load command names them, each encoded as the command holds it: the major version in the high 16 bits, then the minor and the patch in 8 bits each |
| [`StepDef`](#stepdef) | `rec` | one `[step.<name>]` table as parsed |
| [`SourceRoot`](#sourceroot) | `rec` | a module prefix an entry build binds to a directory with `-s name=dir`: a package with no manifest, whose modules are the files under the directory |
| [`Manifest`](#manifest) | `rec` | a parsed `mach.toml` |

| On `NameList` | |
| :--- | :--- |
| [`names_dnit`](#names_dnit) | release what a name list owns |

| On `SimdMode` | |
| :--- | :--- |
| [`SIMD_SCALARIZE`](#simdmode) | `simd = "scalarize"`: emit the scalar expansion |
| [`SIMD_REQUIRE`](#simdmode) | `simd = "require"`: a missing packed instruction is a build error |
| [`simd_named`](#simd_named) | the mode `name` names, as a manifest and the command line write it; none for any other name |

| On `TargetDef` | |
| :--- | :--- |
| [`target_def`](#target_def) | a target with every key a table may leave out at its default: no env, isa, object format, platform, code model, extension or runner, and a zero base and stack |

| On `ArtifactDef` | |
| :--- | :--- |
| [`artifact_def`](#artifact_def) | an artifact with every key a table may leave out at its default: no list, linkage, icon, manifest, size or region, the console subsystem, neither default nor exported |

| On `ProfileDef` | |
| :--- | :--- |
| [`profile_def`](#profile_def) | a profile with every key a table may leave out at its default: not the default, no relax and no allow, the linker free to relax, and vectors scalarized |

| On `LinkSource` | |
| :--- | :--- |
| [`LINK_LOCAL`](#linksource) | `source = "local"`: a file at a project-relative `path` |
| [`LINK_SYSTEM`](#linksource) | `source = "system"`: a system library found by `name` |
| [`LINK_FRAMEWORK`](#linksource) | `source = "framework"`: a macOS framework found by `name` |

| On `LinkDef` | |
| :--- | :--- |
| [`link_def`](#link_def) | a link with every key a table may leave out at its default: no symbol or filter, not exported, always included, and a shared library |

| On `StepDef` | |
| :--- | :--- |
| [`step_dnit`](#step_dnit) | free every array a step owns, its entries and the places they are written at; the step itself is its owner's |

| On `Manifest` | |
| :--- | :--- |
| [`init`](#init) | a manifest with every array nil and every count 0 |
| [`dnit`](#dnit) | free every array a parsed manifest owns and zero the counts |

| Functions | |
| :--- | :--- |
| [`free_strarr`](#free_strarr) | |
| [`site_of`](#site_of) | the place `i` of a model's per-entry places, the zero place when none were recorded |
| [`id_of`](#id_of) | the id of `text`, interned in `itn`; a refused allocation is the failure |
| [`optional_id_of`](#optional_id_of) | `id_of` for an optional key's value: STR_NIL for the empty text an absent key reads as |

---

## NameList

```mach
pub rec NameList {
    items: *intern.StrId;
    sites: *base_source.Location;
    count: u32;
}
```

the names one of a profile's set keys lists, as written: the build checks
each against the catalog it names into

| Field | Type | |
| :--- | :--- | :--- |
| `items` | `*intern.StrId` | the names, nil when the key is absent or empty |
| `sites` | `*base_source.Location` | where each is written, parallel to `items` |
| `count` | `u32` | how many |

### names_dnit

```mach
pub fun names_dnit(alloc: *A.Allocator, l: *NameList)
```

release what a name list owns

## SimdMode

```mach
pub def SimdMode: u8
```

what happens when a vector operator has no packed instruction on the target,
decoded from `[profile.<name>].simd`

| Name | Value | |
| :--- | :--- | :--- |
| `SIMD_SCALARIZE` | `0` | `simd = "scalarize"`: emit the scalar expansion |
| `SIMD_REQUIRE` | `1` | `simd = "require"`: a missing packed instruction is a build error |

### simd_named

```mach
pub fun simd_named(name: str) opt[SimdMode]
```

the mode `name` names, as a manifest and the command line write it; none for
any other name

## TargetDef

```mach
pub rec TargetDef {
    name:          intern.StrId;
    env:           intern.StrId;
    arch:          intern.StrId;
    isa:           intern.StrId;
    sys:           intern.StrId;
    abi:           intern.StrId;
    of:            intern.StrId;
    platform:      intern.StrId;
    code_model:    intern.StrId;
    base:          u64;
    stack_reserve: u64;
    stack_commit:  u64;
    # where `platform`, `base`, `stack_reserve` and `stack_commit` were written, by mach.toml
    # or by `--set`, for a refusal of the value to name
    platform_from:      Origin;
    base_from:          Origin;
    stack_reserve_from: Origin;
    stack_commit_from:  Origin;
    ext:                [resolved.MAX_ADDED]intern.StrId;
    ext_count:          u32;
    runner:             *intern.StrId;
    runner_count:       u32;
    runner_at:          *base_source.Location;
    at:                 base_source.Location;
}
```

one `[target.<name>]` table as parsed; the string fields hold interned copies of
the manifest text, unvalidated against the target registry

| Field | Type | |
| :--- | :--- | :--- |
| `name` | `intern.StrId` | the table key; a portable identifier, never `native` |
| `env` | `intern.StrId` | the `env` key, or STR_NIL when absent; parse checks only that it is a non-empty string, the architecture validates the value later |
| `arch` | `intern.StrId` | the required `arch` key |
| `isa` | `intern.StrId` | the `isa` key, the instruction set every machine of the target has, or STR_NIL for the architecture's default |
| `sys` | `intern.StrId` | the required `sys` key |
| `abi` | `intern.StrId` | the required `abi` key |
| `of` | `intern.StrId` | the `of` object-format override, or STR_NIL when absent |
| `platform` | `intern.StrId` | the `platform` tag, or STR_NIL when absent |
| `code_model` | `intern.StrId` | the `code_model` key, mirl's psABI name of the code model, or STR_NIL for mirl's default; mirl refuses a model the target does not offer |
| `base` | `u64` | the `base` load address, 0 when absent |
| `stack_reserve` | `u64` | the `stack_reserve` byte count, 0 when absent |
| `stack_commit` | `u64` | the `stack_commit` byte count, 0 when absent |
| `ext` | `[resolved.MAX_ADDED]intern.StrId` | the `ext` names, each an identifier and listed once; the architecture refuses a name its vocabulary does not hold when the target resolves |
| `ext_count` | `u32` | how many of `ext` are set |
| `runner` | `*intern.StrId` | the `runner` argv templates the target's programs are launched through; nil when absent. freed by `dnit` |
| `runner_count` | `u32` | length of `runner` |
| `runner_at` | `*base_source.Location` | where each `runner` entry is written |
| `at` | `base_source.Location` | where the table's key is written |

### target_def

```mach
pub fun target_def() TargetDef
```

a target with every key a table may leave out at its default: no env, isa, object format,
platform, code model, extension or runner, and a zero base and stack. the caller sets the
name, arch, sys and abi

## Origin

```mach
pub rec Origin {
    named: str;
    at:    base_source.Location;
}
```

where a value a refusal names was written: in a manifest table, or by `--set` for one
invocation. a refusal of the value leads with `named` and points at `at`, so it sends the user
to the text that wrote it

| Field | Type | |
| :--- | :--- | :--- |
| `named` | `str` | the value as a refusal names it, `mach.toml: [target.x].base` or `--set base`, nil where the key was not written |
| `at` | `base_source.Location` | where the value is written, resolved in its manifest or in the `--set` text |

## ArtifactDef

```mach
pub rec ArtifactDef {
    name:         intern.StrId;
    kind:         Spelled;
    entry:        intern.StrId;
    out:          intern.StrId;
    targets:      *intern.StrId;
    target_count: u32;
    link:         *intern.StrId;
    link_count:   u32;
    need:         *intern.StrId;
    need_count:   u32;
    is_default:   bool;
    is_export:    bool;
    linkage:      intern.StrId;
    linkage_from: Origin;
    subsystem:    *mink_subsystem.Subsystem;
    # the key was written; an omitted key is the console default, which a
    # format without the field does not refuse
    subsystem_declared: bool;
    # where `subsystem` was written, by mach.toml or by `--set`
    subsystem_from: Origin;
    icon:           intern.StrId;
    app_manifest:   intern.StrId;
    sections:       *intern.StrId;
    section_count:  u32;
    sections_at:    *base_source.Location;
    size:           opt[u64];
    size_at:        base_source.Location;
    regions:        *RegionDef;
    region_count:   u32;
    at:             base_source.Location;
    kind_at:        base_source.Location;
    default_at:     base_source.Location;
    export_at:      base_source.Location;
    entry_at:       base_source.Location;
    out_at:         base_source.Location;
    targets_at:     base_source.Location;
    link_at:        *base_source.Location;
    need_at:        *base_source.Location;
}
```

one `[artifact.<name>]` table as parsed. the three arrays are owned by the
manifest and freed by `dnit`

| Field | Type | |
| :--- | :--- | :--- |
| `name` | `intern.StrId` | the table key; a portable identifier |
| `kind` | `Spelled` | the required `kind` key, as mink's output kinds spell it |
| `entry` | `intern.StrId` | the required `entry` key, a project-relative path under `[project].src` |
| `out` | `intern.StrId` | the `out` key, an unexpanded path template relative to the project root; `{project.work}` places the output under the work directory. STR_NIL for an object artifact, whose objects stay under obj/ |
| `targets` | `*intern.StrId` | the required `targets` array of declared target names or "*"; nil when empty |
| `target_count` | `u32` | length of `targets` |
| `link` | `*intern.StrId` | the `link` array of `[link.<name>]` names; nil when absent or empty |
| `link_count` | `u32` | length of `link` |
| `need` | `*intern.StrId` | category-qualified step and artifact names or globs; nil when absent or empty |
| `need_count` | `u32` | length of `need` |
| `is_default` | `bool` | `default = true`: the artifact is in the default selection; false when the key is absent |
| `is_export` | `bool` | `export = true`: the library a bare `use <id>;` binds and whose requirements travel to consumers; at most one artifact sets it |
| `linkage` | `intern.StrId` | the `linkage` key, the name of mink's executable output kind, which mink resolves against the target's host; nil when absent, which takes the target's default; `bin` artifacts only |
| `linkage_from` | `Origin` | where `linkage` was written |
| `subsystem` | `*mink_subsystem.Subsystem` | the `subsystem` key as mink spells the PE subsystem, console when absent |
| `icon` | `intern.StrId` | the `icon` path, or STR_NIL when absent; `bin` artifacts only |
| `app_manifest` | `intern.StrId` | the `manifest` path, or STR_NIL when absent; `bin` artifacts only |
| `sections` | `*intern.StrId` | the `sections` key, the output sections the link places ahead of the rest, in order; nil when absent |
| `section_count` | `u32` | length of `sections` |
| `sections_at` | `*base_source.Location` | where each `sections` entry is written, parallel to `sections` |
| `size` | `opt[u64]` | the `size` key, the most bytes the image loads from its base, none when absent |
| `size_at` | `base_source.Location` | where its `size` value is written, the zero place when absent |
| `regions` | `*RegionDef` | the `[artifact.X.region.<name>]` tables, the memory regions the link lays sections into; nil when none is written |
| `region_count` | `u32` | length of `regions` |
| `at` | `base_source.Location` | where the table's key is written |
| `kind_at` | `base_source.Location` | where its `kind` value is written |
| `default_at` | `base_source.Location` | where its `default` value is written, the zero place when absent |
| `export_at` | `base_source.Location` | where its `export` value is written, the zero place when absent |
| `entry_at` | `base_source.Location` | where its `entry` value is written |
| `out_at` | `base_source.Location` | where its `out` value is written |
| `targets_at` | `base_source.Location` | where its `targets` value is written |
| `link_at` | `*base_source.Location` | where each `link` entry is written, parallel to `link` |
| `need_at` | `*base_source.Location` | where each `need` entry is written, parallel to `need` |

### artifact_def

```mach
pub fun artifact_def() ArtifactDef
```

an artifact with every key a table may leave out at its default: no list, linkage, icon,
manifest, size or region, the console subsystem, neither default nor exported. the caller
sets the name, kind, entry, out and targets

## RegionDef

```mach
pub rec RegionDef {
    name:       intern.StrId;
    origin:     u64;
    length:     u64;
    access:     u8;
    run:        *intern.StrId;
    run_count:  u32;
    load:       *intern.StrId;
    load_count: u32;
    at:         base_source.Location;
}
```

one `[artifact.X.region.<name>]` memory region as parsed, laid out as a linker script's
MEMORY lays one

| Field | Type | |
| :--- | :--- | :--- |
| `name` | `intern.StrId` | the table key; a portable identifier |
| `origin` | `u64` | the required `origin` key, the address it starts at |
| `length` | `u64` | the required `length` key, the bytes it spans |
| `access` | `u8` | the required `access` key, of mink's layout READABLE, WRITABLE and EXECUTABLE, written as the letters r, w and x |
| `run` | `*intern.StrId` | the `run` key, the output sections that run in it; nil when absent |
| `run_count` | `u32` | length of `run` |
| `load` | `*intern.StrId` | the `load` key, the output sections whose bytes load in it and run in another region; nil when absent |
| `load_count` | `u32` | length of `load` |
| `at` | `base_source.Location` | where the table's key is written |

## ProfileDef

```mach
pub rec ProfileDef {
    name:         intern.StrId;
    is_default:   bool;
    optimize:     bool;
    relax:        NameList;
    linker_relax: bool;
    debug:        bool;
    simd:         SimdMode;
    allow:        diagnostic_kind.KindSet;
    at:           base_source.Location;
    default_at:   base_source.Location;
}
```

one `[profile.<name>]` compilation policy as parsed. every manifest declares at
least one

| Field | Type | |
| :--- | :--- | :--- |
| `name` | `intern.StrId` | the table key; a portable identifier |
| `is_default` | `bool` | `default = true`; false when the key is absent. more than one default is a parse error |
| `optimize` | `bool` | the required `optimize` key: whether the release passes run |
| `relax` | `NameList` | the optional `relax` key: the departures from exact semantics the build permits |
| `linker_relax` | `bool` | the optional `linker_relax` key: whether the linker may relax the code, true when the key is absent |
| `debug` | `bool` | the required `debug` key |
| `simd` | `SimdMode` | the required `simd` key |
| `allow` | `diagnostic_kind.KindSet` | the optional `allow` key: the warning kinds the profile silences, empty when the key is absent |
| `at` | `base_source.Location` | where the table's key is written |
| `default_at` | `base_source.Location` | where its `default` value is written, the zero place when absent |

### profile_def

```mach
pub fun profile_def() ProfileDef
```

a profile with every key a table may leave out at its default: not the default, no relax
and no allow, the linker free to relax, and vectors scalarized. the caller sets the name

## DepDef

```mach
pub rec DepDef {
    name:    intern.StrId;
    git:     intern.StrId;
    path:    intern.StrId;
    ref:     intern.StrId;
    version: intern.StrId;
}
```

one `[dep.<name>]` table as parsed. exactly one of `git` and `path` is set

| Field | Type | |
| :--- | :--- | :--- |
| `name` | `intern.StrId` | the table key; a portable identifier |
| `git` | `intern.StrId` | the `git` clone URL, or STR_NIL for a path dependency |
| `path` | `intern.StrId` | the `path` directory, or STR_NIL for a git dependency |
| `ref` | `intern.StrId` | the `ref` key, required with `git` and rejected with `path`; one of `branch/<name>`, `tag/<name>`, `commit/<40 or 64 lowercase hex digits>`. STR_NIL for a path dependency a git dependency names exactly one of `ref` (an exact or branch selector) and `version` (a release range); STR_NIL marks the one it does not |

## LinkSource

```mach
pub def LinkSource: u8
```

where a `[link.<name>]` entry is resolved from, decoded from its `source` key

| Name | Value | |
| :--- | :--- | :--- |
| `LINK_LOCAL` | `0` | `source = "local"`: a file at a project-relative `path` |
| `LINK_SYSTEM` | `1` | `source = "system"`: a system library found by `name` |
| `LINK_FRAMEWORK` | `2` | `source = "framework"`: a macOS framework found by `name` |

## LinkDef

```mach
pub rec LinkDef {
    name:               intern.StrId;
    source:             LinkSource;
    lib_name:           intern.StrId;
    path:               intern.StrId;
    symbols:            *intern.StrId;
    symbol_count:       u32;
    sys:                *intern.StrId;
    sys_count:          u32;
    arch:               *intern.StrId;
    arch_count:         u32;
    abi:                *intern.StrId;
    abi_count:          u32;
    export:             bool;
    include_referenced: bool;
    kind:               input.Kind;
    versions:           LinkVersions;
    at:                 base_source.Location;
    path_at:            base_source.Location;
}
```

one `[link.<name>]` table as parsed. the four arrays are owned by the manifest
and freed by `dnit`

| Field | Type | |
| :--- | :--- | :--- |
| `name` | `intern.StrId` | the table key: the entry's identity, which a `#[library]` attribution names |
| `source` | `LinkSource` | the required `source` key |
| `lib_name` | `intern.StrId` | the `name` key, required for system and framework sources and rejected for local; STR_NIL for local |
| `path` | `intern.StrId` | the `path` key, required for a local source and rejected otherwise; a project-relative path template, STR_NIL for non-local |
| `symbols` | `*intern.StrId` | the `symbols` array; every entry non-empty and unique; nil when absent |
| `symbol_count` | `u32` | length of `symbols` |
| `sys` | `*intern.StrId` | the required `sys` filter axis, each value canonical or "*"; nil for `[]`, which matches no target |
| `sys_count` | `u32` | length of `sys` |
| `arch` | `*intern.StrId` | the `arch` filter axis, as `sys` |
| `arch_count` | `u32` | length of `arch` |
| `abi` | `*intern.StrId` | the `abi` filter axis, as `sys` |
| `abi_count` | `u32` | length of `abi` |
| `export` | `bool` | the `export` key; false when absent |
| `include_referenced` | `bool` | the `include` key: false for "always" (the default), true for "referenced" |
| `kind` | `input.Kind` | the `kind` key, written only on a system source that names a library by a bare name; SHARED when absent |
| `versions` | `LinkVersions` | the `current_version` and `compatibility_version` keys, written only on a system source of the shared kind; unstated when absent |
| `at` | `base_source.Location` | where the table's key is written |
| `path_at` | `base_source.Location` | where the `path` value is written, the zero place when absent |

### link_def

```mach
pub fun link_def() LinkDef
```

a link with every key a table may leave out at its default: no symbol or filter, not
exported, always included, and a shared library. the caller sets the name, source and the
name or path its source takes

## LinkVersions

```mach
pub rec LinkVersions {
    stated:  bool;
    current: u32;
    compat:  u32;
}
```

the versions a shared library is stated at, as a Mach-O load command names them, each
encoded as the command holds it: the major version in the high 16 bits, then the minor
and the patch in 8 bits each

| Field | Type | |
| :--- | :--- | :--- |
| `stated` | `bool` | the entry states them; when it does not, both are zero |
| `current` | `u32` | the version the image is linked against |
| `compat` | `u32` | the oldest version the image may load instead |

## StepDef

```mach
pub rec StepDef {
    name:         intern.StrId;
    argv:         *intern.StrId;
    argv_count:   u32;
    env_keys:     *intern.StrId;
    env_values:   *intern.StrId;
    env_count:    u32;
    env_in:       *intern.StrId;
    env_in_count: u32;
    in:           *intern.StrId;
    in_count:     u32;
    out:          *intern.StrId;
    out_count:    u32;
    need:         *intern.StrId;
    need_count:   u32;
    timeout:      chrono_duration.Duration;
    at:           base_source.Location;
    argv_at:      *base_source.Location;
    env_at:       *base_source.Location;
    env_in_at:    *base_source.Location;
    in_at:        *base_source.Location;
    out_at:       *base_source.Location;
    need_at:      *base_source.Location;
}
```

one `[step.<name>]` table as parsed. the seven arrays are owned by the manifest
and freed by `dnit`. the removed keys `cmd` and `shell` are rejected by name

| Field | Type | |
| :--- | :--- | :--- |
| `name` | `intern.StrId` | the table key; an identifier |
| `argv` | `*intern.StrId` | the required, non-empty `argv` array; `argv[0]` is non-empty |
| `argv_count` | `u32` | length of `argv` |
| `env_keys` | `*intern.StrId` | keys of the `env` table, sorted case-insensitively then bytewise; every key non-empty, free of '=', and unique ignoring case. nil when absent |
| `env_values` | `*intern.StrId` | the string value of each key, parallel to `env_keys` |
| `env_count` | `u32` | length of both `env` arrays |
| `env_in` | `*intern.StrId` | the `env_in` array: names of inherited variables the step reads, each non-empty, free of '=', and distinct by host name identity. nil when absent |
| `env_in_count` | `u32` | length of `env_in` |
| `in` | `*intern.StrId` | the required `in` array of project-relative paths |
| `in_count` | `u32` | length of `in` |
| `out` | `*intern.StrId` | the required `out` array of project-relative paths; a '*' is rejected |
| `out_count` | `u32` | length of `out` |
| `need` | `*intern.StrId` | the `need` array of step-qualified names or globs; nil when absent or empty |
| `need_count` | `u32` | length of `need` |
| `timeout` | `chrono_duration.Duration` | the `timeout` key, a duration such as "30s"; 0 when absent |
| `at` | `base_source.Location` | where the table's key is written |
| `argv_at` | `*base_source.Location` | where each `argv` entry is written, parallel to `argv` |
| `env_at` | `*base_source.Location` | where each `env` value is written, parallel to `env_values` |
| `env_in_at` | `*base_source.Location` | where each `env_in` entry is written, parallel to `env_in` |
| `in_at` | `*base_source.Location` | where each `in` entry is written, parallel to `in` |
| `out_at` | `*base_source.Location` | where each `out` entry is written, parallel to `out` |
| `need_at` | `*base_source.Location` | where each `need` entry is written, parallel to `need` |

### step_dnit

```mach
pub fun step_dnit(alloc: *A.Allocator, s: *StepDef)
```

free every array a step owns, its entries and the places they are written at;
the step itself is its owner's

## SourceRoot

```mach
pub rec SourceRoot {
    name: intern.StrId;
    dir:  intern.StrId;
}
```

a module prefix an entry build binds to a directory with `-s name=dir`: a package with
no manifest, whose modules are the files under the directory

| Field | Type | |
| :--- | :--- | :--- |
| `name` | `intern.StrId` | the prefix, a portable identifier |
| `dir` | `intern.StrId` | the directory, as the command line wrote it |

## Manifest

```mach
pub rec Manifest {
    id:      intern.StrId;
    version: intern.StrId;
    # the compiler range [project].mach states, STR_NIL when the key is absent
    mach: intern.StrId;
    # where the `[project]` table and its `mach` value are written, each place's
    # path the interner's; the zero place when not written
    project_at:     base_source.Location;
    mach_at:        base_source.Location;
    src:            intern.StrId;
    work_tmpl:      intern.StrId;
    work_at:        base_source.Location;
    targets:        *TargetDef;
    target_count:   u32;
    native_target:  *TargetDef;
    artifacts:      *ArtifactDef;
    artifact_count: u32;
    profiles:       *ProfileDef;
    profile_count:  u32;
    deps:           *DepDef;
    dep_count:      u32;
    links:          *LinkDef;
    link_count:     u32;
    steps:          *StepDef;
    step_count:     u32;
    sources:        *SourceRoot;
    source_count:   u32;
    # the session input the manifest was read from and the revision it had,
    # FILE_NIL when no session input states it, as for an entry build or text no session holds
    source:          base_source.FileId;
    source_revision: query.Revision;
}
```

a parsed `mach.toml`. every string is an interned id; every array is owned and
freed by `dnit`, so the counts are the allocation extents. every place a
definition records is resolved in the file the manifest was read from, its
path the interner's, so a refusal made from the model points at the entry
that caused it

| Field | Type | |
| :--- | :--- | :--- |
| `id` | `intern.StrId` | the required `[project].id`, a portable identifier |
| `version` | `intern.StrId` | the required `[project].version`, carried verbatim |
| `src` | `intern.StrId` | the required `[project].src`, a project-relative path |
| `work_tmpl` | `intern.StrId` | the required `[project].work`, an unexpanded path template for the work directory |
| `work_at` | `base_source.Location` | where `[project].work` is written |
| `targets` | `*TargetDef` | the `[target.*]` tables, nil when none |
| `target_count` | `u32` | length of `targets` |
| `native_target` | `*TargetDef` | the owned synthesized host definition when no targets are declared |
| `artifacts` | `*ArtifactDef` | the `[artifact.*]` tables, nil when none |
| `artifact_count` | `u32` | length of `artifacts` |
| `profiles` | `*ProfileDef` | the `[profile.*]` tables, at least one |
| `profile_count` | `u32` | length of `profiles` |
| `deps` | `*DepDef` | the `[dep.*]` tables, nil when none |
| `dep_count` | `u32` | length of `deps` |
| `links` | `*LinkDef` | the `[link.*]` tables, nil when none |
| `link_count` | `u32` | length of `links` |
| `steps` | `*StepDef` | the `[step.*]` tables, nil when none |
| `step_count` | `u32` | length of `steps` |
| `sources` | `*SourceRoot` | the source roots an entry build binds with `-s`, each a package with no manifest; nil for a manifest read from mach.toml, which states none |
| `source_count` | `u32` | length of `sources` |

### init

```mach
pub fun init() Manifest
```

a manifest with every array nil and every count 0. the scalar ids are left
unset, so only `dnit` is safe on the result

| Parameter | |
| :--- | :--- |
| **returns** | the empty manifest |

### dnit

```mach
pub fun dnit(m: *Manifest, alloc: *A.Allocator)
```

free every array a parsed manifest owns and zero the counts. safe on the
result of `init` and on a manifest already released

| Parameter | |
| :--- | :--- |
| `m` | the manifest |
| `alloc` | the allocator `parse` was given |

## Functions

### free_strarr

```mach
pub fun free_strarr(alloc: *A.Allocator, items: *intern.StrId, count: u32)
```

### site_of

```mach
pub fun site_of(sites: *base_source.Location, i: u32) base_source.Location
```

the place `i` of a model's per-entry places, the zero place when none were recorded

### id_of

```mach
pub fun id_of(itn: *intern.Interner, text: str) res[intern.StrId, fail.Fail]
```

the id of `text`, interned in `itn`; a refused allocation is the failure

### optional_id_of

```mach
pub fun optional_id_of(itn: *intern.Interner, text: str) res[intern.StrId, fail.Fail]
```

`id_of` for an optional key's value: STR_NIL for the empty text an absent key reads as

