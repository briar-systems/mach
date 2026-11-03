# mach.lang.driver.toolchain

the toolchain's sessions: every session that builds, checks or analyses is made
here. a target is resolved from the owners' catalogs, which are static, so no
session or caller composes a registry of targets

## fun session_init

```mach
pub fun session_init(alloc: *A.Allocator) res[session.Session, fail.Fail];
```

a session that builds, checks or analyses

alloc: backs the session
ret: the session, released with `session.dnit`; err when the session cannot be made

## fun session_init_over

```mach
pub fun session_init_over(alloc: *A.Allocator, base: *intern.Interner) res[session.Session, fail.Fail];
```

`session_init` for a session layered over the names of `base`, as a build
cell's session is over its plan's

