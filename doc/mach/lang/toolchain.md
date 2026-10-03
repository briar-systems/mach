# mach.lang.toolchain

the toolchain's composition root: the one place the target catalog is
registered. the toolchain owns one composed registry for the life of the
process (`registry`), which every session, the driver, the editor and the
command line borrow; the session holds none. a caller that wants a registry
of its own takes `registry_init`. nothing below this module registers a
target member, and nothing above it composes a registry by hand

## fun registry

```mach
pub fun registry() res[*lang_target.TargetRegistry, fail.Fail];
```

the toolchain's registry, composed on first use and published for the life of
the process; the first use comes before any worker runs, since every target a
build reads is resolved before it fans out

ret: the registry, never released; err when the catalog cannot be composed

## fun session_init

```mach
pub fun session_init(alloc: *A.Allocator) res[session.Session, fail.Fail];
```

a session that builds, checks or analyses, with the toolchain registry
composed for every target it resolves

alloc: backs the session
ret: the session, released with `session.dnit`; err when the session or the catalog cannot be made

## fun registry_init

```mach
pub fun registry_init(alloc: *A.Allocator) res[*lang_target.TargetRegistry, fail.Fail];
```

a target registry holding the whole catalog, for a caller that resolves targets without a session

alloc: backs the registry and every entry
ret: the registry, released with `lang_target.registry_dnit`; err when it cannot be made

