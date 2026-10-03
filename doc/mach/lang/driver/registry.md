# mach.lang.driver.registry

the toolchain's target registry: every member composed once, on first use,
and published for the life of the process. a session holds no registry, so
whoever resolves a target borrows this one. the first use comes before any
worker runs, since every target a build reads is resolved before it fans out

## fun setup_registry

```mach
pub fun setup_registry(reg: *lang_target.TargetRegistry) err[fail.Fail];
```

## fun registry

```mach
pub fun registry() res[*lang_target.TargetRegistry, fail.Fail];
```

