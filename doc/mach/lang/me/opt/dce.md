# mach.lang.me.opt.dce

dead code elimination. unreachable blocks go first, so a value only they
read is dead to the sweep. an instruction with no effect whose value nothing
live reads is erased, a dead phi cycle with it; liveness grows from every
instruction with an effect. a dead value a debug binding names is salvaged
onto a live one where the binding can be rewritten, so `-g` keeps the
variable without keeping the code

## val PASS

```mach
pub val PASS: pass.Pass = pass.Pass;
```

the pass the pipeline schedules

