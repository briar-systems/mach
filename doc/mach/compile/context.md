# mach.compile.context

the frozen build context: everything the computations of one build read of its cell, its
request and its target, captured into the TARGET input so a product that reads it depends on
it, and fingerprinted so a change after the freeze is refused rather than read

## val TARGET

```mach
pub val TARGET: query.Kind = query.Kind;
```

the build context of the last frozen build, keyed 0

## fun capture_build_config

```mach
pub fun capture_build_config(p: *project.Project, alloc: *A.Allocator) res[query.QueryOutput, fail.Fail];
```

## fun capture_configuration_identity

```mach
pub fun capture_configuration_identity(p: *project.Project, alloc: *A.Allocator) res[query.QueryOutput, fail.Fail];
```

the build identity without the request: the persistent key hashes the request
itself, with the project root in canonical form, so the spelled root stays out

## fun freeze

```mach
pub fun freeze(p: *project.Project) err[fail.Fail];
```

freeze the build context: what every computation reads of the cell, the request and the
target is captured once, set as the TARGET input every product that reads it depends on, and
fingerprinted from the same bytes, so a later change to it is refused rather than read

## fun unchanged

```mach
pub fun unchanged(p: *project.Project) err[fail.Fail];
```

refused when the context a frozen build reads no longer captures to the bytes it was frozen at

