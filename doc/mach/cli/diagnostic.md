# mach.cli.diagnostic

## fun flush

```mach
pub fun flush(out: *io_writer.Writer, s: *session.Session, list: *diagnostic.DiagnosticStore);
```

render every diagnostic of a store to a writer, then an "N errors / M warnings" line

out: the destination
s: the session whose SourceMap locates each diagnostic
list: the diagnostics, rendered in order; nothing is written when it is empty

## fun flush_sources

```mach
pub fun flush_sources(out: *io_writer.Writer, sources: *lang_source.SourceMap, list: *diagnostic.DiagnosticStore);
```

render every diagnostic of a store to a writer, then an "N errors / M warnings" line

out: the destination
sources: the SourceMap that locates each diagnostic
list: the diagnostics, rendered in order; nothing is written when it is empty

## fun render_fail

```mach
pub fun render_fail(f: *fail.Fail) i64;
```

print a Fail to stderr as "error[<key>]: <message>", followed by the
" --> <file>:<line>:<column>" it points at when it names one and one such
line for each related place it names, and map it to an exit code
a reported Fail prints nothing, its diagnostics having been rendered already

f: the Fail
ret: the exit code `exit.of` maps the failure to

## fun refuse

```mach
pub fun refuse(f: fail.Fail) i64;
```

render_fail for a failure built in place

f: the Fail
ret: the exit code `exit.of` maps the failure to

## fun dependency_report

```mach
pub fun dependency_report(a: *A.Allocator) package_report.Report;
```

where a dependency command reports: progress on standard output, and notes and failures
the command goes on past on standard error

a: owns the report's effect notes
ret: the report, released with `package_report.dnit`

## fun dependency_outcome

```mach
pub fun dependency_outcome(rep: *package_report.Report, outcome: err[fail.Fail]) i64;
```

show how a dependency command ended: its failure, then a note for each change it made
before failing

rep: the report the command used
outcome: how it ended
ret: exit.OK, or the code `exit.of` maps the failure to

## fun outcome_code

```mach
pub fun outcome_code(bo: *outcome.BuildOutcome) i64;
```

## fun size_split

```mach
pub fun size_split(bytes: usize, unit: *str) i64;
```

split a byte count into the binary magnitude it reads best in and its unit

## rec Readout

```mach
pub rec Readout;
```

every command's events rendered the moment each arrives: the renderer of the
readout contract (doc/language/readout.md). the report's stream, stderr,
carries the build: human text renders unit banners, phase rows, items, the
unit summary, failures and diagnostics, and readout_close writes the closing
tally; json renders each diagnostic and failure as a record, and under -v
each unit, phase row, item and unit summary as unit, phase and phase_item
records. stdout carries a test run in human text: each test as it finishes, a
module's roll-up once its last test has, and the closing summary. under json the
run is also records on the report's stream: run_start, a test record per
finished test, and run_end

sink: what the engine and the test runner are given; its ctx is this record, which must not move
r: the command's report
units: the plan's unit count; a banner prints only when it is more than one
quiet: `--quiet`, which suppresses the banners
tally: the human tally, counted as the events pass
out: where a test run renders, stdout
run: the running test run's results, from its run_start to its run_end
count: how many results run holds
runner: the command each test is launched through, or nil
mod_w: the module column's width
name_w: the test column's width

## fun readout_init

```mach
pub fun readout_init(rd: *Readout, r: *Report, units: usize, quiet: bool, level: u8);
```

start rendering a command's events to r, a test run in human text to stdout

rd: the renderer, initialised in place since its sink points at it
r: the command's report
units: the plan's unit count
quiet: `--quiet`
level: the readout level the command asked for, LEVEL_RESULTS to LEVEL_ITEMS

## fun readout_close

```mach
pub fun readout_close(rd: *Readout);
```

close a plan's rendering: one summary line under human text closes every
failure and diagnostic the plan rendered, a failure record printed as its own
`error:` line (an encoder or rules refusal that reached no store) counted in it
beside every store's diagnostics, so the tally never reads `0 errors` above a
nonzero exit. json closes in report_close

## fun result_name

```mach
pub fun result_name(r: *validation.ValidationGateResult) *u8;
```

the outcome a test record names: `pass`, `exit`, `signal`, `spawn`,
`timeout`, or `other`

## fun has_capture

```mach
pub fun has_capture(r: *validation.ValidationGateResult) bool;
```

whether a result kept output worth pointing at

## def Format

```mach
pub def Format: u8
```

how a command writes its diagnostics to stderr: `--diagnostics=human`, the
default, or `--diagnostics=json`

## val FORMAT_HUMAN

```mach
pub val FORMAT_HUMAN: Format = 0
```

## val FORMAT_JSON

```mach
pub val FORMAT_JSON:  Format = 1
```

## fun format_named

```mach
pub fun format_named(name: str) opt[Format];
```

the format a `--diagnostics` value names

name: the value
ret: the format, or none for a name outside `human` and `json`

## rec Report

```mach
pub rec Report;
```

how a build-shaped command reports to stderr, and what it has reported. under
json every line it writes is a record and `report_close` ends the run with the
summary record, whose counts are the severities of the records before it

format: human text or json records
w: where the report is written, stderr
root: the project root as the command resolved it, nil until then; a json
          record names a file inside it relative to it
errors: the error records written
warnings: the warning records written
made: whether the arena record paths are built in, and the base they are
          measured from, have been made; they are on first use
built: under json, each unit's `built` line, which the summary record carries

## fun report_init

```mach
pub fun report_init(r: *Report, format: Format);
```

start a report in place; it holds its own arena, so it stays where it was made
until `report_close`

r: the report
format: the format the command writes

## fun report_fail

```mach
pub fun report_fail(r: *Report, f: fail.Fail, origin: diagnostic.Origin) i64;
```

report a failure raised outside a diagnostic store: `error[<key>]: <message>`
as text, or a failure record under json. a reported failure writes nothing,
its diagnostics having been rendered already

r: the command's report
f: the failure
origin: the phase it is reported under; ORIGIN_NONE for the command line itself
ret: the exit code `exit.of` maps the failure to

## fun report_test

```mach
pub fun report_test(r: *Report, t: *readout.TestResult);
```

one test's result as a record under json; human text writes nothing here,
the runner's own readout carrying it

r: the command's report
t: the finished test

## fun report_case

```mach
pub fun report_case(r: *Report, art: *outcome.TestArtifact);
```

one collected test as a record under json, as `mach test --list` lists it; human text
writes nothing here

r: the command's report
art: the test

## fun report_skip

```mach
pub fun report_skip(r: *Report, target: str, profile: str, reason: str);
```

a (target, profile) whose tests were built and not run, as a record under json; human text
writes nothing here

r: the command's report
target: the declared target name
profile: the profile name
reason: why its tests did not run

## fun report_close

```mach
pub fun report_close(r: *Report, code: i64) i64;
```

end the run: under json the summary record, the last line the command writes;
the report's arena is released either way

r: the command's report
code: the exit code the command returns
ret: code

