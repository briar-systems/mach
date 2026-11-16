[mach](../README.md) / [cli](README.md) / **diagnostic**

# mach.cli.diagnostic

how a command reports: diagnostics, failures, the build readout and test results

a command's `Report` writes to stderr as human text or, under `--diagnostics json`,
one record a line closed by a summary record. a `Readout` renders the engine's and the
test runner's events as they arrive, by the readout contract (doc/language/readout.md).

Source: [`src/cli/diagnostic.mach`](../../../src/cli/diagnostic.mach)

Uses types from [`mach.base.diagnostic`](../base/diagnostic.md), [`mach.base.diagnostic.kind`](../base/diagnostic/kind.md), [`mach.base.fail`](../base/fail.md), [`mach.base.source`](../base/source.md), [`mach.build.outcome`](../build/outcome.md), [`mach.package.report`](../package/report.md), [`mach.report.readout`](../report/readout.md), [`mach.report.testrun`](../report/testrun.md), [`mach.session`](../session.md).

| Type | | |
| :--- | :--- | :--- |
| [`Readout`](#readout) | `rec` | every command's events rendered the moment each arrives: the renderer of the readout contract (doc/language/readout.md) |
| [`Format`](#format) | `def` | how a command writes its diagnostics to stderr: `--diagnostics=human`, the default, or `--diagnostics=json` |
| [`Report`](#report) | `rec` | how a build-shaped command reports to stderr, and what it has reported |

| On `Readout` | |
| :--- | :--- |
| [`readout_begin`](#readout_begin) | start rendering a command's events to r, a test run in human text to stdout |
| [`readout_end`](#readout_end) | close a plan's rendering: one summary line under human text closes every failure and diagnostic the plan rendered, a carried failure (an encoder or rules refusal that reached no store) counted in it beside every store's diagnostics, so the tally never reads `0 errors` above a nonzero exit |

| On `Format` | |
| :--- | :--- |
| [`FORMAT_HUMAN`](#format) | |
| [`format_named`](#format_named) | the format a `--diagnostics` value names |

| On `Report` | |
| :--- | :--- |
| [`report_store`](#report_store) | render every diagnostic of a store through a command's report: as text with its tally line, or a record each under json |
| [`dependency_report`](#dependency_report) | where a dependency command reports: progress on standard output, and notes and failures, the ones the command goes on past too, through the command's report |
| [`report_init`](#report_init) | start a report in place; it holds its own arena, so it stays where it was made until `report_dnit` |
| [`out`](#out) | where a command writes its output, stdout through the report, so a refused write fails the command as the system's |
| [`report_note`](#report_note) | report a note of kind `k`, which points at nothing: `note: text` in human text, or a diagnostic record of note severity under json |
| [`report_fail`](#report_fail) | report a carried failure as the diagnostic it reads as, in text or as a diagnostic record under json |
| [`report_case`](#report_case) | one collected test as a record under json, as `mach test --list` lists it; human text writes nothing here |
| [`report_skip`](#report_skip) | a (target, profile) whose tests were built and not run, as a record under json; human text writes nothing here |
| [`report_dnit`](#report_dnit) | end the run: a refusal kept while it ran, such as a refused write to stdout, reported, then under json the summary record, the last line the command writes; the report's arena is released either way |

| Functions | |
| :--- | :--- |
| [`flush`](#flush) | render every diagnostic of a store to a writer, then an "N errors / M warnings" line |
| [`flush_sources`](#flush_sources) | render every diagnostic of a store to a writer, then an "N errors / M warnings" line |
| [`render_fail`](#render_fail) | print a Fail to stderr as the diagnostic it reads as, and map it to an exit code a reported Fail prints nothing, its diagnostics having been rendered already |
| [`refuse`](#refuse) | render_fail for a failure built in place |
| [`dependency_outcome`](#dependency_outcome) | show how a dependency command ended: its failure, then a note for each change it made before failing |
| [`outcome_code`](#outcome_code) | |
| [`has_capture`](#has_capture) | whether a result kept output worth pointing at |

---

## Readout

```mach
pub rec Readout {
    sink:   mach.report.readout.Sink;
    r:      *Report;
    units:  usize;
    quiet:  bool;
    tally:  Tally;
    run:    *mach.report.readout.TestResult;
    count:  u32;
    mod_w:  usize;
    name_w: usize;
}
```

every command's events rendered the moment each arrives: the renderer of the
readout contract (doc/language/readout.md). the report's stream, stderr,
carries the build: human text renders unit banners, phase rows, items, the
unit summary, failures and diagnostics, and readout_end writes the closing
tally; json renders each diagnostic and failure as a record, and under -v
each unit, phase row, item and unit summary as unit, phase and phase_item
records. stdout carries a test run in human text: each test as it finishes, a
module's roll-up once its last test has, and the closing summary. under json the
run is also records on the report's stream: run_start, a test record per
finished test, and run_end

| Field | Type | |
| :--- | :--- | :--- |
| `sink` | `mach.report.readout.Sink` | what the engine and the test runner are given; its ctx is this record, which must not move |
| `r` | `*Report` | the command's report |
| `units` | `usize` | the plan's unit count; a banner prints only when it is more than one |
| `quiet` | `bool` | `--quiet`, which suppresses the banners |
| `tally` | `Tally` | the human tally, counted as the events pass |
| `run` | `*mach.report.readout.TestResult` | the running test run's results, from its run_start to its run_end |
| `count` | `u32` | how many results run holds |
| `mod_w` | `usize` | the module column's width |
| `name_w` | `usize` | the test column's width |

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

close a plan's rendering: one summary line under human text closes every
failure and diagnostic the plan rendered, a carried failure (an encoder or
rules refusal that reached no store) counted in it beside every store's
diagnostics, so the tally never reads `0 errors` above a nonzero exit. json
closes in report_dnit

## Format

```mach
pub def Format: u8
```

how a command writes its diagnostics to stderr: `--diagnostics=human`, the
default, or `--diagnostics=json`

| Name | Value | |
| :--- | :--- | :--- |
| `FORMAT_HUMAN` | `0` |  |

### format_named

```mach
pub fun format_named(name: str) opt[Format]
```

the format a `--diagnostics` value names

| Parameter | |
| :--- | :--- |
| `name` | the value |
| **returns** | the format, or none for a name outside `human` and `json` |

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

render every diagnostic of a store through a command's report: as text with its tally line,
or a record each under json

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
| `r` | the command's report, in the format `--diagnostics` selected |
| **returns** | the report, released with `report.dnit` |

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

report a note of kind `k`, which points at nothing: `note: text` in human text, or a
diagnostic record of note severity under json

| Parameter | |
| :--- | :--- |
| `r` | the command's report |
| `k` | the note's kind, a row of note level |
| `text` | what the note says |

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
writes; the report's arena is released either way

| Parameter | |
| :--- | :--- |
| `r` | the command's report |
| `code` | the exit code the command returns |
| **returns** | the worse of code and the kept refusal's |

## Functions

### flush

```mach
pub fun flush(out: *std.io.writer.Writer, s: *mach.session.Session, list: *mach.base.diagnostic.DiagnosticStore)
```

render every diagnostic of a store to a writer, then an "N errors / M warnings" line

| Parameter | |
| :--- | :--- |
| `out` | the destination |
| `s` | the session whose SourceMap locates each diagnostic |
| `list` | the diagnostics, rendered in order; nothing is written when it is empty |

### flush_sources

```mach
pub fun flush_sources(out: *std.io.writer.Writer, sources: *mach.base.source.SourceMap, list: *mach.base.diagnostic.DiagnosticStore)
```

render every diagnostic of a store to a writer, then an "N errors / M warnings" line

| Parameter | |
| :--- | :--- |
| `out` | the destination |
| `sources` | the SourceMap that locates each diagnostic |
| `list` | the diagnostics, rendered in order; nothing is written when it is empty |

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

### has_capture

```mach
pub fun has_capture(r: *mach.report.testrun.GateResult) bool
```

whether a result kept output worth pointing at

