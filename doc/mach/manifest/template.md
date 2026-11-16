[mach](../README.md) / [manifest](README.md) / **template**

# mach.manifest.template

the one template engine: every path and value template of a manifest, and
every `#[embed]` path, expands through it. a template is text in which a
`{name}` group names a variable. which variables a template may name is
decided by where it is written, its context; the variables are rows of one
table, each naming the contexts that admit it. the engine walks a template
twice, once to size the result and refuse what the context does not admit,
once to write it, and the load-time check is the first walk without values

Source: [`src/manifest/template.mach`](../../../src/manifest/template.mach)

Uses types from [`mach.base.fail`](../base/fail.md), [`mach.base.intern`](../base/intern.md).

| Type | | |
| :--- | :--- | :--- |
| [`Context`](#context) | `def` | where a template is written, which decides the variables it may name |
| [`Renamed`](#renamed) | `rec` | a variable a previous major named, and the name that replaced it, which `mach migrate` writes |
| [`Requirement`](#requirement) | `rec` | one artifact a template may name as `{artifact.<id>.out}` |
| [`Requirements`](#requirements) | `rec` | the artifacts the modules of one project may name as `{artifact.<id>.out}` |
| [`Values`](#values) | `rec` | the values a template expands with |

| On `Context` | |
| :--- | :--- |
| [`CONTEXT_PROJECT_WORK`](#context) | `[project].work` |
| [`CONTEXT_ARTIFACT_OUT`](#context) | an artifact's own `out` |
| [`CONTEXT_PATH`](#context) | a step's `in` and `out` entries and a local link's `path` |
| [`CONTEXT_STEP_VALUE`](#context) | a step's `argv` and `env` values |
| [`CONTEXT_EMBED`](#context) | an `#[embed]` path |
| [`CONTEXT_RUNNER`](#context) | a target's `runner` entries |
| [`expand`](#expand) | expand `tmpl`, written in `ctx` |
| [`path_expand`](#path_expand) | `expand`, then require the result to be a canonical strict descendant of the project root |
| [`check`](#check) | refuse what `expand` would refuse in `ctx` before any value is known: an unterminated group, and a variable that does not exist or that the context does not admit |

| On `Renamed` | |
| :--- | :--- |
| [`renamed`](#renamed-1) | the renamed row at `i`, nil past the end |

| On `Requirement` | |
| :--- | :--- |
| [`values_with`](#values_with) | `values_of` with the requirements `{artifact.<id>.out}` may name |
| [`embed_values`](#embed_values) | the values an `#[embed]` path expands with: the requirements of one scope |

| On `Values` | |
| :--- | :--- |
| [`values_of`](#values_of) | the values for one target and profile, with no requirements |
| [`values_make`](#values_make) | values that name nothing, each field set by the caller that has it |

| Functions | |
| :--- | :--- |
| [`artifact_id`](#artifact_id) | the id the first `{artifact.<id>.out}` group in `tmpl` names, owned by `alloc`; none when the template holds no such group |

---

## Context

```mach
pub def Context: u8
```

where a template is written, which decides the variables it may name

| Name | Value | |
| :--- | :--- | :--- |
| `CONTEXT_PROJECT_WORK` | `0` | `[project].work` |
| `CONTEXT_ARTIFACT_OUT` | `1` | an artifact's own `out` |
| `CONTEXT_PATH` | `2` | a step's `in` and `out` entries and a local link's `path` |
| `CONTEXT_STEP_VALUE` | `3` | a step's `argv` and `env` values |
| `CONTEXT_EMBED` | `4` | an `#[embed]` path |
| `CONTEXT_RUNNER` | `5` | a target's `runner` entries |

### expand

```mach
pub fun expand(alloc: *std.allocator.Allocator, tmpl: str, ctx: Context, project_work: str, v: *Values) res[str, mach.base.fail.Fail]
```

expand `tmpl`, written in `ctx`

| Parameter | |
| :--- | :--- |
| `alloc` | owns the result and a refusal's text |
| `tmpl` | the template |
| `ctx` | where it is written |
| `project_work` | the value of `{project.work}` |
| `v` | the other values |
| **returns** | the expansion; err for an unterminated group, a variable the context does not admit or that does not exist, an artifact outside `v.reqs` or one whose output is ambiguous |

### path_expand

```mach
pub fun path_expand(alloc: *std.allocator.Allocator, tmpl: str, ctx: Context, project_work: str, v: *Values, field: str) res[str, mach.base.fail.Fail]
```

`expand`, then require the result to be a canonical strict descendant of the
project root

| Parameter | |
| :--- | :--- |
| `field` | what the template is, for the refusal |

### check

```mach
pub fun check(alloc: *std.allocator.Allocator, tmpl: str, ctx: Context) err[mach.base.fail.Fail]
```

refuse what `expand` would refuse in `ctx` before any value is known: an
unterminated group, and a variable that does not exist or that the context
does not admit

## Renamed

```mach
pub rec Renamed {
    from: str;
    to:   str;
}
```

a variable a previous major named, and the name that replaced it, which
`mach migrate` writes

### renamed

```mach
pub fun renamed(i: usize) *Renamed
```

the renamed row at `i`, nil past the end

## Requirement

```mach
pub rec Requirement {
    name:    str;
    out:     str;
    objects: bool;
}
```

one artifact a template may name as `{artifact.<id>.out}`

| Field | Type | |
| :--- | :--- | :--- |
| `name` | `str` | the required artifact's name |
| `out` | `str` | its final output path when it builds for exactly one target here, or "" when it builds for several, which makes `{artifact.<id>.out}` ambiguous, or when it is an object artifact |
| `objects` | `bool` | an object artifact, whose objects stay under obj/, so it has no out to name |

### values_with

```mach
pub fun values_with(itn: *mach.base.intern.Interner, t: *TargetDef, pn: str, reqs: *Requirement, req_count: u32) Values
```

`values_of` with the requirements `{artifact.<id>.out}` may name

### embed_values

```mach
pub fun embed_values(reqs: *Requirement, req_count: u32, dependency: str) Values
```

the values an `#[embed]` path expands with: the requirements of one scope

| Parameter | |
| :--- | :--- |
| `reqs` | the requirements |
| `req_count` | length of `reqs` |
| `dependency` | the dependency whose default library requirements they are, "" for the artifact being built |

## Requirements

```mach
pub rec Requirements {
    owner:      mach.base.intern.StrId;
    dependency: bool;
    reqs:       *Requirement;
    req_count:  u32;
}
```

the artifacts the modules of one project may name as `{artifact.<id>.out}`

| Field | Type | |
| :--- | :--- | :--- |
| `owner` | `mach.base.intern.StrId` | the id of the project whose modules resolve in this scope |
| `dependency` | `bool` | the requirements are a dependency's default library artifacts', not the requirements of the artifact being built |
| `reqs` | `*Requirement` | the requirements; nil when none |
| `req_count` | `u32` | length of `reqs` |

## Values

```mach
pub rec Values {
    target:          str;
    arch:            str;
    sys:             str;
    abi:             str;
    declared:        *TargetDef;
    profile:         str;
    format:          str;
    artifact_suffix: str;
    run_out:         str;
    reqs:            *Requirement;
    req_count:       u32;
    dependency:      str;
}
```

the values a template expands with

| Field | Type | |
| :--- | :--- | :--- |
| `target` | `str` | `{target.name}` |
| `arch` | `str` | `{target.arch}` |
| `sys` | `str` | `{target.sys}` |
| `abi` | `str` | `{target.abi}` |
| `declared` | `*TargetDef` | the declared target the values name, nil when none |
| `profile` | `str` | `{profile.name}` |
| `format` | `str` | the target's explicit `of` override, "" when the format is derived |
| `artifact_suffix` | `str` | `{artifact.suffix}`, set for the artifact whose output is named |
| `run_out` | `str` | `{artifact.out}`, set for the program a runner launches |
| `reqs` | `*Requirement` | the artifacts `{artifact.<id>.out}` may name; nil when none |
| `req_count` | `u32` | length of `reqs` |
| `dependency` | `str` | the id of the dependency whose default library requirements `reqs` are, "" for the artifact being built |

### values_of

```mach
pub fun values_of(itn: *mach.base.intern.Interner, t: *TargetDef, pn: str) Values
```

the values for one target and profile, with no requirements

| Parameter | |
| :--- | :--- |
| `itn` | resolves the target's strings |
| `t` | the target |
| `pn` | the profile name |

### values_make

```mach
pub fun values_make() Values
```

values that name nothing, each field set by the caller that has it

## Functions

### artifact_id

```mach
pub fun artifact_id(alloc: *std.allocator.Allocator, tmpl: str) res[opt[str], mach.base.fail.Fail]
```

the id the first `{artifact.<id>.out}` group in `tmpl` names, owned by
`alloc`; none when the template holds no such group

