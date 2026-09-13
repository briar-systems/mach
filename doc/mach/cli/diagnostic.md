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

## fun render_fail

```mach
pub fun render_fail(f: *outcome.Fail) i64;
```

print a Fail to stderr as "error: <message>" and map it to an exit code
a reported Fail prints nothing, its diagnostics having been rendered already

f: the Fail
ret: 1 for reported and user failures, 2 for internal, 3 for environment

## fun outcome_code_w

```mach
pub fun outcome_code_w(w: *writer.Writer, bo: *outcome.BuildOutcome) i64;
```

map a build outcome's severity to the process exit code

bo: the outcome
ret: 0 ok, 1 user, 2 internal, 3 environment; a severity outside the catalog
is an internal failure written to `w` naming the catalog and the tag

## fun outcome_code

```mach
pub fun outcome_code(bo: *outcome.BuildOutcome) i64;
```

## fun render_outcome

```mach
pub fun render_outcome(bo: *outcome.BuildOutcome, quiet: bool);
```

render a build outcome's events to stderr in order: unit banners, scalarization notes,
failures, and diagnostics

bo: the outcome
quiet: suppress the "building <artifact> (<target>)" banner, which prints only when the
       outcome has more than one unit

