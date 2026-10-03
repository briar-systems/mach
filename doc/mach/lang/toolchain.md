# mach.lang.toolchain

the toolchain's composition root: the one place the target catalog is
registered. a session that builds, checks or analyses comes from
`session_init`, and a caller that needs targets without a session takes
`registry_init`. nothing below this module registers a target member, and
nothing above it composes a registry by hand

## fun session_init

```mach
pub fun session_init(alloc: *A.Allocator) res[session.Session, fail.Fail];
```

a session whose target registry holds the whole catalog

alloc: backs the session and its registry
ret: the session, released with `session.dnit`; err when the session or the catalog cannot be made

## fun registry_init

```mach
pub fun registry_init(alloc: *A.Allocator) res[*lang_target.TargetRegistry, fail.Fail];
```

a target registry holding the whole catalog, for a caller that resolves targets without a session

alloc: backs the registry and every entry
ret: the registry, released with `lang_target.registry_dnit`; err when it cannot be made

