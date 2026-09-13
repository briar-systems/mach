# mach.lang.me.pass.cse

## fun run

```mach
pub fun run(m: *ir.Module) res[bool, fail.Fail];
```

a run that owns its workspace; the pipeline runs `run_in` over one

## fun run_in

```mach
pub fun run_in(m: *ir.Module, workspace: *scratch.Workspace) res[bool, fail.Fail];
```

