# mach.lang.me.pass.inline

## val PASS

```mach
pub val PASS: pass.Pass = pass.Pass;
```

the pass the pipeline schedules

## fun run_in

```mach
pub fun run_in(ctx: *pass.Context) res[bool, fail.Fail];
```

the bodies `ctx.available` offers are attached to the module for the run

## fun mark_recursive

```mach
pub fun mark_recursive(m: *me_ir.Module, recursive: *bool, scratch_alloc: *A.Allocator) err[fail.Fail];
```

the visit state is `scratch`; `recursive` is the caller's

