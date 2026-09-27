# mach.cli.diagnostic

## fun flush

```mach
pub fun flush(out: *writer.Writer, s: *session.Session, list: *diagnostic.DiagnosticStore);
```

render every diagnostic of a store to a writer, then an "N errors / M warnings" line

out: the destination
s: the session whose SourceMap locates each diagnostic
list: the diagnostics, rendered in order; nothing is written when it is empty

## fun flush_sources

```mach
pub fun flush_sources(out: *writer.Writer, sources: *source.SourceMap, list: *diagnostic.DiagnosticStore);
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

print a Fail to stderr as "error[<key>]: <message>" and map it to an exit code
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
pub fun outcome_code_w(w: *writer.Writer, bo: *outcome.BuildOutcome) i64;
```

map a build outcome's severity to the process exit code

bo: the outcome
ret: the shared exit code of the severity; a severity outside the catalog
is an internal failure written to `w` naming the catalog and the tag

## fun outcome_code

```mach
pub fun outcome_code(bo: *outcome.BuildOutcome) i64;
```

## fun render_outcome

```mach
pub fun render_outcome(bo: *outcome.BuildOutcome, quiet: bool, r: *Rendering);
```

render a build outcome's events to stderr in order. human text renders unit
banners, failures, diagnostics and the closing tally; json renders each
diagnostic as one record, and a failure that is not a diagnostic as text

bo: the outcome
quiet: suppress the "building <artifact> (<target>)" banner, which prints only when the
       outcome has more than one unit
r: the format, and the project root json paths are relative to

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

## rec Rendering

```mach
pub rec Rendering;
```

what rendering an outcome needs beyond the outcome

format: human text or json records
root: the project root as the command resolved it; a json record names a
        file inside it relative to it

## val SCHEMA_VERSION

```mach
pub val SCHEMA_VERSION: i64 = 1
```

the version every json record carries in `schema`. adding a field, a record
type or a value of an enumerated field keeps it; changing or removing one
bumps it (doc/language/diagnostics-json.md)

