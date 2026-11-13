# mach.lang.build.declassified

the listing of a checked project's declassifications, which `mach check --declassified` prints:
every `:>T` of the project's own source, read off the syntax and the types sema gave it, so it
lists what the source states and never what lowering or an instance makes of it

## fun list

```mach
pub fun list(p: *project.Project, bo: *outcome.BuildOutcome) err[fail.Fail];
```

record every declassification of the project's own modules in `bo`, module by module, each in
source order

