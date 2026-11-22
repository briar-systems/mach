[mach](../README.md) / [report](README.md) / **readout**

# mach.report.readout

the progress events a command reports, and the timed phase rows of a build unit

the build engine and the test runner report what happens as it happens, in order, to a sink
the command supplies, which renders each event when it arrives. the compiler does no terminal
i/o here: every rendering is the sink's. an event's payload is borrowed for the call and never
retained by the stream, except a test run's results, which stay in place from run_start to
run_end.

a phase row is timed from its phase_begin to its phase_end. a phase that runs inside another
(lower inside codegen) is carved: phase_carved marks it hosted and span_end moves each measured
span out of the host into it, so no time is counted twice. a unit's `other` time is its wall
time less every row.

a counter is a number an owner keeps of what it made, posted per module and closed into the
unit's total, a sum or the most any module reached.

Source: [`src/report/readout.mach`](../../../src/report/readout.mach)

Uses types from [`mach.base.diagnostic`](../base/diagnostic.md), [`mach.base.fail`](../base/fail.md), [`mach.base.source`](../base/source.md), [`mach.report.testrun`](testrun.md).

| Type | | |
| :--- | :--- | :--- |
| [`Phase`](#phase) | `def` | a readout phase, an index into the phase table |
| [`Noun`](#noun) | `def` | what a phase row counts, an index into the noun table |
| [`Counter`](#counter) | `def` | a readout counter, an index into the counter table |
| [`Group`](#group) | `def` | the line a person reads a counter on: what the code came to, or what building it took of memory |
| [`Unit`](#unit) | `rec` | a unit of the plan starts: what its goal calls the work (`building`, `checking`), the artifact, the target and the profile |
| [`PhaseEnd`](#phaseend) | `rec` | a phase row closes: the count it processed, the noun that counts it, its time, the threads that time is summed over (0 for a row of the unit's own time), the modules it dropped as target-gated, and under LEVEL_ITEMS its slowest item, nil when it had fewer than two |
| [`Item`](#item) | `rec` | one module or file a phase finished, when it finished |
| [`Tally`](#tally) | `rec` | a counter's value: one module's under LEVEL_ITEMS, named by it, or the unit's total under LEVEL_PHASES, with a nil name |
| [`Closed`](#closed) | `def` | what a unit's summary closes: an artifact built, a check, a test build's dispatcher |
| [`Summary`](#summary) | `rec` | a unit finished: what it closed, its artifact, target and profile, the output path, the modules in it, the compiled units the build made code for of the units it holds, whether the output was written, its size in bytes when the output is one file, whether phase rows were kept and so `other` holds the time no row accounts for, and the unit's wall time |
| [`Advance`](#advance) | `rec` | a compile finished one more of its units, sent to a live sink at most every LIVE_EVERY |
| [`TestResult`](#testresult) | `rec` | one test's result, filled in place when the test finishes |
| [`RunStart`](#runstart) | `rec` | a test run starts over count tests in collection order, each with its art set and none done |
| [`RunEnd`](#runend) | `rec` | a test run ended: its results in collection order, the counts of passed, failed, skipped and timed-out tests among those that finished, the run's wall time, and the gate tally over every finished result |
| [`Growth`](#growth) | `rec` | a growing cycle of instances being expanded: the module that declares its templates, the templates on it, and how many instances of it the module being typed has made |
| [`Event`](#event) | `tag` | one event of a command, in the order it happened |
| [`Sink`](#sink) | `rec` | where a build's events go: on_event is called once per event, in order, never from two threads at once, and may be called from a worker thread |
| [`Instant`](#instant) | `def` | an instant a readout measures from: absent when the platform clock refused the sample, so a duration measured from it is zero rather than invented |
| [`Progress`](#progress) | `rec` | one unit's progress: its wall time, and its phase rows, timed as they run and sent to the sink as each closes when `rows` |

| On `Phase` | |
| :--- | :--- |
| [`LOAD`](#phase), [`RESOLVE`](#phase), [`SEMA`](#phase), [`LOWER`](#phase), [`CODEGEN`](#phase), [`EMIT`](#phase), [`LINK`](#phase), [`CACHE`](#phase) | |
| [`TEST_LOWER`](#phase) | a test build's test objects, lowered against their modules' normal objects |
| [`TEST_CODEGEN`](#phase) | a test build's test objects, generated |
| [`STEPS`](#phase) | the build steps a project and its dependencies declare |
| [`VERIFY`](#phase) | the lowered modules held to mirl's verifier |
| [`TEST_VERIFY`](#phase) | the lowered test objects held to mirl's verifier |
| [`PASSES`](#phase) | mirl's pass schedule, the target's lowering rows and the neutral ones, over normal and test objects alike |
| [`SELECT`](#phase) | mirl's instruction selection |
| [`ALLOC`](#phase) | mirl's register allocation |
| [`FRAME`](#phase) | mirl's frame and the steps that finish framed code |
| [`MIRL_EMIT`](#phase) | mirl's block layout, variable ranges and texts, less masc's two |
| [`LAYOUT`](#phase) | masc laying out the object |
| [`ENCODE`](#phase) | masc encoding the settled layout |
| [`TARGET`](#phase) | the build's target resolved and its back half opened: mink's formats, mirl's assembled and readied target |
| [`PHASE_NONE`](#phase) | no phase: what an item no phase row took is shown under |
| [`label_of`](#label_of) | the row label of phase ph |

| On `Noun` | |
| :--- | :--- |
| [`NOUN_TARGETS`](#noun), [`NOUN_MODULES`](#noun), [`NOUN_OBJECTS`](#noun), [`NOUN_BINARIES`](#noun), [`NOUN_ARCHIVES`](#noun), [`NOUN_LIBRARIES`](#noun), [`NOUN_STEPS`](#noun) | |
| [`noun_of`](#noun_of) | noun n as a count of `count` takes it |

| On `Counter` | |
| :--- | :--- |
| [`FUNCTIONS`](#counter) | the functions of the lowered modules, as mirl's module holds them |
| [`IR_INSTRUCTIONS`](#counter) | the instructions lowering built into the modules' bodies, as mirl's bodies hold them |
| [`VERIFY_RUNS`](#counter) | the runs of mirl's verifier over the lowered modules, mach's and mirl's own |
| [`ALLOCATED_BLOCKS`](#counter) | the blocks a step allocated from its own heap, which the lowered module lives on, as std's heap counts them |
| [`ALLOCATED_BYTES`](#counter) | the bytes of those allocations |
| [`HEAP_PEAK`](#counter) | the most bytes a step's own heap mapped, as for the blocks |
| [`SELECTED`](#counter) | the machine instructions mirl's selection made |
| [`ALLOCATED`](#counter) | the machine instructions after mirl's register allocation |
| [`ALLOC_ROUNDS`](#counter) | the rounds of mirl's register allocation |
| [`counter_group`](#counter_group) | the group counter c is shown on |
| [`counter_word`](#counter_word) | the word counter c takes on its group's line for a value of `count` |
| [`counter_label`](#counter_label) | the label of counter c |
| [`counter_bytes`](#counter_bytes) | whether counter c counts bytes |

| On `Group` | |
| :--- | :--- |
| [`GROUP_CODE`](#group), [`GROUP_MEMORY`](#group) | |

| On `Closed` | |
| :--- | :--- |
| [`CLOSED_BUILT`](#closed), [`CLOSED_CHECKED`](#closed), [`CLOSED_TESTS`](#closed) | |

| On `TestResult` | |
| :--- | :--- |
| [`span`](#span) | the wall span of the tests from `first` to `last`, from the earliest launch to the latest finish, zero when none of them was launched at a sampled instant |

| On `Sink` | |
| :--- | :--- |
| [`unit`](#unit-1) | a unit starts; a nil sink is a no-op |
| [`failure`](#failure) | a failure reached no store; a nil sink is a no-op |
| [`diagnostics`](#diagnostics) | a unit's diagnostics, sent when it has any; a nil sink is a no-op |
| [`growth`](#growth-1) | a growing cycle expanded to `instances` instances, sent under the phase readout; a nil sink is a no-op |
| [`run_start`](#run_start) | a test run starts; a nil sink is a no-op |
| [`test_end`](#test_end) | the test at position at finished, its result filled and marked done; a nil sink is a no-op |
| [`run_end`](#run_end) | a test run ended; a nil sink is a no-op |

| On `Instant` | |
| :--- | :--- |
| [`sample`](#sample) | the instant now, or none when the clock refused |
| [`elapsed`](#elapsed) | the time since start, zero when either sample is absent |

| On `Progress` | |
| :--- | :--- |
| [`start`](#start) | start a unit's progress over sink, timed from now |
| [`phase_begin`](#phase_begin) | phase ph's row opens now; its time runs to phase_end, less what is carved out of it |
| [`phase_carved`](#phase_carved) | phase ph runs inside another phase and is timed by the spans moved into it |
| [`phase_summed`](#phase_summed) | phase ph runs on up to `threads` threads side by side, timed by the sum of what each spent (summed_into): its row is not part of the unit's wall time and says how many threads it sums |
| [`summed_into`](#summed_into) | the time d one thread spent in the summed phase ph |
| [`phase_dropped`](#phase_dropped) | phase ph dropped n modules as target-gated |
| [`span_begin`](#span_begin) | the start of a span that span_end moves into a hosted phase |
| [`span_end`](#span_end) | the span from start belongs to ph, and leaves the phase host that ran it |
| [`span_into`](#span_into) | the span from start belongs to the hosted phase ph, and was spent outside every other row |
| [`carve`](#carve) | the time d belongs to ph, and leaves the phase host that spent it |
| [`item_begin`](#item_begin) | the start of an item, sampled only when the sink takes items |
| [`rows`](#rows) | whether a progress keeps phase rows, which every phase, span and item call asks |
| [`advance`](#advance-1) | a compile finished `done` of its `total` units, the last of them `name`: sent to a live sink when LIVE_EVERY has passed since the last it was sent |
| [`item`](#item-1) | name finished phase ph, measured from start |
| [`item_spent`](#item_spent) | name spent d in phase ph, measured by its caller |
| [`count`](#count) | the module name's value of counter c, added to the unit's total and sent under LEVEL_ITEMS |
| [`counts_end`](#counts_end) | send the unit's total of every counter posted since the last close, then start them over |
| [`phase_end`](#phase_end) | close phase ph's row: count is what it processed, counted by noun |
| [`summary`](#summary-1) | the unit finished: its wall time is measured here, and the time no row accounts for, which floors at zero, is set when the progress kept rows |

| Functions | |
| :--- | :--- |
| [`label_width`](#label_width) | the widest row label of the phase table |
| [`noun_width`](#noun_width) | the widest spelling of the noun table |

| Constants | |
| :--- | :--- |
| [`STAGE_COUNT`](#constants) | mirl's and masc's stages, the rows a module's step carves out of its codegen row, in the order mirl runs them |
| [`STAGES`](#constants), [`COUNTER_COUNT`](#constants) | |
| [`LEVEL_RESULTS`](#constants) | what a sink asks to receive |
| [`LEVEL_PHASES`](#constants), [`LEVEL_ITEMS`](#constants) | |

---

## Phase

```mach
pub def Phase: u32
```

a readout phase, an index into the phase table

| Name | Value | |
| :--- | :--- | :--- |
| `LOAD` | `0` |  |
| `RESOLVE` | `1` |  |
| `SEMA` | `2` |  |
| `LOWER` | `3` |  |
| `CODEGEN` | `4` |  |
| `EMIT` | `5` |  |
| `LINK` | `6` |  |
| `CACHE` | `7` |  |
| `TEST_LOWER` | `8` | a test build's test objects, lowered against their modules' normal objects |
| `TEST_CODEGEN` | `9` | a test build's test objects, generated |
| `STEPS` | `10` | the build steps a project and its dependencies declare |
| `VERIFY` | `11` | the lowered modules held to mirl's verifier |
| `TEST_VERIFY` | `12` | the lowered test objects held to mirl's verifier |
| `PASSES` | `13` | mirl's pass schedule, the target's lowering rows and the neutral ones, over normal and test objects alike |
| `SELECT` | `14` | mirl's instruction selection |
| `ALLOC` | `15` | mirl's register allocation |
| `FRAME` | `16` | mirl's frame and the steps that finish framed code |
| `MIRL_EMIT` | `17` | mirl's block layout, variable ranges and texts, less masc's two |
| `LAYOUT` | `18` | masc laying out the object |
| `ENCODE` | `19` | masc encoding the settled layout |
| `TARGET` | `20` | the build's target resolved and its back half opened: mink's formats, mirl's assembled and readied target |
| `PHASE_NONE` | `0xffffffff` | no phase: what an item no phase row took is shown under |

### label_of

```mach
pub fun label_of(ph: Phase) str
```

the row label of phase ph

## Noun

```mach
pub def Noun: u8
```

what a phase row counts, an index into the noun table

| Name | Value | |
| :--- | :--- | :--- |
| `NOUN_TARGETS` | `0` |  |
| `NOUN_MODULES` | `1` |  |
| `NOUN_OBJECTS` | `2` |  |
| `NOUN_BINARIES` | `3` |  |
| `NOUN_ARCHIVES` | `4` |  |
| `NOUN_LIBRARIES` | `5` |  |
| `NOUN_STEPS` | `6` |  |

### noun_of

```mach
pub fun noun_of(n: Noun, count: u64) str
```

noun n as a count of `count` takes it

## Counter

```mach
pub def Counter: u32
```

a readout counter, an index into the counter table

| Name | Value | |
| :--- | :--- | :--- |
| `FUNCTIONS` | `0` | the functions of the lowered modules, as mirl's module holds them |
| `IR_INSTRUCTIONS` | `1` | the instructions lowering built into the modules' bodies, as mirl's bodies hold them |
| `VERIFY_RUNS` | `2` | the runs of mirl's verifier over the lowered modules, mach's and mirl's own |
| `ALLOCATED_BLOCKS` | `3` | the blocks a step allocated from its own heap, which the lowered module lives on, as std's heap counts them |
| `ALLOCATED_BYTES` | `4` | the bytes of those allocations |
| `HEAP_PEAK` | `5` | the most bytes a step's own heap mapped, as for the blocks |
| `SELECTED` | `6` | the machine instructions mirl's selection made |
| `ALLOCATED` | `7` | the machine instructions after mirl's register allocation |
| `ALLOC_ROUNDS` | `8` | the rounds of mirl's register allocation |

### counter_group

```mach
pub fun counter_group(c: Counter) Group
```

the group counter c is shown on

### counter_word

```mach
pub fun counter_word(c: Counter, count: u64) str
```

the word counter c takes on its group's line for a value of `count`

### counter_label

```mach
pub fun counter_label(c: Counter) str
```

the label of counter c

### counter_bytes

```mach
pub fun counter_bytes(c: Counter) bool
```

whether counter c counts bytes

## Group

```mach
pub def Group: u8
```

the line a person reads a counter on: what the code came to, or what building it took of memory

| Name | Value | |
| :--- | :--- | :--- |
| `GROUP_CODE` | `0` |  |
| `GROUP_MEMORY` | `1` |  |

## Unit

```mach
pub rec Unit {
    artifact: str;
    target:   str;
    profile:  str;
    verb:     str;
}
```

a unit of the plan starts: what its goal calls the work (`building`,
`checking`), the artifact, the target and the profile

## PhaseEnd

```mach
pub rec PhaseEnd {
    phase:           Phase;
    count:           u32;
    noun:            Noun;
    elapsed:         std.chrono.duration.Duration;
    threads:         u32;
    dropped:         u32;
    slowest:         str;
    slowest_elapsed: std.chrono.duration.Duration;
}
```

a phase row closes: the count it processed, the noun that counts it, its
time, the threads that time is summed over (0 for a row of the unit's own
time), the modules it dropped as target-gated, and under LEVEL_ITEMS its
slowest item, nil when it had fewer than two

## Item

```mach
pub rec Item {
    phase:   Phase;
    name:    str;
    elapsed: std.chrono.duration.Duration;
}
```

one module or file a phase finished, when it finished

## Tally

```mach
pub rec Tally {
    counter: Counter;
    name:    str;
    value:   u64;
}
```

a counter's value: one module's under LEVEL_ITEMS, named by it, or the unit's total
under LEVEL_PHASES, with a nil name

## Closed

```mach
pub def Closed: u8
```

what a unit's summary closes: an artifact built, a check, a test build's dispatcher

| Name | Value | |
| :--- | :--- | :--- |
| `CLOSED_BUILT` | `0` |  |
| `CLOSED_CHECKED` | `1` |  |
| `CLOSED_TESTS` | `2` |  |

## Summary

```mach
pub rec Summary {
    closed:   Closed;
    artifact: str;
    target:   str;
    profile:  str;
    out:      str;
    modules:  u32;
    compiled: u32;
    units:    u32;
    written:  bool;
    bytes:    usize;
    has_size: bool;
    rows:     bool;
    other:    std.chrono.duration.Duration;
    total:    std.chrono.duration.Duration;
}
```

a unit finished: what it closed, its artifact, target and profile, the output path, the
modules in it, the compiled units the build made code for of the units it holds, whether
the output was written, its size in bytes when the output is one file, whether phase rows
were kept and so `other` holds the time no row accounts for, and the unit's wall time

## Advance

```mach
pub rec Advance {
    done:  u32;
    total: u32;
    name:  str;
}
```

a compile finished one more of its units, sent to a live sink at most every LIVE_EVERY

## TestResult

```mach
pub rec TestResult {
    art:       *mach.report.testrun.TestArtifact;
    done:      bool;
    result:    mach.report.testrun.GateResult;
    code:      i32;
    start:     Instant;
    elapsed:   std.chrono.duration.Duration;
    bound:     std.chrono.duration.Duration;
    out:       *u8;
    out_len:   usize;
    truncated: bool;
    log:       str;
    launch:    str;
}
```

one test's result, filled in place when the test finishes. start is the instant it
was launched, none for a test that never ran or when the clock refused, and elapsed
its wall time from then. log is the file its captured output is kept in, nil when
none could be named. launch is the command the test was launched through, its
runner's words and the program before the test's index, nil when the program ran
itself

### span

```mach
pub fun span(tests: *TestResult, first: u32, last: u32) std.chrono.duration.Duration
```

the wall span of the tests from `first` to `last`, from the earliest launch to the latest
finish, zero when none of them was launched at a sampled instant

| Parameter | |
| :--- | :--- |
| `tests` | the results, indexed by test |
| `first` | the index of the first test of the span |
| `last` | the index of the last test of the span, inclusive |
| **returns** | the time from the earliest launch to the latest finish |

## RunStart

```mach
pub rec RunStart {
    tests: *TestResult;
    count: u32;
}
```

a test run starts over count tests in collection order, each with its art set
and none done. tests stays valid and in place until the run's run_end, so a
renderer may read every result as it finishes

## RunEnd

```mach
pub rec RunEnd {
    tests:     *TestResult;
    count:     u32;
    passed:    u32;
    failed:    u32;
    skipped:   u32;
    timed_out: u32;
    wall:      std.chrono.duration.Duration;
    tally:     *mach.report.testrun.GateTally;
}
```

a test run ended: its results in collection order, the counts of passed,
failed, skipped and timed-out tests among those that finished, the run's wall
time, and the gate tally over every finished result

## Growth

```mach
pub rec Growth {
    module:    str;
    members:   str;
    instances: u32;
}
```

a growing cycle of instances being expanded: the module that declares its templates,
the templates on it, and how many instances of it the module being typed has made

## Event

```mach
pub tag Event: u8 {
    unit:        Unit;
    phase:       PhaseEnd;
    item:        Item;
    tally:       Tally;
    summary:     Summary;
    advance:     Advance;
    failure:     Failure;
    diagnostics: Diagnostics;
    run_start:   RunStart;
    test_end:    TestEnd;
    run_end:     RunEnd;
    growth:      Growth;
}
```

one event of a command, in the order it happened. a new kind is a new member
here and a new arm in each renderer, which skips a member it does not render

## Sink

```mach
pub rec Sink {
    ctx:      ptr;
    on_event: fun(ptr, *Event);
    level:    u8;
    live:     bool;
}
```

where a build's events go: on_event is called once per event, in order, never
from two threads at once, and may be called from a worker thread. level is
what the sink asks to receive, and live whether it shows advances as they happen

### unit

```mach
pub fun unit(sink: *Sink, artifact: str, target: str, profile: str, verb: str)
```

a unit starts; a nil sink is a no-op

### failure

```mach
pub fun failure(sink: *Sink, f: *mach.base.fail.Fail, origin: mach.base.diagnostic.Origin)
```

a failure reached no store; a nil sink is a no-op

### diagnostics

```mach
pub fun diagnostics(sink: *Sink, sources: *mach.base.source.SourceMap, diags: *mach.base.diagnostic.DiagnosticStore)
```

a unit's diagnostics, sent when it has any; a nil sink is a no-op

### growth

```mach
pub fun growth(sink: *Sink, module: str, members: str, instances: u32)
```

a growing cycle expanded to `instances` instances, sent under the phase readout; a nil
sink is a no-op

### run_start

```mach
pub fun run_start(sink: *Sink, tests: *TestResult, count: u32)
```

a test run starts; a nil sink is a no-op

### test_end

```mach
pub fun test_end(sink: *Sink, tests: *TestResult, at: u32)
```

the test at position at finished, its result filled and marked done; a nil
sink is a no-op

### run_end

```mach
pub fun run_end(sink: *Sink, end: RunEnd)
```

a test run ended; a nil sink is a no-op

## Instant

```mach
pub def Instant: opt[std.chrono.time.Instant]
```

an instant a readout measures from: absent when the platform clock refused
the sample, so a duration measured from it is zero rather than invented

### sample

```mach
pub fun sample() Instant
```

the instant now, or none when the clock refused

### elapsed

```mach
pub fun elapsed(start: Instant) std.chrono.duration.Duration
```

the time since start, zero when either sample is absent

## Progress

```mach
pub rec Progress {
    sink:       *Sink;
    rows:       bool;
    unit_start: Instant;
    unit_rows:  std.chrono.duration.Duration;
    phases:     [PHASE_COUNT]PhaseState;
    counts:     [COUNTER_COUNT]u64;
    live_at:    Instant;
}
```

one unit's progress: its wall time, and its phase rows, timed as they run and sent to the
sink as each closes when `rows`. a run with no sink has no progress, and a progress without
rows times no phase, span or item

### start

```mach
pub fun start(pr: *Progress, sink: *Sink) *Progress
```

start a unit's progress over sink, timed from now

| Parameter | |
| :--- | :--- |
| `pr` | the storage, which must outlive every use of the returned pointer |
| `sink` | the command's sink, or nil |
| **returns** | pr, or nil when there is no sink |

### phase_begin

```mach
pub fun phase_begin(pr: *Progress, ph: Phase)
```

phase ph's row opens now; its time runs to phase_end, less what is carved out of it

### phase_carved

```mach
pub fun phase_carved(pr: *Progress, ph: Phase)
```

phase ph runs inside another phase and is timed by the spans moved into it

### phase_summed

```mach
pub fun phase_summed(pr: *Progress, ph: Phase, threads: u32)
```

phase ph runs on up to `threads` threads side by side, timed by the sum of what each spent
(summed_into): its row is not part of the unit's wall time and says how many threads it sums

### summed_into

```mach
pub fun summed_into(pr: *Progress, ph: Phase, d: std.chrono.duration.Duration)
```

the time d one thread spent in the summed phase ph

### phase_dropped

```mach
pub fun phase_dropped(pr: *Progress, ph: Phase, n: u32)
```

phase ph dropped n modules as target-gated

### span_begin

```mach
pub fun span_begin(pr: *Progress) Instant
```

the start of a span that span_end moves into a hosted phase

### span_end

```mach
pub fun span_end(pr: *Progress, ph: Phase, host: Phase, start: Instant)
```

the span from start belongs to ph, and leaves the phase host that ran it

### span_into

```mach
pub fun span_into(pr: *Progress, ph: Phase, start: Instant)
```

the span from start belongs to the hosted phase ph, and was spent outside every other row

### carve

```mach
pub fun carve(pr: *Progress, ph: Phase, host: Phase, d: std.chrono.duration.Duration)
```

the time d belongs to ph, and leaves the phase host that spent it

### item_begin

```mach
pub fun item_begin(pr: *Progress) Instant
```

the start of an item, sampled only when the sink takes items

### rows

```mach
pub fun rows(pr: *Progress) bool
```

whether a progress keeps phase rows, which every phase, span and item call asks

### advance

```mach
pub fun advance(pr: *Progress, done: u32, total: u32, name: str)
```

a compile finished `done` of its `total` units, the last of them `name`: sent to a live
sink when LIVE_EVERY has passed since the last it was sent

### item

```mach
pub fun item(pr: *Progress, ph: Phase, name: str, start: Instant)
```

name finished phase ph, measured from start

### item_spent

```mach
pub fun item_spent(pr: *Progress, ph: Phase, name: str, d: std.chrono.duration.Duration)
```

name spent d in phase ph, measured by its caller

### count

```mach
pub fun count(pr: *Progress, c: Counter, name: str, value: u64)
```

the module name's value of counter c, added to the unit's total and sent under LEVEL_ITEMS

### counts_end

```mach
pub fun counts_end(pr: *Progress)
```

send the unit's total of every counter posted since the last close, then start them over

### phase_end

```mach
pub fun phase_end(pr: *Progress, ph: Phase, count: u32, noun: Noun)
```

close phase ph's row: count is what it processed, counted by noun

### summary

```mach
pub fun summary(pr: *Progress, sum: Summary)
```

the unit finished: its wall time is measured here, and the time no row accounts for, which
floors at zero, is set when the progress kept rows. a nil progress is a no-op

## Functions

### label_width

```mach
pub fun label_width() usize
```

the widest row label of the phase table

### noun_width

```mach
pub fun noun_width() usize
```

the widest spelling of the noun table

## Constants

| Name | Type | Value | |
| :--- | :--- | :--- | :--- |
| `STAGE_COUNT` | `usize` | `7` | mirl's and masc's stages, the rows a module's step carves out of its codegen row, in the order mirl runs them |
| `STAGES` | `[STAGE_COUNT]Phase` | `[STAGE_COUNT]Phase{PASSES, SELECT, ALLOC, FRAME, MIRL_EMIT, LAYOUT, ENCODE}` |  |
| `COUNTER_COUNT` | `usize` | `9` |  |
| `LEVEL_RESULTS` | `u8` | `0` | what a sink asks to receive |
| `LEVEL_PHASES` | `u8` | `1` |  |
| `LEVEL_ITEMS` | `u8` | `2` |  |

