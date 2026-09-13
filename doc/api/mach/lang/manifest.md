# mach.lang.manifest

## val MANIFEST_FILE

```mach
pub val MANIFEST_FILE: str = "mach.toml"
```

the file name of a project manifest, looked up in a project root

## fun validate_source_bytes

```mach
pub fun validate_source_bytes(src: *u8, n: usize) err[outcome.Fail];
```

reject manifest text that would carry a NUL byte into the TOML parser

src: manifest bytes, not required to be NUL-terminated
n: number of bytes to scan
ret: ok when clean; err "mach.toml: source cannot contain NUL bytes" for a raw
     0 byte anywhere, or "mach.toml: strings cannot contain NUL bytes" for a
     `\u0000` or `\U00000000` escape inside a basic (double-quoted) string,
     single-line or multi-line. comments and literal (single-quoted) strings are
     skipped without escape checks

## fun validate_source_text

```mach
pub fun validate_source_text(src: str) err[outcome.Fail];
```

`validate_source_bytes` over a NUL-terminated string

src: manifest text
ret: as `validate_source_bytes`

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

## def LibKind

```mach
pub def LibKind: u8
```

the library shape of a build unit, decoded from `[artifact.<name>].kind`

## val LIBKIND_STATIC

```mach
pub val LIBKIND_STATIC: LibKind = 0
```

`kind = "static"`; also the value a `bin` unit carries, so check `is_lib` first

## val LIBKIND_SHARED

```mach
pub val LIBKIND_SHARED: LibKind = 1
```

`kind = "shared"`

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

## rec LinkRequirement

```mach
pub rec LinkRequirement;
```

a `[link.<name>]` entry resolved for one target, as the linker consumes it.
the symbol array is owned; release it with `free_link_claims`

source: copied from the link
text: for a local source, the `path` expanded and interned; otherwise the link's `name`
library: the link's logical `library` name
symbols: an owned copy of the link's `symbols`, or nil when it has none
symbol_count: length of `symbols`
include_referenced: copied from the link

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

## fun canonical_module

```mach
pub fun canonical_module(m: *Manifest) intern.StrId;
```

the public entry a bare `use <id>;` binds: the one `entry` shared by every
library artifact marked `default = true`. a `bin` never publishes an entry

m: a parsed manifest
ret: the shared entry id; STR_NIL when no library artifact is a default or two
     default library artifacts name different entries

## rec Selection

```mach
pub rec Selection;
```

what the caller asked to build: a target, a profile, and optionally an
artifact. empty strings mean "not selected"

target: a declared target name, or "" to resolve `native` (or the artifact's pinned target)
profile: a declared profile name, or "" for the default profile
artifact: an artifact name, read only when `has_artifact`
want_lib: with `has_artifact`, whether the named artifact must be a library
              (true) or a `bin` (false); a name with the wrong shape does not match
has_artifact: whether `artifact` and `want_lib` are set

## rec BuildUnit

```mach
pub rec BuildUnit;
```

one resolved build cell: a target, a profile, and an artifact with every path
expanded. produced by `resolve_build_unit`; `libs` is owned by the unit

target: the resolved target; points into the manifest, except for the host
               target synthesized when the manifest declares no targets
artifact: the resolved artifact, pointing into the manifest
target_name: `target.name`
profile_name: the resolved profile's name
entry: the artifact's `entry`
bin_path: the artifact's output: the expanded `[project].out` joined with the
               expanded artifact `out`
obj_root: `<expanded project out>/obj`
ir_root: `<expanded project out>/ir`
asm_root: `<expanded project out>/asm`
test_root: `<expanded project out>/test/{name}`, with `{name}` left literal for
               the test runner to fill
opt: from the resolved profile
debug: from the resolved profile
simd: from the resolved profile
vectorize: from the resolved profile
float_reassoc: from the resolved profile
is_lib: the artifact's `is_lib`
kind: LIBKIND_SHARED for `kind = "shared"`, LIBKIND_STATIC otherwise, including for a `bin`
libs: the artifact's `link` entries that match the target, resolved in `link`
               order and sized exactly; a `link` name matching no `[link.*]` table is skipped
lib_count: length of `libs`
warned: `native` matched no declared target for the host and a declared target
               was chosen anyway; the driver prints the warning

## rec ResolvedTarget

```mach
pub rec ResolvedTarget;
```

the outcome of target resolution for a selection

target: the chosen target; points into the manifest unless synthesized for a
        manifest with no `[target.*]`, in which case it is allocated and never freed by `dnit`
warned: `native` matched no host target and a declared target was chosen anyway
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

## rec TmplVars

```mach
pub rec TmplVars;
```

the values a path template expands with; see `expand`

target: `{target.name}`
isa: `{target.isa}`
os: `{target.os}`
abi: `{target.abi}`
profile: `{profile.name}`
format: the target's explicit `of` override, "" when the format is derived
artifact_suffix: `{artifact.suffix}`, set by `expand_artifact_output` for the
                 artifact being named; nil everywhere else, where the variable is refused
reqs: the artifacts this expansion may name as `{artifact.<id>.out}`; nil when none
req_count: length of `reqs`

## rec ArtifactReq

```mach
pub rec ArtifactReq;
```

one artifact another artifact requires, as `{artifact.<id>.out}` resolves it

name: the required artifact's name
out: its final output path when it builds for exactly one target here, or ""
      when it builds for several, which makes `{artifact.<id>.out}` an error

## fun host_tuple

```mach
pub fun host_tuple(alloc: *A.Allocator) str;
```

the host as "<isa>-<os>", e.g. "x86_64-linux"

alloc: owns the returned string
ret: the formatted tuple; on allocation failure the literal "out of memory",
       which must not be freed

## fun is_valid_id

```mach
pub fun is_valid_id(s: str) bool;
```

whether `s` is a portable identifier: non-empty, only ASCII letters, digits,
'_' and '-'. the rule for project, target, artifact, profile, dependency,
link and step names

s: the candidate
ret: true when every byte is allowed

## fun removed_key_msg

```mach
pub fun removed_key_msg(alloc: *A.Allocator, key: str, label: str) str;
```

## fun is_project_path

```mach
pub fun is_project_path(value: str) bool;
```

whether `value` is a canonical strict descendant of the project root: non-empty,
'/'-separated, no '\', not absolute, no drive letter, no empty component, no
'.' or '..' component

value: the candidate path
ret: true when every rule holds

## fun empty

```mach
pub fun empty() Manifest;
```

a manifest with every array nil and every count 0. the scalar ids are left
unset, so only `dnit` is safe on the result

ret: the empty manifest

## fun parse

```mach
pub fun parse(alloc: *A.Allocator, itn: *intern.Interner, t: *toml.Table, as_root: bool) res[Manifest, outcome.Fail];
```

build a `Manifest` from a parsed TOML document. accepted root tables are
`[project]`, `[target.*]`, `[artifact.*]`, `[profile.*]`, `[dep.*]`,
`[link.*]` and `[step.*]`; any other root key, and any key a table does not
define, is an error naming it. on any error every array allocated so far is
freed before returning

alloc: owns the manifest's arrays
itn: receives every string of the manifest
t: the TOML document
as_root: true for the project being built, false for a dependency's manifest.
         the root form requires at least one `[profile.*]` table, `[artifact].link`
         and `need`, `[link].os`, `isa`, `abi` and `export`, `[step].need`, requires
         link filter values to be canonical or "*", and rejects more than one
         `default = true` profile. the dependency form treats each of those keys as
         optional. a declared `[profile.*]` table requires `opt`, `debug`, `simd`,
         `vectorize` and `float_reassoc` in both forms. the key set is closed for
         both forms: a key is read or refused. the five 4.26.x keys never read
         (`[project].name`, `description`, `mach`, `[profile].emit_ir`, `emit_asm`)
         are refused with a message naming their removal
ret: the manifest, or the first error as a "mach.toml: ..." message. the
         `[project]` table and its `id`, `version`, `src` and `out` are always
         required; `src`, `out`, artifact `entry` and `out`, local link `path` and
         step `in` and `out` entries must satisfy `is_project_path`; every table name
         must satisfy `is_valid_id`; `[target.native]` and `[dep.<x>].version` are
         reserved; a `[step]` with `cmd` or `shell` is rejected by name; `need`
         entries are checked by `validate_needs`. `[profile.*]` absent or empty is
         an error at the root and synthesizes `debug` and `release` in a dependency

## val MISSING_PROFILE_MESSAGE

```mach
pub val MISSING_PROFILE_MESSAGE: str = "mach.toml: no [profile.<name>] table is declared
```

## fun dnit

```mach
pub fun dnit(m: *Manifest, alloc: *A.Allocator);
```

free every array a parsed manifest owns and zero the counts. safe on the
result of `empty` and on a manifest already released

m: the manifest
alloc: the allocator `parse` was given

## fun free_link_claims

```mach
pub fun free_link_claims(alloc: *A.Allocator, items: *LinkRequirement, count: u32);
```

free the symbol arrays of `count` requirements and nil them. the `items` array
itself is not freed

alloc: the allocator the claims were made from
items: the requirements; nil is accepted
count: how many entries to release

## fun finish_link_requirements

```mach
pub fun finish_link_requirements(alloc: *A.Allocator, items: *LinkRequirement,
capacity: u32, count: u32,
out_items: **LinkRequirement,
out_count: *u32) err[outcome.Fail];
```

turn a partially filled requirement buffer into an exactly sized result, taking
ownership of the buffer

alloc: the buffer's allocator
items: a buffer of `capacity` entries with the first `count` filled
capacity: the buffer's allocated extent
count: the filled prefix
out_items: receives the exact array, or nil when `count` is 0 or on error
out_count: receives `count`, or 0 when it is 0 or on error
ret: ok; when `count` is 0 the buffer is freed. on a failed shrink the filled
           claims and the whole buffer are freed and the allocator's error is returned

## fun merge_link_claims

```mach
pub fun merge_link_claims(alloc: *A.Allocator, dst: *LinkRequirement,
src: *LinkRequirement) err[outcome.Fail];
```

add every symbol of `src` that `dst` lacks to `dst`, keeping `dst` order first
and the new entries in `src` order. `src` is not modified

alloc: the allocator of `dst.symbols`; the old array is freed and replaced
dst: the requirement that grows
src: the requirement whose symbols are added
ret: ok, without allocating, when nothing is new; err on a symbol count
       overflow or allocation failure, leaving `dst` unchanged

## fun tmpl_vars_of

```mach
pub fun tmpl_vars_of(itn: *intern.Interner, t: *TargetDef, pn: str) TmplVars;
```

template variables for one target and profile, with no artifact requirements

itn: resolves the target's interned strings
t: the target
pn: the profile name
ret: the variables; `reqs` is nil

## tag TemplateError

```mach
pub tag TemplateError: u8 {
    internal: str;
    rejected: str;
}
```

a template that could not be expanded: rejected names the template's own
fault in a message the caller's allocator owns (released by
template_error_dnit); internal carries the compiler's static text

## fun template_internal

```mach
pub fun template_internal(message: str) TemplateError;
```

## fun template_error_dnit

```mach
pub fun template_error_dnit(alloc: *A.Allocator, e: TemplateError);
```

## fun template_rejection

```mach
pub fun template_rejection(alloc: *A.Allocator, field: str, tmpl: str, reason: str) TemplateError;
```

## fun template_text

```mach
pub fun template_text(e: TemplateError) str;
```

the message either case carries

## fun expand_artifact_path

```mach
pub fun expand_artifact_path(alloc: *A.Allocator, tmpl: str, reqs: *ArtifactReq, req_count: u32,
field: str) res[str, TemplateError];
```

## fun tmpl_vars_with_reqs

```mach
pub fun tmpl_vars_with_reqs(itn: *intern.Interner, t: *TargetDef, pn: str,
reqs: *ArtifactReq, req_count: u32) TmplVars;
```

`tmpl_vars_of` with artifact requirements attached

itn: resolves the target's interned strings
t: the target
pn: the profile name
reqs: the requirements `{artifact.<id>.out}` may name
req_count: length of `reqs`
ret: the variables

## fun expand

```mach
pub fun expand(alloc: *A.Allocator, tmpl: str, project_out: str,
v: *TmplVars) res[str, outcome.Fail];
```

expand a path template. the placeholders are `{project.out}`, `{target.name}`,
`{target.isa}`, `{target.os}`, `{target.abi}`, `{profile.name}` and
`{artifact.<id>.out}`; nothing else is accepted

alloc: owns the returned string
tmpl: the template
project_out: the expanded `[project].out`, or "" when expanding `[project].out`
             itself, in which case `{project.out}` is an error
v: the values
ret: the expanded string; err on an unterminated '{', an unknown placeholder,
             an artifact not in `v.reqs`, or an artifact whose output is ambiguous

## fun expand_step_value

```mach
pub fun expand_step_value(alloc: *A.Allocator, tmpl: str, project_out: str,
v: *TmplVars) res[str, outcome.Fail];
```

expand a build step `argv` or `env` value. known placeholders expand as in
`expand`; a brace pair that is not a placeholder is copied verbatim, unless it
starts with `project`, `target`, `profile` or `artifact`, which is an error

alloc: owns the returned string
tmpl: the value
project_out: the expanded `[project].out`
v: the values
ret: the expanded string; err for an unknown or unterminated reserved
             placeholder, the `expand` errors, or a result too large to size

## fun expand_project_path

```mach
pub fun expand_project_path(alloc: *A.Allocator, tmpl: str, project_out: str,
v: *TmplVars, field: str) res[str, outcome.Fail];
```

`expand`, then require the result to satisfy `is_project_path`

alloc: owns the returned string
tmpl: the template
project_out: as `expand`
v: the values
field: what the template is, for the error text
ret: the expanded path; the `expand` errors, or err naming `field` when the
             result escapes the project root

## fun target_matches_host

```mach
pub fun target_matches_host(itn: *intern.Interner, d: *TargetDef, host_os: u32, host_arch: u32) bool;
```

whether a target's `os` and `isa` name the given host ids. the abi is not compared

itn: resolves the target's strings
d: the target
host_os: an os id from `mach.lang.target.os`
host_arch: an arch id from `mach.lang.target.isa`
ret: true when both ids match; false when either string is unknown to `itn`

## fun resolve_profile

```mach
pub fun resolve_profile(alloc: *A.Allocator, itn: *intern.Interner, m: *Manifest, pick: str) res[ResolvedProfile, outcome.Fail];
```

choose the profile a selection names. a named profile must be declared, except
that a dependency manifest with no profiles at all answers "debug" and
"release" with the synthesized ones. an empty name takes the sole declared
profile, else the one with `default = true`, else the synthesized debug
profile of a dependency manifest. several declared profiles with none marked
default are refused: no profile is ever selected by table order

alloc: owns error text
itn: interns the name
m: the manifest
pick: the profile name, or "" for the default
ret: the profile; err "mach.toml: no profile named '<pick>'" or, with an empty
       name, an error when more than one profile is marked default

## fun resolve_artifact

```mach
pub fun resolve_artifact(itn: *intern.Interner, m: *Manifest, pick: Selection) *ArtifactDef;
```

the artifact a selection names: with `has_artifact`, the one whose name and
`is_lib` both match `pick`; otherwise the sole declared artifact

itn: interns the name
m: the manifest
pick: the selection
ret: the artifact, or nil when none is declared, the name has no match of
      the requested shape, or several artifacts are declared and none was named

## fun link_matches_target

```mach
pub fun link_matches_target(itn: *intern.Interner, l: *LinkDef, t: *TargetDef) bool;
```

whether a link's `os`, `isa` and `abi` filters all admit a target. an absent
axis admits everything, an empty one nothing, "*" everything

itn: interns "*"
l: the link
t: the target
ret: true when all three axes admit `t`

## fun link_requirement

```mach
pub fun link_requirement(alloc: *A.Allocator, itn: *intern.Interner, l: *LinkDef,
proj_out: str, v: *TmplVars) res[LinkRequirement, outcome.Fail];
```

resolve a link entry for one target into a `LinkRequirement`. a local `path`
is expanded with `expand_project_path` and interned; the symbols are copied
into an owned array

alloc: owns the symbol copy
itn: interns the expanded path
l: the link
proj_out: the expanded `[project].out`
v: the template values
ret: the requirement; on error nothing is left allocated

## fun local_path_demanded_by_step

```mach
pub fun local_path_demanded_by_step(alloc: *A.Allocator, itn: *intern.Interner, m: *Manifest,
expanded_path: str, proj_out: str, v: *TmplVars) bool;
```

whether some build step produces a path; `step_producing_out` as a bool

alloc: owns temporary expansions
itn: resolves step strings
m: the manifest
expanded_path: the path to match, already expanded
proj_out: the expanded `[project].out`
v: the template values
ret: true when a step lists the path in `out`

## fun find_artifact

```mach
pub fun find_artifact(itn: *intern.Interner, m: *Manifest, name: str) *ArtifactDef;
```

look an artifact up by name

itn: interns the name
m: the manifest
name: the artifact name
ret: the artifact, or nil

## fun artifact_needs_artifact

```mach
pub fun artifact_needs_artifact(itn: *intern.Interner, a: *ArtifactDef, other: *ArtifactDef) bool;
```

## fun required_artifact_uses_target

```mach
pub fun required_artifact_uses_target(itn: *intern.Interner, req: *ArtifactDef,
consumer_target: intern.StrId, t: *TargetDef) bool;
```

whether a required artifact is built for a target when its consumer builds for
`consumer_target`: only that target when `req` supports it, otherwise every
target `req` supports

itn: interns "*"
req: the required artifact
consumer_target: the consumer's target name
t: the target asked about
ret: true when `req` is built for `t` on behalf of that consumer

## fun artifact_required_by_any

```mach
pub fun artifact_required_by_any(itn: *intern.Interner, m: *Manifest, ra: *ArtifactDef) bool;
```

whether any artifact's `need` selects `ra`

itn: resolves the names
m: the manifest
ra: the candidate requirement
ret: true when some other artifact needs it

## fun resolve_artifact_reqs

```mach
pub fun resolve_artifact_reqs(alloc: *A.Allocator, itn: *intern.Interner, reg: *tgt.TargetRegistry, m: *Manifest,
consumer: *ArtifactDef, consumer_target: *TargetDef, profile: str,
whole_project: bool,
out_items: **ArtifactReq, out_count: *u32) err[outcome.Fail];
```

list the artifacts a consumer requires, each with its output path when that
path is unique, for `{artifact.<id>.out}` expansion

alloc: owns the returned array and each non-empty `out` string
itn: resolves names
m: the manifest
consumer: the requiring artifact; nil with `whole_project` false yields nothing
consumer_target: the consumer's target
profile: the profile name the outputs expand with
whole_project: select every artifact some artifact needs instead of `consumer`'s
out_items: receives the array, or nil when nothing is required
out_count: receives its length
ret: ok; err from the output path expansion, with the array freed

## fun plan_steps

```mach
pub fun plan_steps(alloc: *A.Allocator, itn: *intern.Interner, m: *Manifest,
art_names: *intern.StrId, art_count: u32, t: *TargetDef,
proj_out: str, v: *TmplVars,
out_order: **u32, out_count: *u32) err[outcome.Fail];
```

order the build steps the named artifacts demand: a step producing a local
link path that matches the target, every step a `need` names (globs match
step names), and transitively each step's own `need`, dependencies first

alloc: owns the returned order
itn: resolves names
m: the manifest
art_names: the artifacts to plan for; unknown names are skipped
art_count: length of `art_names`
t: the target
proj_out: the expanded `[project].out`
v: the template values
out_order: receives the step indices in run order, sized exactly; nil when none
out_count: receives the length
ret: ok; err on a step `need` cycle, a step `need` naming an unknown step, or
           a local link path that fails to expand

## fun plan_export_steps

```mach
pub fun plan_export_steps(alloc: *A.Allocator, itn: *intern.Interner, m: *Manifest,
t: *TargetDef, proj_out: str, v: *TmplVars,
out_order: **u32, out_count: *u32) err[outcome.Fail];
```

order the build steps that produce an exported local link path matching the
target, with their transitive step `need`s; the steps a consumer of this
manifest must run

alloc: owns the returned order
itn: resolves names
m: the manifest
t: the target
proj_out: the expanded `[project].out`
v: the template values
out_order: receives the step indices in run order, sized exactly; nil when none
out_count: receives the length
ret: ok; the `plan_steps` errors

## fun artifact_supports_target

```mach
pub fun artifact_supports_target(itn: *intern.Interner, a: *ArtifactDef, tname: intern.StrId) bool;
```

whether an artifact's `targets` lists a target name or "*"

itn: interns "*"
a: the artifact
tname: the target name
ret: true when listed

## fun cell_supported

```mach
pub fun cell_supported(alloc: *A.Allocator, itn: *intern.Interner, m: *Manifest, pick: Selection) res[bool, outcome.Fail];
```

whether the selection's artifact builds for the selection's target. the target
is resolved as `native` would be, never pinned by the artifact

alloc: owns error text
itn: resolves names
m: the manifest
pick: the selection
ret: the support bit; err from target resolution, or when `resolve_artifact`
       finds nothing

## val AMBIGUOUS_TARGET_MSG

```mach
pub val AMBIGUOUS_TARGET_MSG:   str = "mach.toml: several targets are declared, none matches the host and none is marked `default = true`
```

the 4.30 first-declared fallbacks are gone: a selection several candidates
could satisfy is refused where a command must pick one (#3222, #3226)

## val AMBIGUOUS_PROFILE_MSG

```mach
pub val AMBIGUOUS_PROFILE_MSG:  str = "mach.toml: several profiles are declared and none is marked `default = true`
```

## val AMBIGUOUS_ARTIFACT_MSG

```mach
pub val AMBIGUOUS_ARTIFACT_MSG: str = "mach.toml: several artifacts support the selected target and none is marked `default = true`
```

## fun select_primary_artifact

```mach
pub fun select_primary_artifact(alloc: *A.Allocator, itn: *intern.Interner,
m: *Manifest, pick: *Selection) err[outcome.Fail];
```

fill in `pick.artifact` when none was named and several artifacts are declared:
the sole artifact supporting the selected target, else the one marked
`default = true`; several candidates with none marked default are refused,
never selected by table order

alloc: owns error text
itn: resolves names
m: the manifest
pick: updated in place; untouched when it already names an artifact or fewer
       than two are declared
ret: ok; err from target resolution, when no artifact supports the target,
       when more than one candidate is marked default, or when several are and none is

## fun select_sole_target_artifact

```mach
pub fun select_sole_target_artifact(alloc: *A.Allocator, itn: *intern.Interner,
m: *Manifest, pick: *Selection) err[outcome.Fail];
```

`select_primary_artifact` without the fallback: fill in `pick.artifact` only
when the target leaves one candidate or one marked `default = true`, and stay
silent otherwise

alloc: owns error text
itn: resolves names
m: the manifest
pick: updated in place when a choice is clear
ret: ok unless the artifact name cannot be looked up

## fun select_sole_executable_artifact

```mach
pub fun select_sole_executable_artifact(alloc: *A.Allocator, itn: *intern.Interner,
m: *Manifest, pick: *Selection) err[outcome.Fail];
```

`select_sole_target_artifact` over `bin` artifacts only, so a bin beside a
library is chosen for `run` and `test`

alloc: owns error text
itn: resolves names
m: the manifest
pick: updated in place when a choice is clear
ret: ok unless the artifact name cannot be looked up

## fun resolve_build_unit

```mach
pub fun resolve_build_unit(alloc: *A.Allocator, itn: *intern.Interner, reg: *tgt.TargetRegistry, m: *Manifest, pick: Selection) res[BuildUnit, outcome.Fail];
```

resolve a selection into one `BuildUnit`. with no target named, an artifact
that supports exactly one declared target, or exactly one host-matching
target, pins it; otherwise `native` resolution applies. the profile comes
from `resolve_profile`, the artifact from `resolve_artifact`, and every path
is expanded with `expand_project_path`

alloc: owns `libs` and temporary strings
itn: interns the expanded paths
m: the manifest
pick: the selection
ret: the unit; err from target, profile or artifact resolution, when the
       artifact does not support the target (naming the artifact's targets when
       none was selected), or from path expansion

## fun resolve_test_libs

```mach
pub fun resolve_test_libs(alloc: *A.Allocator, itn: *intern.Interner, m: *Manifest, t: *TargetDef,
proj_out: str, v: *TmplVars,
out_items: **LinkRequirement, out_count: *u32) err[outcome.Fail];
```

the union of every artifact's link requirements for a target, for the test
binary. requirements equal in source, text and library merge with
`merge_link_claims`

alloc: owns the returned array
itn: resolves names
m: the manifest
t: the target
proj_out: the expanded `[project].out`
v: the template values
out_items: receives the exact array, or nil when no artifact links anything
out_count: receives its length
ret: ok; err from `link_requirement` or a merge, with everything freed

## fun check_collisions

```mach
pub fun check_collisions(alloc: *A.Allocator, itn: *intern.Interner, reg: *tgt.TargetRegistry, m: *Manifest,
v: *TmplVars) err[outcome.Fail];
```

reject two artifacts whose `out` expands to the same path under one set of
template values

alloc: owns temporary strings
itn: resolves names
m: the manifest; fewer than two artifacts always pass
v: the template values
ret: ok; err "mach.toml: artifacts 'a' and 'b' collide on output path '<p>'",
       or an expansion error

## def DepSource

```mach
pub def DepSource: u8
```

the source key a `mach dep add` writes

## val DEP_SOURCE_GIT

```mach
pub val DEP_SOURCE_GIT: DepSource = 1
```

a `git = "<url>"` dependency, with an optional `ref`

## val DEP_SOURCE_PATH

```mach
pub val DEP_SOURCE_PATH: DepSource = 2
```

a `path = "<dir>"` dependency

## rec DepTableSpec

```mach
pub rec DepTableSpec;
```

a `[dep.<name>]` table to append to a manifest's text

name: the dependency name; must satisfy `is_valid_id`
source: which key `value` is written under
value: the git URL or the path; escaped as a TOML basic string
ref: the `ref` value; written only for a git source and only when non-empty

## fun manifest_add_dep_table

```mach
pub fun manifest_add_dep_table(alloc: *A.Allocator, source_text: str, spec: *DepTableSpec) res[str, outcome.Fail];
```

append a `[dep.<name>]` table to manifest text, leaving every existing byte in
place. the block uses the file's line ending and is separated by one blank
line (two when the text lacks a trailing newline)

alloc: owns the returned text
source_text: the current `mach.toml`
spec: the table to add
ret: the new text; err when the name is not an identifier, the source is not
             git or path, the text is not valid TOML, the dependency is already declared
             in any form, or the result does not reparse with the table present

## fun manifest_remove_dep_table

```mach
pub fun manifest_remove_dep_table(alloc: *A.Allocator, source_text: str, name: str) res[str, outcome.Fail];
```

cut a `[dep.<name>]` table out of manifest text, from its header line to the
next header, leaving everything else byte for byte

alloc: owns the returned text
source_text: the current `mach.toml`
name: the dependency name
ret: the new text; err when the name is not an identifier, the text is not
             valid TOML, the dependency is absent, is declared but not as a table, is
             declared more than once, or its header is not spelled exactly `[dep.<name>]`
             at the start of a line

