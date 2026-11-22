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
| [`Readout`](#readout) | `rec` | every command's events rendered the moment each arrives: the renderer of the readout contract (doc/language/readout.md) |
| [`Format`](#format) | `def` | how a command reports to stderr: `--report human`, the default, or the records in one of their two encodings, `--report json` or `--report lines` |
| [`Report`](#report) | `rec` | how a build-shaped command reports to stderr, and what it has reported |

| On `Readout` | |
| :--- | :--- |
| [`readout_begin`](#readout_begin) | start rendering a command's events to r, a test run in human text to stdout |
| [`readout_end`](#readout_end) | close a plan's rendering: under human text the table a plan of several units closes with |

| On `Format` | |
| :--- | :--- |
| [`FORMAT_HUMAN`](#format) | |
| [`format_named`](#format_named) | the format a `--report` value names |

| On `Report` | |
| :--- | :--- |
| [`report_store`](#report_store) | render every diagnostic of a store through a command's report: as text, counted into the tally the report closes with, or a record each under json |
| [`dependency_report`](#dependency_report) | where a dependency command reports: progress on standard output, and notes and failures, the ones the command goes on past too, through the command's report |
| [`is_records`](#is_records) | whether the report writes records rather than human text |
| [`report_init`](#report_init) | start a report in place; it holds its own arena, so it stays where it was made until `report_dnit` |
| [`out`](#out) | where a command writes its output, stdout through the report, so a refused write fails the command as the system's |
| [`report_note`](#report_note) | report a note of kind `k`, which points at nothing: `note text` in human text, or a diagnostic record of note severity under json |
| [`report_said`](#report_said) | report what a command did as a progress line in human text, and as the note record of kind `k` saying `text` under the records modes |
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
| [`has_capture`](#has_capture) | whether a result kept output worth pointing at |
| [`said_row`](#said_row) | a progress line on w: its verb in the gutter, then `n` texts parted by GAP |

---

## Readout

```mach
pub rec Readout {
    sink:    mach.report.readout.Sink;
    r:       *Report;
    units:   usize;
    quiet:   bool;
    cell:    mach.report.readout.Unit;
    in_cell: bool;
    named:   bool;
    live:    bool;
    shown:   bool;
    a:       *std.allocator.Allocator;
    rows:    Vector[StageRow];
    modules: Vector[ModuleRow];
    done:    Vector[UnitRow];
    run:     *mach.report.readout.TestResult;
    count:   u32;
    roll:    [ROLL_COLS]mach.report.layout.Column;
    line:    [3]mach.report.layout.Column;
    counts:  [readout.COUNTER_COUNT]u64;
    held:    [readout.COUNTER_COUNT]bool;
    holding: bool;
}
```

every command's events rendered the moment each arrives: the renderer of the
readout contract (doc/language/readout.md). the report's stream, stderr,
carries the build: human text renders a unit's progress line or, among several units, the
table their plan closes with, under -v each unit's table, failures and diagnostics, and
among several units each unit's first diagnostic or failure follows a line naming the unit,
and the report counts what is rendered for its closing tally; the records modes write each diagnostic and failure as
a record, and under -v each unit, phase row, item and counter. stdout carries a test run in
human text: a module's row once its last test has finished, or each test under -v, then each
failure once and the verdict. under the records modes the run is also records on the
report's stream: run_start, a test record per finished test, and run_end

| Field | Type | |
| :--- | :--- | :--- |
| `sink` | `mach.report.readout.Sink` | what the engine and the test runner are given; its ctx is this record, which must not move |
| `r` | `*Report` | the command's report |
| `units` | `usize` | the plan's unit count; one table closes a plan of more than one |
| `quiet` | `bool` | `--quiet`, which suppresses the progress lines and banners |
| `cell` | `mach.report.readout.Unit` | the unit the events now arriving belong to, when `in_cell` |
| `named` | `bool` | whether a line naming `cell` has been written |
| `live` | `bool` | whether a live progress line is shown, and `shown` whether one is on the terminal now |
| `a` | `*std.allocator.Allocator` | the arena what is held lives in, nil until something is |
| `rows` | `Vector[StageRow]` | the -v table's rows, held until the unit closes |
| `modules` | `Vector[ModuleRow]` | each module's counters under -vv |
| `done` | `Vector[UnitRow]` | the summaries of a plan of several units |
| `run` | `*mach.report.readout.TestResult` | the running test run's results, from its run_start to its run_end |
| `count` | `u32` | how many results run holds |
| `roll` | `[ROLL_COLS]mach.report.layout.Column` | a module row's columns: module, time, then the ok, skip and FAIL counts |
| `line` | `[3]mach.report.layout.Column` | a test line's columns under -v: outcome, test, time |
| `counts` | `[readout.COUNTER_COUNT]u64` | the counter totals held until the unit's rows close, those `held` |
| `holding` | `bool` | whether any total is held |

### readout_begin

```mach
pub fun readout_begin(rd: *Readout, r: *Report, units: usize, quiet: bool, level: u8)
```

start rendering a command's events to r, a test run in human text to stdout

| Parameter | |
| :--- | :--- |
| `rd` | the renderer, initialised in place since its sink points at it |
| `r` | the command's report |
| `units` | the plan's unit count |
| `quiet` | `--quiet` |
| `level` | the readout level the command asked for, LEVEL_RESULTS to LEVEL_ITEMS |

### readout_end

```mach
pub fun readout_end(rd: *Readout)
```

close a plan's rendering: under human text the table a plan of several units closes with.
the tally of what it rendered closes the report, in report_dnit

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
tally the report closes with, or a record each under json

| Parameter | |
| :--- | :--- |
| `r` | the command's report |
| `sources` | the SourceMap that locates each diagnostic |
| `list` | the diagnostics, in order |

### dependency_report

```mach
pub fun dependency_report(a: *std.allocator.Allocator, r: *Report) mach.package.report.Report
```

where a dependency command reports: progress on standard output, and notes and failures,
the ones the command goes on past too, through the command's report

| Parameter | |
| :--- | :--- |
| `a` | owns the report's effect notes |
| `r` | the command's report, in the format `--report` selected |
| **returns** | the report, released with `report.dnit` |

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
pub fun report_said(r: *Report, k: mach.base.diagnostic.kind.Kind, v: mach.report.layout.Verb, texts: *str, n: usize, detail: str, text: str)
```

report what a command did as a progress line in human text, and as the note record of kind
`k` saying `text` under the records modes

| Parameter | |
| :--- | :--- |
| `r` | the command's report |
| `k` | the note's kind, a row of note level |
| `v` | the verb of the progress line |
| `texts` | what it acted on, then its detail on the line, `n` of them |
| `n` | how many texts |
| `detail` | a line beneath it, nil for none |
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

### has_capture

```mach
pub fun has_capture(r: *mach.report.testrun.GateResult) bool
```

whether a result kept output worth pointing at

### said_row

```mach
pub fun said_row(w: *std.io.writer.Writer, v: mach.report.layout.Verb, texts: *str, n: usize)
```

a progress line on w: its verb in the gutter, then `n` texts parted by GAP

