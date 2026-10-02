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
pub fun headline_lead(k: dkind.Kind);
```

write the lead of an error of kind `k` to stderr, `error[<key>]: `, for a
command that prints the rest of the message itself

## fun render_fail

```mach
pub fun render_fail(f: *outcome.Fail) i64;
```

print a Fail to stderr as "error[<key>]: <message>", followed by the
" --> <file>:<line>:<column>" it points at when it names one and one such
line for each related place it names, and map it to an exit code
a reported Fail prints nothing, its diagnostics having been rendered already

f: the Fail
ret: the exit code `exit.of` maps the failure to

## fun refuse

```mach
pub fun refuse(f: outcome.Fail) i64;
```

render_fail for a failure built in place

f: the Fail
ret: the exit code `exit.of` maps the failure to

## fun outcome_code_w

```mach
pub fun outcome_code_w(w: *io_writer.Writer, bo: *outcome.BuildOutcome) i64;
```

map a build outcome's severity to the process exit code

bo: the outcome
ret: the shared exit code of the severity; a severity outside the catalog
is an internal failure written to `w` naming the catalog and the tag

## fun outcome_code

```mach
pub fun outcome_code(bo: *outcome.BuildOutcome) i64;
```

## fun report_outcome

```mach
pub fun report_outcome(r: *Report, bo: *outcome.BuildOutcome, quiet: bool);
```

render a build outcome's events to stderr in order. human text renders unit
banners, failures, diagnostics and the closing tally; json renders each
diagnostic and each failure as one record

r: the command's report
bo: the outcome
quiet: suppress the "building <artifact> (<target>)" banner, which prints only when the
       outcome has more than one unit

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
pub fun report_fail(r: *Report, f: outcome.Fail, origin: diagnostic.Origin) i64;
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

## val SCHEMA_VERSION

```mach
pub val SCHEMA_VERSION: i64 = 1
```

the version every json record carries in `schema`. adding a field, a record
type or a value of an enumerated field keeps it; changing or removing one
bumps it (doc/language/diagnostics-json.md)

