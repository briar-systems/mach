# mach.lang.build.engine

## fun execute

```mach
pub fun execute(bp: *build_plan.BuildPlan, backing: *A.Allocator, oa: *A.Allocator,
sink: *readout.Sink) res[outcome.BuildOutcome, fail.Fail];
```

run every planned cell cold: each unit gets its own session and heap over `backing`
and is torn down when it finishes, so nothing of one unit's frontend survives into the
next. a unit whose requirement failed is recorded as blocked and skipped; a unit that
fails is recorded and the remaining units still run. ok is the outcome of the whole
plan, its severity the worst unit's; err is a failure of the engine itself, never of a
unit

bp: the plan, its closure located; read only, and trusted as planned
backing: the allocator each unit's heap draws its spans from; released per unit
oa: owns the returned outcome and every path and text in it
sink: where every unit, failure, diagnostic and phase row is sent as it happens; nil for none
ret: the outcome, released with outcome.outcome_dnit, or an engine failure whose text
         `oa` owns

