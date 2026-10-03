# mach.lang.manifest.project

## fun parse_project

```mach
pub fun parse_project(alloc: *A.Allocator, itn: *intern.Interner, t: *toml.Table, m: *Manifest) err[fail.Fail];
```

## fun expand_project_out

```mach
pub fun expand_project_out(alloc: *A.Allocator, itn: *intern.Interner, m: *Manifest, v: *template.Values) res[str, fail.Fail];
```

expand `m`'s `[project].out` with `v`; a refusal points at the `out` value

