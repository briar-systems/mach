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

## fun headline_lead

```mach
pub fun headline_lead(k: diagnostic_kind.Kind);
```

write the lead of an error of kind `k` to stderr, `error[<key>]: `, for a
command that prints the rest of the message itself

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

a build's events rendered to the report's stream the moment each arrives.
human text renders unit banners, phase rows, items, the unit summary,
failures and diagnostics, and readout_close writes the closing tally; json
renders each diagnostic and each failure as one record

sink: what the engine is given; its ctx is this record, which must not move
units: the plan's unit count; a banner prints only when it is more than one
quiet: `--quiet`, which suppresses the banners
tally: the human tally, counted as the events pass

## fun readout_init

```mach
pub fun readout_init(rd: *Readout, r: *Report, units: usize, quiet: bool, level: u8);
```

start rendering a plan's events to r

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
notes: the note records written
made: whether the arena record paths are built in, and the base they are
          measured from, have been made; they are on first use

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
pub fun report_test(r: *Report, art: *outcome.TestArtifact, result: str, code: i64);
```

one test's result as a record under json; human text writes nothing here,
the runner's own readout carrying it

r: the command's report
art: the test
result: its outcome, a kind `mach test --format json` reports
code: its exit code

## fun report_close

```mach
pub fun report_close(r: *Report, code: i64) i64;
```

end the run: under json the summary record, the last line the command writes;
the report's arena is released either way

r: the command's report
code: the exit code the command returns
ret: code

