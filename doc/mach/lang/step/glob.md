# mach.lang.step.glob

the input patterns a build step names: a wildcard over one directory, or one
recursive `**` segment, expanded beneath the project without following a link

## fun wildcard_match

```mach
pub fun wildcard_match(pat: str, name: str) bool;
```

## fun shape_ok

```mach
pub fun shape_ok(pattern: str) bool;
```

## fun expand

```mach
pub fun expand(alloc: *A.Allocator, project_root: str, pattern: str, out: *collections_vector.Vector[str]) err[fail.Fail];
```

