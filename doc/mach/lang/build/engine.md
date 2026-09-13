# mach.lang.build.engine

## fun execute

```mach
pub fun execute(bp: *plan.BuildPlan, backing: *A.Allocator, oa: *A.Allocator,
ev: *readout.Progress) res[outcome.BuildOutcome, outcome.Fail];
```

run every planned cell cold: each unit gets its own session and arena over `backing`
and is torn down when it finishes, so nothing of one unit's frontend survives into the
next. a unit whose requirement failed is recorded as blocked and skipped; a unit that
fails is recorded and the remaining units still run. ok is the outcome of the whole
plan, its severity the worst unit's; err is a failure of the engine itself, never of a
unit

bp: the plan, read only
backing: the allocator each unit's arena grows from; released per unit
oa: owns the returned outcome and every path, event and text in it
ev: progress sink for the readout; nil for none
ret: the outcome, released with outcome.outcome_dnit, or an engine failure whose text
         `oa` owns

## fun execute_warm

```mach
pub fun execute_warm(bp: *plan.BuildPlan, unit_index: usize, s: *session.Session, oa: *A.Allocator,
ev: *readout.Progress) res[outcome.BuildOutcome, outcome.Fail];
```

run one planned cell through a caller-owned session, reusing its query cache, source
overlays and typed surface across calls (the editor's build path). the session's
diagnostic store is replaced for the call and its build allocator is a per-call arena
restored on return, so every raw product a caller borrowed from the session before the
call is expired by it

bp: the plan, read only
unit_index: the cell to run; out of range is an internal failure
s: the warm session; its registry, sources and queries carry over between calls
oa: owns the returned outcome
ev: progress sink for the readout; nil for none
ret: the outcome of that one unit, released with outcome.outcome_dnit, or an engine failure

## fun execute_warm_all

```mach
pub fun execute_warm_all(bp: *plan.BuildPlan, s: *session.Session, oa: *A.Allocator,
ev: *readout.Progress) res[outcome.BuildOutcome, outcome.Fail];
```

execute_warm over every planned cell in order, merged into one outcome the way
execute merges its cold units: blocked units recorded and skipped, a failed unit
recorded and the rest still run, the severity the worst unit's

bp: the plan, read only
s: the warm session
oa: owns the returned outcome
ev: progress sink for the readout; nil for none
ret: the merged outcome, released with outcome.outcome_dnit, or an engine failure

## fun set_link_config_input

```mach
pub fun set_link_config_input(p: *driver.Project, out_path: *u8, product: *u8, out_kind: u32,
ext_paths: **u8, ext_count: u32,
dynlibs: *of.DynLib, dynlib_count: u32,
image: *emit.LoadedImageOptions) err[outcome.Fail];
```

publish the link configuration of a project as the Q_LINK_CONFIG query input: the
output path, product name, link kind, external inputs, dynamic libraries with their
provider policy, and the loaded-image options, fingerprinted so an unchanged
configuration keeps its revision and any change invalidates the link

p: the project whose session holds the query db
ret: ok, or the fingerprint or query failure

