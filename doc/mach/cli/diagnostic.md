[mach](../README.md) / [cli](README.md) / **diagnostic**

# mach.cli.diagnostic

how a command reports: diagnostics, failures, the build readout and test results

a command's `Report` writes to stderr as human text or, under `--report json` or `--report lines`,
one record a line closed by a summary record. a `Readout` renders the engine's and the
test runner's events as they arrive, by the readout contract (doc/language/readout.md).

Source: [`src/cli/diagnostic.mach`](../../../src/cli/diagnostic.mach)

Uses types from [`mach.base.diagnostic`](../base/diagnostic.md), [`mach.base.diagnostic.kind`](../base/diagnostic/kind.md), [`mach.base.fail`](../base/fail.md), [`mach.base.source`](../base/source.md), [`mach.build.outcome`](../build/outcome.md), [`mach.package.report`](../package/report.md), [`mach.report.layout`](../report/layout.md), [`mach.report.readout`](../report/readout.md), [`mach.report.testrun`](../report/testrun.md).

| Type | | |
| :--- | :--- | :--- |
| [`Readout`](#readout) | `rec` | every command's build and test events rendered the moment each arrives: the renderer of the readout contract (doc/language/readout.md) |
| [`Format`](#format) | `def` | how a command reports to stderr: `--report human`, the default, or the records in one of their two encodings, `--report json` or `--report lines` |
| [`Report`](#report) | `rec` | how a build-shaped command reports to stderr, and what it has reported |

| On `Readout` | |
| :--- | :--- |
| [`readout_begin`](#readout_begin) | start rendering a command's events to r, at the level and quietness its command line set |
| [`readout_unit`](#readout_unit) | the unit a test run that follows belongs to, which its results name; each name borrowed until the run ends |
| [`readout_end`](#readout_end) | close a plan's rendering: the live line goes |

| On `Format` | |
| :--- | :--- |
| [`FORMAT_HUMAN`](#format) | |
| [`format_named`](#format_named) | the format a `--report` value names |

| On `Report` | |
| :--- | :--- |
| [`report_store`](#report_store) | render every diagnostic of a store through a command's report: as text, counted into the report's errors and warnings, or a record each under json |
| [`dependency_report`](#dependency_report) | where a dependency command reports: results on standard output, notes and failures, the ones the command goes on past too, through the command's report, and each fetch on the live line and in the log |
| [`report_level`](#report_level) | the readout level and quietness the command line asked for |
| [`results_shown`](#results_shown) | whether a command's results are written: human text without `--quiet` |
| [`log_stage`](#log_stage) | a stage of a command that is not a build closed: under -v its log line, `[t] label  time  detail` |
| [`log_item`](#log_item) | one item of a stage finished: under -vv its line, `[t]   label  time  name` |
| [`live_begin`](#live_begin) | the live line shows the stage `label` running from now, on a terminal and without `--quiet` |
| [`live_reached`](#live_reached) | the running stage has done `done` of `total` and reached `name`, 0 of 0 for a stage that counts nothing |
| [`live_end`](#live_end) | nothing runs now, and the live line goes |
| [`is_records`](#is_records) | whether the report writes records rather than human text |
| [`report_init`](#report_init) | start a report in place; it holds its own arena, so it stays where it was made until `report_dnit` |
| [`out`](#out) | where a command writes its output, stdout through the report, so a refused write fails the command as the system's |
| [`report_note`](#report_note) | report a note of kind `k`, which points at nothing: `note text` in human text, or a diagnostic record of note severity under json |
| [`report_said`](#report_said) | report what a command did as a result in human text, and as the note record of kind `k` saying `text` under the records modes |
| [`report_fail`](#report_fail) | report a carried failure as the diagnostic it reads as, in text or as a diagnostic record under json |
| [`report_case`](#report_case) | one collected test as a record under json, as `mach test --list` lists it; human text writes nothing here |
| [`report_skip`](#report_skip) | a (target, profile) whose tests were built and not run, as a record under json; human text writes nothing here |
| [`report_dnit`](#report_dnit) | end the run: a refusal kept while it ran, such as a refused write to stdout, reported, then under json the summary record, the last line the command writes; the report's arena is released either way |

| Functions | |
| :--- | :--- |
| [`render_fail`](#render_fail) | print a Fail to stderr as the diagnostic it reads as, and map it to an exit code a reported Fail prints nothing, its diagnostics having been rendered already |
| [`refuse`](#refuse) | render_fail for a failure built in place |
| [`dependency_outcome`](#dependency_outcome) | show how a dependency command ended: its failure, then a note for each change it made before failing |
| [`outcome_code`](#outcome_code) | map a build outcome's severity to the process exit code, an unknown severity reported to stderr |
| [`shown_path`](#shown_path) | a path as a result shows it, without a leading `./` |
| [`has_capture`](#has_capture) | whether a result kept output worth pointing at |

| Constants | |
| :--- | :--- |
| [`FETCH_LABEL`](#constants) | what the dependency work calls a fetch from a remote, on the live line and in the log |

---

## Readout

```mach
pub rec Readout {
    sink:     mach.report.readout.Sink;
    r:        *Report;
    quiet:    bool;
    unit:     mach.report.readout.Unit;
    errors:   u32;
    warnings: u32;
    rows:     [readout.PHASE_COUNT]Row;
    a:        *std.allocator.Allocator;
    modules:  Vector[ModuleRow];
    last:     usize;
    counts:   [readout.COUNTER_COUNT]u64;
    held:     [readout.COUNTER_COUNT]bool;
    run:      *mach.report.readout.TestResult;
    count:    u32;
    finished: u32;
}
```

every command's build and test events rendered the moment each arrives: the renderer of the
readout contract (doc/language/readout.md). results go to stdout, the same at every
level; log lines, items, facts and diagnostics go to stderr as they happen, and a terminal
shows the live line beneath them. the records modes write each diagnostic and failure as a
record, and under -v each unit, phase row, item, fact and counter

| Field | Type | |
| :--- | :--- | :--- |
| `sink` | `mach.report.readout.Sink` | what the engine and the test runner are given; its ctx is this record, which must not move |
| `r` | `*Report` | the command's report |
| `quiet` | `bool` | `--quiet`, which leaves out the results |
| `unit` | `mach.report.readout.Unit` | the unit the events now arriving belong to |
| `errors` | `u32` | the errors the unit reported, its warnings in `warnings` |
| `rows` | `[readout.PHASE_COUNT]Row` | the unit's recap, by phase |
| `a` | `*std.allocator.Allocator` | the arena what is held lives in, nil until something is |
| `modules` | `Vector[ModuleRow]` | each module's codegen times and counters under -vv, the last one met at `last` |
| `counts` | `[readout.COUNTER_COUNT]u64` | the unit's counter totals, those `held` |
| `run` | `*mach.report.readout.TestResult` | the running test run's results, from its run_start to its run_end, `count` of them |
| `finished` | `u32` | how many of them have finished |

### readout_begin

```mach
pub fun readout_begin(rd: *Readout, r: *Report)
```

start rendering a command's events to r, at the level and quietness its command line set

| Parameter | |
| :--- | :--- |
| `rd` | the renderer, initialised in place since its sink points at it |
| `r` | the command's report |

### readout_unit

```mach
pub fun readout_unit(rd: *Readout, artifact: str, target: str, profile: str)
```

the unit a test run that follows belongs to, which its results name; each name borrowed
until the run ends

### readout_end

```mach
pub fun readout_end(rd: *Readout)
```

close a plan's rendering: the live line goes

## Format

```mach
pub def Format: u8
```

how a command reports to stderr: `--report human`, the default, or the records in one of
their two encodings, `--report json` or `--report lines`

| Name | Value | |
| :--- | :--- | :--- |
| `FORMAT_HUMAN` | `0` |  |

### format_named

```mach
pub fun format_named(name: str) opt[Format]
```

the format a `--report` value names

| Parameter | |
| :--- | :--- |
| `name` | the value |
| **returns** | the format, or none for a name outside `human`, `json` and `lines` |

## Report

```mach
pub rec Report {
    format:   Format;
    w:        std.io.writer.Writer;
    root:     str;
    errors:   usize;
    warnings: usize;
    notes:    usize;
    made:     bool;
    backing:  std.allocator.Allocator;
    ar:       std.allocator.arena.Arena;
    a:        std.allocator.Allocator;
    pb:       PathBase;
    built:    Vector[Built];
    # the texts a failure's locations name, nil while the command holds none
    sources: *mach.base.source.SourceMap;
    # the first refusal met while writing, which fails the report at its close
    refusal: opt[mach.base.fail.Fail];
    out:     std.io.writer.Writer;
    # when the command began, which a log line's stamp and a run's time count from
    origin: mach.report.readout.Instant;
    # the readout level `-v` and `-vv` set, and `--quiet`, which leaves out the results
    level: u8;
    quiet: bool;
    # the console both streams share with the live line: `lock` serialises their writes and the
    # drawer, `err_bare` and `out_bare` say the last byte written to stderr, and to stdout where
    # it is the terminal, ended a line, and `owed` that a blank line is owed before the next write
    lock:     std.sync.mutex.Mutex;
    err_bare: bool;
    out_bare: bool;
    out_tty:  bool;
    owed:     bool;
    # the live line: the slot the engine writes, whether the drawer runs and is told to stop,
    # what wakes it, its thread, the terminal's width and the line on screen, none when shown_len is 0
    live:      mach.report.readout.Live;
    drawing:   bool;
    stop:      bool;
    wake:      std.sync.condition.Condition;
    drawer:    std.sync.thread.Thread;
    width:     usize;
    shown:     [LIVE_LINE]u8;
    shown_len: usize;
}
```

how a build-shaped command reports to stderr, and what it has reported. under
json every line it writes is a record and `report_dnit` ends the run with the
summary record, whose counts are the severities of the records before it

| Field | Type | |
| :--- | :--- | :--- |
| `format` | `Format` | human text or json records |
| `w` | `std.io.writer.Writer` | where the report is written, stderr |
| `root` | `str` | the project root as the command resolved it, nil until then; a json record names a file inside it relative to it |
| `errors` | `usize` | the error records written |
| `warnings` | `usize` | the warning records written |
| `made` | `bool` | whether the arena record paths are built in, and the base they are measured from, have been made; they are on first use |
| `built` | `Vector[Built]` | under json, each unit's `built` line, which the summary record carries |
| `out` | `std.io.writer.Writer` | stdout as the command writes it, its first refused write kept as the report's refusal |

### report_store

```mach
pub fun report_store(r: *Report, sources: *mach.base.source.SourceMap, list: *mach.base.diagnostic.DiagnosticStore)
```

render every diagnostic of a store through a command's report: as text, counted into the
report's errors and warnings, or a record each under json

| Parameter | |
| :--- | :--- |
| `r` | the command's report |
| `sources` | the SourceMap that locates each diagnostic |
| `list` | the diagnostics, in order |

### dependency_report

```mach
pub fun dependency_report(a: *std.allocator.Allocator, r: *Report) mach.package.report.Report
```

where a dependency command reports: results on standard output, notes and failures, the
ones the command goes on past too, through the command's report, and each fetch on the live
line and in the log

| Parameter | |
| :--- | :--- |
| `a` | owns the report's effect notes |
| `r` | the command's report, in the format `--report` selected |
| **returns** | the report, released with `report.dnit` |

### report_level

```mach
pub fun report_level(r: *Report, level: u8, quiet: bool)
```

the readout level and quietness the command line asked for

| Parameter | |
| :--- | :--- |
| `r` | the command's report |
| `level` | LEVEL_RESULTS to LEVEL_ITEMS, as `-v` and `-vv` set it |
| `quiet` | `--quiet` |

### results_shown

```mach
pub fun results_shown(r: *Report) bool
```

whether a command's results are written: human text without `--quiet`

### log_stage

```mach
pub fun log_stage(r: *Report, label: str, d: std.chrono.duration.Duration, detail: str)
```

a stage of a command that is not a build closed: under -v its log line, `[t] label  time  detail`

| Parameter | |
| :--- | :--- |
| `r` | the command's report |
| `label` | the stage, a word of at most LABEL_W bytes |
| `d` | the time it took |
| `detail` | what it did, as `2 files` |

### log_item

```mach
pub fun log_item(r: *Report, label: str, d: std.chrono.duration.Duration, name: str)
```

one item of a stage finished: under -vv its line, `[t]   label  time  name`

| Parameter | |
| :--- | :--- |
| `r` | the command's report |
| `label` | the stage it belongs to |
| `d` | the time it took |
| `name` | what it was |

### live_begin

```mach
pub fun live_begin(r: *Report, label: str)
```

the live line shows the stage `label` running from now, on a terminal and without `--quiet`

| Parameter | |
| :--- | :--- |
| `r` | the command's report |
| `label` | the stage, which must outlive it |

### live_reached

```mach
pub fun live_reached(r: *Report, done: u32, total: u32, name: str)
```

the running stage has done `done` of `total` and reached `name`, 0 of 0 for a stage that
counts nothing

### live_end

```mach
pub fun live_end(r: *Report)
```

nothing runs now, and the live line goes

### is_records

```mach
pub fun is_records(r: *Report) bool
```

whether the report writes records rather than human text

### report_init

```mach
pub fun report_init(r: *Report, format: Format)
```

start a report in place; it holds its own arena, so it stays where it was made
until `report_dnit`

| Parameter | |
| :--- | :--- |
| `r` | the report |
| `format` | the format the command writes |

### out

```mach
pub fun out(r: *Report) *std.io.writer.Writer
```

where a command writes its output, stdout through the report, so a refused write fails the
command as the system's

### report_note

```mach
pub fun report_note(r: *Report, k: mach.base.diagnostic.kind.Kind, text: str)
```

report a note of kind `k`, which points at nothing: `note text` in human text, or a
diagnostic record of note severity under json

| Parameter | |
| :--- | :--- |
| `r` | the command's report |
| `k` | the note's kind, a row of note level |
| `text` | what the note says |

### report_said

```mach
pub fun report_said(r: *Report, k: mach.base.diagnostic.kind.Kind, v: mach.report.layout.Verb, subject: str, fields: *mach.report.layout.Field, n: usize, detail: str, text: str)
```

report what a command did as a result in human text, and as the note record of kind `k`
saying `text` under the records modes

| Parameter | |
| :--- | :--- |
| `r` | the command's report |
| `k` | the note's kind, a row of note level |
| `v` | the result's verb |
| `subject` | what it acted on |
| `fields` | the result's fields, `n` of them |
| `detail` | a note after it, nil for none |
| `text` | the note record's message |

### report_fail

```mach
pub fun report_fail(r: *Report, f: mach.base.fail.Fail, origin: mach.base.diagnostic.Origin) i64
```

report a carried failure as the diagnostic it reads as, in text or as a diagnostic record
under json. a reported failure writes nothing, its diagnostics having been rendered already

| Parameter | |
| :--- | :--- |
| `r` | the command's report |
| `f` | the failure |
| `origin` | the phase it is reported under; ORIGIN_NONE for the command line itself |
| **returns** | the exit code `exit.of` maps the failure to |

### report_case

```mach
pub fun report_case(r: *Report, art: *mach.report.testrun.TestArtifact)
```

one collected test as a record under json, as `mach test --list` lists it; human text
writes nothing here

| Parameter | |
| :--- | :--- |
| `r` | the command's report |
| `art` | the test |

### report_skip

```mach
pub fun report_skip(r: *Report, target: str, profile: str, reason: str)
```

a (target, profile) whose tests were built and not run, as a record under json; human text
writes nothing here

| Parameter | |
| :--- | :--- |
| `r` | the command's report |
| `target` | the declared target name |
| `profile` | the profile name |
| `reason` | why its tests did not run |

### report_dnit

```mach
pub fun report_dnit(r: *Report, code: i64) i64
```

end the run: a refusal kept while it ran, such as a refused write to stdout,
reported, then under json the summary record, the last line the command
writes; the report's arena is released either way. a command an interrupt
stopped ends with the one line report_interrupted writes instead

| Parameter | |
| :--- | :--- |
| `r` | the command's report |
| `code` | the exit code the command returns |
| **returns** | the worse of code and the kept refusal's |

## Functions

### render_fail

```mach
pub fun render_fail(sources: *mach.base.source.SourceMap, f: *mach.base.fail.Fail) i64
```

print a Fail to stderr as the diagnostic it reads as, and map it to an exit code
a reported Fail prints nothing, its diagnostics having been rendered already

| Parameter | |
| :--- | :--- |
| `sources` | the source map the failure's places are in, nil when it can point at none |
| `f` | the Fail |
| **returns** | the exit code `exit.of` maps the failure to |

### refuse

```mach
pub fun refuse(sources: *mach.base.source.SourceMap, f: mach.base.fail.Fail) i64
```

render_fail for a failure built in place

| Parameter | |
| :--- | :--- |
| `sources` | the source map the failure's places are in, nil when it can point at none |
| `f` | the Fail |
| **returns** | the exit code `exit.of` maps the failure to |

### dependency_outcome

```mach
pub fun dependency_outcome(rep: *mach.package.report.Report, outcome: err[mach.base.fail.Fail]) i64
```

show how a dependency command ended: its failure, then a note for each change it made
before failing

| Parameter | |
| :--- | :--- |
| `rep` | the report the command used |
| `outcome` | how it ended |
| **returns** | exit.OK, or the code `exit.of` maps the failure to |

### outcome_code

```mach
pub fun outcome_code(bo: *mach.build.outcome.BuildOutcome) i64
```

map a build outcome's severity to the process exit code, an unknown severity reported to stderr

| Parameter | |
| :--- | :--- |
| `bo` | the outcome |
| **returns** | the shared exit code of the severity, exit.INTERNAL for one outside the catalog |

### shown_path

```mach
pub fun shown_path(p: str) str
```

a path as a result shows it, without a leading `./`

### has_capture

```mach
pub fun has_capture(r: *mach.report.testrun.GateResult) bool
```

whether a result kept output worth pointing at

## Constants

| Name | Type | Value | |
| :--- | :--- | :--- | :--- |
| `FETCH_LABEL` | `str` | `"fetch"` | what the dependency work calls a fetch from a remote, on the live line and in the log |

