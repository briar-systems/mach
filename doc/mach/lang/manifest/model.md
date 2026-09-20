# mach.lang.manifest.model

## def MOpt

```mach
pub def MOpt: u8
```

optimisation level of a profile, decoded from `[profile.<name>].opt`

## val MOPT_DEBUG

```mach
pub val MOPT_DEBUG: MOpt = 1
```

`found = 0`: the debug pipeline

## val MOPT_RELEASE

```mach
pub val MOPT_RELEASE: MOpt = 2
```

`found = 1` or `found = 2`: the release pipeline; both integers map here

## def SimdMode

```mach
pub def SimdMode: u8
```

what happens when a vector operator has no packed instruction on the target,
decoded from `[profile.<name>].simd`

## val SIMD_SCALARIZE

```mach
pub val SIMD_SCALARIZE: SimdMode = 0
```

`simd = "scalarize"`: emit the scalar expansion; the default for a dependency profile

## val SIMD_REQUIRE

```mach
pub val SIMD_REQUIRE: SimdMode = 1
```

`simd = "require"`: a missing packed instruction is a build error

## rec TargetDef

```mach
pub rec TargetDef;
```

one `[target.<name>]` table as parsed; the string fields hold interned copies of
the manifest text, unvalidated against the target registry

name: the table key; a portable identifier, never `native`
is_default: `default = true`; false when the key is absent
env: the `env` key, or STR_NIL when absent; parse checks only that it is a
                 non-empty string, the isa validates the value later
isa: the required `isa` key
os: the required `os` key
abi: the required `abi` key
of: the `of` object-format override, or STR_NIL when absent
platform: the `platform` tag, or STR_NIL when absent
base: the `base` load address, 0 when absent
stack_reserve: the `stack_reserve` byte count, 0 when absent
stack_commit: the `stack_commit` byte count, 0 when absent
extensions: the `extensions` names, each an identifier and listed once; the isa
                 refuses a name its vocabulary does not hold when the target resolves
extension_count: how many of `extensions` are set

## rec ArtifactDef

```mach
pub rec ArtifactDef;
```

one `[artifact.<name>]` table as parsed. the three arrays are owned by the
manifest and freed by `dnit`

name: the table key; a portable identifier
kind: the required `kind` key, one of "bin", "static", "shared"
entry: the required `entry` key, a project-relative path under `[project].src`
out: the required `out` key, an unexpanded path template relative to the
              expanded `[project].out`
targets: the required `targets` array of declared target names or "*"; nil when empty
target_count: length of `targets`
link: the `link` array of `[link.<name>]` names; required at the root (`[]` for
              none), optional in a dependency; nil when empty
link_count: length of `link`
need: category-qualified step and artifact names or globs; required at the root
              (`[]` for none), optional in a dependency; nil when empty
need_count: length of `need`
is_lib: true unless `kind = "bin"`
is_default: `default = true`; false when the key is absent
subsystem: the `subsystem` key, console when absent; only "console" and "gui" parse
icon: the `icon` path, or STR_NIL when absent; `bin` artifacts only
app_manifest: the `manifest` path, or STR_NIL when absent; `bin` artifacts only

## rec ProfileDef

```mach
pub rec ProfileDef;
```

one explicit `[profile.<name>]` compilation policy as parsed, or one of the two
profiles synthesized for a dependency manifest that declares none: `debug`
(opt 0, debug true, vectorize false, default) and `release` (opt 2, debug
false, vectorize true). a root manifest declares at least one

name: the table key; a portable identifier
is_default: `default = true`; false when the key is absent. more than one
               default across declared profiles is a parse error at the root
opt: the required `opt` key
debug: the required `debug` key
simd: the required `simd` key
vectorize: the required `vectorize` key
float_reassoc: the required `float_reassoc` key

## rec DepDef

```mach
pub rec DepDef;
```

one `[dep.<name>]` table as parsed. exactly one of `git` and `path` is set

name: the table key; a portable identifier
git: the `git` clone URL, or STR_NIL for a path dependency
path: the `path` directory, or STR_NIL for a git dependency
ref: the `ref` key, required with `git` and rejected with `path`; one of
      `branch/<name>`, `tag/<name>`, `commit/<40 or 64 lowercase hex digits>`.
      STR_NIL for a path dependency
a git dependency names exactly one of `ref` (an exact or branch selector) and `version`
(a release range, #3496); STR_NIL marks the one it does not

## def LinkSource

```mach
pub def LinkSource: u8
```

where a `[link.<name>]` entry is resolved from, decoded from its `source` key

## val LINK_LOCAL

```mach
pub val LINK_LOCAL: LinkSource = 0
```

`source = "local"`: a file at a project-relative `path`

## val LINK_SYSTEM

```mach
pub val LINK_SYSTEM: LinkSource = 1
```

`source = "system"`: a system library found by `name`

## val LINK_FRAMEWORK

```mach
pub val LINK_FRAMEWORK: LinkSource = 2
```

`source = "framework"`: a macOS framework found by `name`

## rec LinkDef

```mach
pub rec LinkDef;
```

one `[link.<name>]` table as parsed. the four arrays are owned by the manifest
and freed by `dnit`

name: the table key; a portable identifier
source: the required `source` key
library: the `library` key; defaults to `name` when absent. an empty string is an error
lib_name: the `name` key, required for system and framework sources and rejected
                    for local; STR_NIL for local
path: the `path` key, required for a local source and rejected otherwise; a
                    project-relative path template, STR_NIL for non-local
symbols: the `symbols` array; every entry non-empty and unique; nil when absent
symbol_count: length of `symbols`
os: the `os` filter axis: a string, an array of strings, or "*"; nil when absent or `[]`
os_count: length of `os`
os_present: whether the `os` key was written. absent matches every target, `[]`
                    matches none. required at the root, where every value must be canonical or "*"
isa: the `isa` filter axis, as `os`
isa_count: length of `isa`
isa_present: as `os_present`
abi: the `abi` filter axis, as `os`
abi_count: length of `abi`
abi_present: as `os_present`
export: the `export` key, required at the root; false when absent in a dependency
include_referenced: the `include` key: false for "always" (the default), true for "referenced"

## rec StepDef

```mach
pub rec StepDef;
```

one `[step.<name>]` table as parsed. the six arrays are owned by the manifest
and freed by `dnit`. the removed keys `cmd` and `shell` are rejected by name

name: the table key; an identifier
argv: the required, non-empty `argv` array; `argv[0]` is non-empty
argv_count: length of `argv`
env_keys: keys of the `env` table, sorted case-insensitively then bytewise;
                 every key non-empty, free of '=', and unique ignoring case. nil when absent
env_values: the string value of each key, parallel to `env_keys`
env_count: length of both `env` arrays
in: the required `in` array of project-relative paths
in_count: length of `in`
out: the required `out` array of project-relative paths; a '*' is rejected
out_count: length of `out`
need: the `need` array of step-qualified names or globs; required at the root (`[]` for none),
                 optional in a dependency
need_count: length of `need`
timeout_seconds: the `timeout_seconds` key, an integer of at least 1; 0 when absent

## rec Manifest

```mach
pub rec Manifest;
```

a parsed `mach.toml`. every string is an interned id; every array is owned and
freed by `dnit`, so the counts are the allocation extents

id: the required `[project].id`, a portable identifier
version: the required `[project].version`, carried verbatim
src: the required `[project].src`, a project-relative path
out_tmpl: the required `[project].out`, an unexpanded path template
default_target: always the interned string "native"
targets: the `[target.*]` tables, nil when none
target_count: length of `targets`
native_target: the owned synthesized host definition when no targets are declared
artifacts: the `[artifact.*]` tables, nil when none
artifact_count: length of `artifacts`
profiles: the `[profile.*]` tables; at the root at least one, in a dependency
                the two synthesized ones when none are declared
profile_count: length of `profiles`
deps: the `[dep.*]` tables, nil when none
dep_count: length of `deps`
links: the `[link.*]` tables, nil when none
link_count: length of `links`
steps: the `[step.*]` tables, nil when none
step_count: length of `steps`

## fun empty

```mach
pub fun empty() Manifest;
```

a manifest with every array nil and every count 0. the scalar ids are left
unset, so only `dnit` is safe on the result

ret: the empty manifest

## fun dnit

```mach
pub fun dnit(m: *Manifest, alloc: *A.Allocator);
```

free every array a parsed manifest owns and zero the counts. safe on the
result of `empty` and on a manifest already released

m: the manifest
alloc: the allocator `parse` was given

