# mach.lang.me.pass.inline

## rec InlineBudget

```mach
pub rec InlineBudget;
```

## fun run

```mach
pub fun run(m: *ir.Module, tgt: *target.Target, out_report: *InlineBudget) res[bool, fail.Fail];
```

## fun run_available

```mach
pub fun run_available(m: *ir.Module, tgt: *target.Target, available: *body.Available, out_report: *InlineBudget) res[bool, fail.Fail];
```

a run that owns its workspace; the pipeline runs `run_in` over one

## fun run_in

```mach
pub fun run_in(m: *ir.Module, tgt: *target.Target, available: *body.Available, out_report: *InlineBudget, workspace: *scratch.Workspace) res[bool, fail.Fail];
```

the analysis (recursion marks, call counts, address-taken facts, duplication
counts, the per-call remap tables and the peel frontier) is scratch for the
whole module; the clone itself, its metadata and the peel snapshot are the
module's

## fun mark_recursive

```mach
pub fun mark_recursive(m: *ir.Module, recursive: *bool, scratch_alloc: *A.Allocator) err[fail.Fail];
```

the visit state is `scratch`; `recursive` is the caller's

