# mach.lang.manifest.template

the one template engine: every path and value template of a manifest, and
every `#[embed]` path, expands through it. a template is text in which a
`{name}` group names a variable. which variables a template may name is
decided by where it is written, its context; the variables are rows of one
table, each naming the contexts that admit it. the engine walks a template
twice, once to size the result and refuse what the context does not admit,
once to write it, and the load-time check is the first walk without values

## def Context

```mach
pub def Context: u8
```

where a template is written, which decides the variables it may name

## val CONTEXT_PROJECT_OUT

```mach
pub val CONTEXT_PROJECT_OUT: Context = 0
```

`[project].out`

## val CONTEXT_ARTIFACT_OUT

```mach
pub val CONTEXT_ARTIFACT_OUT: Context = 1
```

an artifact's own `out`

## val CONTEXT_PATH

```mach
pub val CONTEXT_PATH: Context = 2
```

a step's `in` and `out` entries and a local link's `path`

## val CONTEXT_STEP_VALUE

```mach
pub val CONTEXT_STEP_VALUE: Context = 3
```

a step's `argv` and `env` values

## val CONTEXT_EMBED

```mach
pub val CONTEXT_EMBED: Context = 4
```

an `#[embed]` path

## rec Requirement

```mach
pub rec Requirement;
```

one artifact a template may name as `{artifact.<id>.out}`

name: the required artifact's name
out: its final output path when it builds for exactly one target here, or ""
      when it builds for several, which makes `{artifact.<id>.out}` ambiguous

## rec Requirements

```mach
pub rec Requirements;
```

the artifacts the modules of one project may name as `{artifact.<id>.out}`

owner: the id of the project whose modules resolve in this scope
dependency: the requirements are a dependency's default library artifacts',
            not the requirements of the artifact being built
reqs: the requirements; nil when none
req_count: length of `reqs`

## rec Values

```mach
pub rec Values;
```

the values a template expands with

target: `{target.name}`
isa: `{target.isa}`
os: `{target.os}`
abi: `{target.abi}`
profile: `{profile.name}`
format: the target's explicit `of` override, "" when the format is derived
artifact_suffix: `{artifact.suffix}`, set for the artifact whose output is named
reqs: the artifacts `{artifact.<id>.out}` may name; nil when none
req_count: length of `reqs`
dependency: the id of the dependency whose default library requirements
                 `reqs` are, "" for the artifact being built

## fun values_of

```mach
pub fun values_of(itn: *intern.Interner, t: *TargetDef, pn: str) Values;
```

the values for one target and profile, with no requirements

itn: resolves the target's strings
t: the target
pn: the profile name

## fun values_with

```mach
pub fun values_with(itn: *intern.Interner, t: *TargetDef, pn: str, reqs: *Requirement, req_count: u32) Values;
```

`values_of` with the requirements `{artifact.<id>.out}` may name

## fun embed_values

```mach
pub fun embed_values(reqs: *Requirement, req_count: u32, dependency: str) Values;
```

the values an `#[embed]` path expands with: the requirements of one scope

reqs: the requirements
req_count: length of `reqs`
dependency: the dependency whose default library requirements they are, "" for
            the artifact being built

## fun expand

```mach
pub fun expand(alloc: *A.Allocator, tmpl: str, ctx: Context, project_out: str, v: *Values) res[str, fail.Fail];
```

expand `tmpl`, written in `ctx`

alloc: owns the result and a refusal's text
tmpl: the template
ctx: where it is written
project_out: the value of `{project.out}`
v: the other values
ret: the expansion; err for an unterminated group, a variable the
             context does not admit or that does not exist, an artifact
             outside `v.reqs` or one whose output is ambiguous

## fun path_expand

```mach
pub fun path_expand(alloc: *A.Allocator, tmpl: str, ctx: Context, project_out: str, v: *Values, field: str) res[str, fail.Fail];
```

`expand`, then require the result to be a canonical strict descendant of the
project root

field: what the template is, for the refusal

## fun check

```mach
pub fun check(alloc: *A.Allocator, tmpl: str, ctx: Context) err[fail.Fail];
```

refuse what `expand` would refuse in `ctx` before any value is known: an
unterminated group, and a variable that does not exist or that the context
does not admit

## fun artifact_id

```mach
pub fun artifact_id(alloc: *A.Allocator, tmpl: str) res[opt[str], fail.Fail];
```

the id the first `{artifact.<id>.out}` group in `tmpl` names, owned by
`alloc`; none when the template holds no such group

