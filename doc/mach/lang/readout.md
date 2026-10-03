# mach.lang.readout

## def Phase

```mach
pub def Phase: u32
```

a readout phase, an index into the phase table

## val LOAD

```mach
pub val LOAD:     Phase = 0
```

## val RESOLVE

```mach
pub val RESOLVE:  Phase = 1
```

## val SEMA

```mach
pub val SEMA:     Phase = 2
```

## val LOWER

```mach
pub val LOWER:    Phase = 3
```

## val OPTIMIZE

```mach
pub val OPTIMIZE: Phase = 4
```

## val CODEGEN

```mach
pub val CODEGEN:  Phase = 5
```

## val EMIT

```mach
pub val EMIT:     Phase = 6
```

## val LINK

```mach
pub val LINK:     Phase = 7
```

## val CACHE

```mach
pub val CACHE:    Phase = 8
```

## val TEST_LOWER

```mach
pub val TEST_LOWER: Phase = 9
```

a test build's test objects, lowered against their modules' normal objects

## val TEST_CODEGEN

```mach
pub val TEST_CODEGEN: Phase = 10
```

a test build's test objects, generated

## val STEPS

```mach
pub val STEPS: Phase = 11
```

the build steps a project and its dependencies declare

## rec PhaseSpec

```mach
pub rec PhaseSpec;
```

what the readout shows of a phase: its row label

## fun label_of

```mach
pub fun label_of(ph: Phase) str;
```

the row label of phase ph

## val LEVEL_RESULTS

```mach
pub val LEVEL_RESULTS: u8 = 0
```

what a sink asks to receive. every level receives the unit, failure,
diagnostics and test run events; phases adds the phase rows and the unit
summary, items adds one event per module or file a phase processed. a test
renderer reads the level for how much of each test it shows

## val LEVEL_PHASES

```mach
pub val LEVEL_PHASES:  u8 = 1
```

## val LEVEL_ITEMS

```mach
pub val LEVEL_ITEMS:   u8 = 2
```

## rec Unit

```mach
pub rec Unit;
```

a unit of the plan starts: what its goal calls the work (`building`,
`checking`), the artifact, or none for the whole project, the target and the
profile

## rec PhaseEnd

```mach
pub rec PhaseEnd;
```

a phase row closes: the count it processed, in the noun that counts it, its
time, the workers it ran on, the modules it dropped as target-gated, and under
LEVEL_ITEMS its slowest item, nil when it had fewer than two

## rec Item

```mach
pub rec Item;
```

one module or file a phase finished, when it finished

## rec Summary

```mach
pub rec Summary;
```

a unit built its output: the path, the modules in it, its size in bytes when
the output is one file, the time no row accounts for, and the unit's wall time

## rec Failure

```mach
pub rec Failure;
```

a failure that reached no diagnostic store, and the phase it is reported under

## rec Diagnostics

```mach
pub rec Diagnostics;
```

a unit's diagnostics with the sources they point into

## rec TestResult

```mach
pub rec TestResult;
```

one test's result, filled in place when the test finishes. log is the file its
captured output is kept in, nil when none could be named

## rec RunStart

```mach
pub rec RunStart;
```

a test run starts over count tests in collection order, each with its art set
and none done, launched through runner when it is not nil. tests stays valid
and in place until the run's run_end, so a renderer may read every result as
it finishes

## rec TestEnd

```mach
pub rec TestEnd;
```

the test at position at of the run finished, whatever its outcome

## rec RunEnd

```mach
pub rec RunEnd;
```

a test run ended: its results in collection order, the counts of passed,
failed and timed-out tests, the run's wall time, and the gate tally over every
result

## tag Event

```mach
pub tag Event: u8 {
    unit:        Unit;
    phase:       PhaseEnd;
    item:        Item;
    summary:     Summary;
    failure:     Failure;
    diagnostics: Diagnostics;
    run_start:   RunStart;
    test_end:    TestEnd;
    run_end:     RunEnd;
}
```

one event of a command, in the order it happened. a new kind is a new member
here and a new arm in each renderer, which skips a member it does not render

## rec Sink

```mach
pub rec Sink;
```

where a build's events go: on_event is called once per event, in order, never
from two threads at once, and may be called from a worker thread. level is
what the sink asks to receive

## fun unit

```mach
pub fun unit(sink: *Sink, artifact: str, target: str, profile: str, verb: str, has_artifact: bool);
```

a unit starts; a nil sink is a no-op

## fun failure

```mach
pub fun failure(sink: *Sink, f: *fail.Fail, origin: diagnostic.Origin);
```

a failure reached no store; a nil sink is a no-op

## fun diagnostics

```mach
pub fun diagnostics(sink: *Sink, sources: *lang_source.SourceMap, diags: *diagnostic.DiagnosticStore);
```

a unit's diagnostics, sent when it has any; a nil sink is a no-op

## fun run_start

```mach
pub fun run_start(sink: *Sink, tests: *TestResult, count: u32, runner: str);
```

a test run starts; a nil sink is a no-op

## fun test_end

```mach
pub fun test_end(sink: *Sink, tests: *TestResult, at: u32);
```

the test at position at finished, its result filled and marked done; a nil
sink is a no-op

## fun run_end

```mach
pub fun run_end(sink: *Sink, end: RunEnd);
```

a test run ended; a nil sink is a no-op

## def Instant

```mach
pub def Instant: opt[time.Instant]
```

an instant a readout measures from: absent when the platform clock refused
the sample, so a duration measured from it is zero rather than invented

## fun sample

```mach
pub fun sample() Instant;
```

the instant now, or none when the clock refused

## fun elapsed

```mach
pub fun elapsed(start: Instant) chrono_duration.Duration;
```

the time since start, zero when either sample is absent

## rec Progress

```mach
pub rec Progress;
```

one unit's phase rows, timed as they run and sent to the sink as each closes.
a parallel section's workers post their items to a queue that whichever
thread holds the drain delivers, so a worker never waits on the sink

## fun start

```mach
pub fun start(pr: *Progress, sink: *Sink) *Progress;
```

start a unit's progress over sink, timed from now

pr: the storage, which must outlive every use of the returned pointer
sink: the command's sink, or nil
ret: pr, or nil when the sink takes no phase rows, which every phase call reads as none

## fun phase_begin

```mach
pub fun phase_begin(pr: *Progress, ph: Phase);
```

phase ph's row opens now; its time runs to phase_end, less what is carved out of it

## fun phase_carved

```mach
pub fun phase_carved(pr: *Progress, ph: Phase);
```

phase ph runs inside another phase and is timed by the spans moved into it

## fun phase_workers

```mach
pub fun phase_workers(pr: *Progress, ph: Phase, n: u32);
```

phase ph ran on n workers

## fun phase_dropped

```mach
pub fun phase_dropped(pr: *Progress, ph: Phase, n: u32);
```

phase ph dropped n modules as target-gated

## fun span_begin

```mach
pub fun span_begin(pr: *Progress) Instant;
```

the start of a span that span_end moves into a hosted phase

## fun span_end

```mach
pub fun span_end(pr: *Progress, ph: Phase, host: Phase, start: Instant);
```

the span from start belongs to ph, and leaves the phase host that ran it

## fun item_begin

```mach
pub fun item_begin(pr: *Progress) Instant;
```

the start of an item, sampled only when the sink takes items

## fun item

```mach
pub fun item(pr: *Progress, ph: Phase, name: str, start: Instant);
```

name finished phase ph, measured from start

## fun workers_begin

```mach
pub fun workers_begin(pr: *Progress, a: *A.Allocator, n: usize) err[fail.Fail];
```

open the item queue of a parallel section that posts at most n items. the
queue's storage is drawn from a and returned by workers_end

## fun item_post

```mach
pub fun item_post(pr: *Progress, ph: Phase, name: str, d: chrono_duration.Duration);
```

a worker thread's item: queued, then delivered by this thread unless another
holds the drain, which then delivers it. name must outlive the section

## fun workers_end

```mach
pub fun workers_end(pr: *Progress, a: *A.Allocator, ph: Phase, n: u32);
```

close a parallel section of phase ph that ran on n workers, after every worker
has joined: every posted item still queued is delivered and the queue's storage
returned

## fun phase_end

```mach
pub fun phase_end(pr: *Progress, ph: Phase, count: u32, noun_one: str, noun_many: str);
```

close phase ph's row: count is what it processed, named by noun_one or noun_many

## fun summary

```mach
pub fun summary(pr: *Progress, out: str, modules: u32, bytes: usize, has_size: bool);
```

the unit built out: the time no row accounts for floors at zero

