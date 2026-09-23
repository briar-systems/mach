# mach.lang.manifest.artifact

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

## rec RequirementScope

```mach
pub rec RequirementScope;
```

the artifacts the modules of one project may name as `{artifact.<id>.out}`

owner: the id of the project whose modules resolve in this scope
dependency: the requirements are a dependency's default library artifacts',
            not the requirements of the artifact being built
reqs: the requirements; nil when none
req_count: length of `reqs`

## fun parse_artifacts

```mach
pub fun parse_artifacts(alloc: *A.Allocator, itn: *intern.Interner, t: *toml.Table, m: *Manifest, as_root: bool) err[outcome.Fail];
```

## fun artifact_template_id

```mach
pub fun artifact_template_id(alloc: *A.Allocator, tmpl: str) opt[str];
```

the artifact id the first `{artifact.<id>.out}` group in `tmpl` names, allocated
in `alloc`; none when the template holds no such group. a diagnostic uses this to
say whose output a resolved path is, which the path alone no longer shows

alloc: owns the returned id
tmpl: the unexpanded template

## fun tmpl_vars_of

```mach
pub fun tmpl_vars_of(itn: *intern.Interner, t: *TargetDef, pn: str) TmplVars;
```

template variables for one target and profile, with no artifact requirements

itn: resolves the target's interned strings
t: the target
pn: the profile name
ret: the variables; `reqs` is nil

## fun tmpl_vars_of_reqs

```mach
pub fun tmpl_vars_of_reqs(reqs: *ArtifactReq, req_count: u32) TmplVars;
```

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
dependency: str, field: str) res[str, TemplateError];
```

expand an `{artifact.<id>.out}` path, the one template vocabulary a source path
accepts

alloc: owns the returned path and a rejection's text
tmpl: the template
reqs: the requirements the template may name
req_count: length of `reqs`
dependency: the id of the dependency whose default library requirements `reqs`
            are, or "" for the artifact being built; names the scope a rejection
            points at
field: what the template is, for the rejection
ret: the expanded path; err rejected for a name outside `reqs`, an ambiguous
            output or any other template, internal for allocation

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

## fun find_artifact

```mach
pub fun find_artifact(itn: *intern.Interner, m: *Manifest, name: str) *ArtifactDef;
```

look an artifact up by name

itn: interns the name
m: the manifest
name: the artifact name
ret: the artifact, or nil

## fun find_artifact_by_name

```mach
pub fun find_artifact_by_name(m: *Manifest, name: intern.StrId) *ArtifactDef;
```

## fun artifact_needs_artifact

```mach
pub fun artifact_needs_artifact(itn: *intern.Interner, a: *ArtifactDef, other: *ArtifactDef) bool;
```

## fun expand_artifact_output

```mach
pub fun expand_artifact_output(alloc: *A.Allocator, itn: *intern.Interner, reg: *tgt.TargetRegistry,
a: *ArtifactDef, project_out: str, vars: *TmplVars) res[str, outcome.Fail];
```

## fun validate_need_cycles

```mach
pub fun validate_need_cycles(alloc: *A.Allocator, itn: *intern.Interner, m: *Manifest) err[outcome.Fail];
```

## fun artifact_supports_target

```mach
pub fun artifact_supports_target(itn: *intern.Interner, a: *ArtifactDef, tname: intern.StrId) bool;
```

whether an artifact's `targets` lists a target name or "*"

itn: interns "*"
a: the artifact
tname: the target name
ret: true when listed

## fun mk_tv

```mach
pub fun mk_tv(target: str, isa: str, os: str, abi: str, profile: str) TmplVars;
```

