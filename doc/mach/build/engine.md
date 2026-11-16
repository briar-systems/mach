[mach](../README.md) / [build](README.md) / **engine**

# mach.build.engine

the build engine, which runs a plan cell by cell through its phases

each unit runs cold, on a session and a heap of its own torn down when it finishes. a
unit whose requirement failed is recorded as blocked and skipped, a unit that fails does
not stop the rest, and the plan's outcome carries the worst unit's severity.

<sub>source: [`src/build/engine.mach`](../../../src/build/engine.mach) · imported by 2 modules</sub>

<sub>`build_plan` is [`mach.build.plan`](plan.md) · `A` is `std.allocator` · `heap` is `std.allocator.heap` · `readout` is [`mach.report.readout`](../report/readout.md) · `outcome` is [`mach.build.outcome`](outcome.md) · `fail` is [`mach.base.fail`](../base/fail.md)</sub>

| Functions | |
| :--- | :--- |
| [`execute`](#execute) | run every planned cell cold: each unit gets its own session and heap over `spans` and is torn down when it finishes, so nothing of one unit's frontend survives into the next |

---

## Functions

### execute

```mach
pub fun execute(bp: *build_plan.BuildPlan, backing: *A.Allocator, spans: *heap.Pool, oa: *A.Allocator,
    sink: *readout.Sink) res[outcome.BuildOutcome, fail.Fail]
```

run every planned cell cold: each unit gets its own session and heap over `spans`
and is torn down when it finishes, so nothing of one unit's frontend survives into the
next. a unit whose requirement failed is recorded as blocked and skipped; a unit that
fails is recorded and the remaining units still run. ok is the outcome of the whole
plan, its severity the worst unit's; err is a failure of the engine itself, never of a
unit

| Parameter | |
| :--- | :--- |
| `bp` | the plan, its closure located; read only, and trusted as planned |
| `backing` | the allocator a unit's scratch work draws from and returns to |
| `spans` | the pool each unit's heap and each module's draws its spans from and gives them back to when it is torn down |
| `oa` | owns the returned outcome and every path and text in it |
| `sink` | where every unit, failure, diagnostic and phase row is sent as it happens; nil for none |
| **returns** | the outcome, released with outcome.outcome_dnit, or an engine failure whose text `oa` owns |

