[mach](../README.md) / [build](README.md) / **declassified**

# mach.build.declassified

the listing of a checked project's declassifications, which `mach check --declassified` prints:
every `:>T` of the project's own source, read off the syntax and the types sema gave it, so it
lists what the source states and never what lowering or an instance makes of it

Source: [`src/build/declassified.mach`](../../../src/build/declassified.mach)

Uses types from [`mach.base.fail`](../base/fail.md), [`mach.build.outcome`](outcome.md), [`mach.compile.project`](../compile/project.md).

| Functions | |
| :--- | :--- |
| [`list`](#list) | record every declassification of the project's own modules in `bo`, module by module, each in source order |

---

## Functions

### list

```mach
pub fun list(p: *mach.compile.project.Project, bo: *mach.build.outcome.BuildOutcome) err[mach.base.fail.Fail]
```

record every declassification of the project's own modules in `bo`, module by module, each in
source order

