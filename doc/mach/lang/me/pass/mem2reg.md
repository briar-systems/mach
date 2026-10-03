# mach.lang.me.pass.mem2reg

## fun run

```mach
pub fun run(m: *me_ir.Module) res[bool, fail.Fail];
```

a run that owns its workspace; the pipeline runs `run_in` over one

## val PASS

```mach
pub val PASS: pass.Pass = pass.Pass;
```

the pass the pipeline schedules

## fun run_in

```mach
pub fun run_in(ctx: *pass.Context) res[bool, fail.Fail];
```

