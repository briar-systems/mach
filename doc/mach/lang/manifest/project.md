# mach.lang.manifest.project

## fun parse_project

```mach
pub fun parse_project(alloc: *A.Allocator, itn: *intern.Interner, t: *toml.Table, m: *Manifest) err[fail.Fail];
```

## fun expand_project_work

```mach
pub fun expand_project_work(alloc: *A.Allocator, itn: *intern.Interner, m: *Manifest, v: *template.Values) res[str, fail.Fail];
```

expand `m`'s `[project].work` with `v`; a refusal points at the `work` value

