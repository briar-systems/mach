# Machine-readable diagnostics

`mach build`, `mach check` and `mach test` take `--diagnostics=<human|json>`.
`human`, the default, is the rendering shown in [diagnostics.md](diagnostics.md).
`json` writes each diagnostic as one JSON object on one line of stderr
(NDJSON), for editors, CI and other tools. The records are the contract: the
human text may change from release to release, and a record changes only under
the [stability rule](#stability). The flag changes no exit code.

```
mach check . --diagnostics=json
```

A bare `--diagnostics` or any other value is refused with
`error[cli.flag_value]`.

## The stream

- Records go to stderr, one per line, in the order the human rendering shows
  the same diagnostics. Stdout keeps what the command writes there, such as the
  events of `mach test --format json`.
- Every record is a JSON object on a line of its own, and every record line
  starts with `{`. A line that does not start with `{` is not a record: a
  failure mach does not yet report as a record (#3794) still prints as human
  text, and a tool skips such a line or shows it as it is.
- The text is ASCII. A non-ASCII character in a message, a label or a path is
  written as a `\u` escape, and a byte that is not valid UTF-8 as `�`, so
  every line is valid UTF-8 and valid JSON.
- Under `json` the human tally (`N errors / M warnings`) and the
  `building <artifact>` banners are not written.

## A record

```json
{"schema":1,"record":"diagnostic","severity":"error","code":"name.unresolved","message":"unresolved identifier `helpr`","origin":"resolve","primary":{"file":"src/main.mach","line":5,"column":18,"end_line":5,"end_column":23,"byte_start":107,"byte_end":112},"related":[],"notes":[],"help":["did you mean `helper`?"],"fixes":[{"label":"replace with `helper`","edits":[{"file":"src/main.mach","line":5,"column":18,"end_line":5,"end_column":23,"byte_start":107,"byte_end":112,"replacement":"helper"}]}]}
```

A type error in a generic instance, whose primary span covers two lines and
whose related site is in another file:

```json
{"schema":1,"record":"diagnostic","severity":"error","code":"operator.operand_mismatch","message":"type mismatch: incompatible operand types `R` and `i64`","origin":"sema","primary":{"file":"src/main.mach","line":6,"column":16,"end_line":7,"end_column":11,"byte_start":111,"byte_end":127},"related":[{"file":"src/lib.mach","line":3,"column":9,"end_line":3,"end_column":14,"byte_start":53,"byte_end":58,"label":"in this generic body, checked against this instance's type arguments"}],"notes":[],"help":[],"fixes":[]}
```

| Member | Type | Meaning |
|---|---|---|
| `schema` | integer | the schema version, `1` |
| `record` | string | the record type, `"diagnostic"` |
| `severity` | string | `"error"`, `"warning"` or `"note"` |
| `code` | string | the diagnostic's key from the [registry](diagnostics.md#the-registry), such as `name.unresolved`; the stable identity of the kind |
| `message` | string | the primary text; its wording may change, the `code` does not |
| `origin` | string | the phase that produced the diagnostic, see [Origins](#origins); absent when mach cannot attribute it |
| `primary` | span or `null` | where the diagnostic points; `null` for a diagnostic with no location, such as `target.skipped` |
| `related` | array of span | other sites the diagnostic names, each span with a `label`: a string, or `null` for a site shown without one |
| `notes` | array of string | the notes, in order, including a related site's label when mach cannot locate that site |
| `help` | array of string | the help lines, in order |
| `fixes` | array of fix | the suggested fixes, in order |

A **fix** is an object with a `label` (string, what the fix does) and `edits`,
an array of spans each with a `replacement` string: applying a fix replaces
the bytes of every one of its edits with that edit's replacement. The edits of
one fix never overlap.

### Spans

A span is an object:

| Member | Type | Meaning |
|---|---|---|
| `file` | string | the source file: relative to the project root for a file inside it, absolute for a file outside it, in the host's path spelling |
| `line` | integer | the line of the span's first byte, from 1 |
| `column` | integer | the column of the span's first byte, from 1, counted in UTF-8 bytes |
| `end_line` | integer | the line of the position just past the span's last byte |
| `end_column` | integer | the column of that position, from 1, in UTF-8 bytes |
| `byte_start` | integer | the byte offset of the span's first byte in the file, from 0 |
| `byte_end` | integer | the byte offset just past its last byte |

The end is exclusive, as in the Language Server Protocol: a span of `helpr` at
line 5 column 18 ends at line 5 column 23. An empty span has its end equal to
its start. A column counts bytes, not characters, so a tool that needs UTF-16
columns converts from the line's text. `line` and `column` are the position
the human rendering shows after `-->`.

`primary` may carry a `label`, a string, when a diagnostic labels its primary
span.

### Origins

| Origin | Produced by |
|---|---|
| `build` | build configuration: the manifest's target, profile and artifact selection, and build steps |
| `load` | loading and parsing the module closure, and `$if` gates decided while loading |
| `resolve` | name resolution, `use`, exports and visibility |
| `sema` | type checking, including a declaration's unfulfilled `#[expect]` |
| `lower` | lowering to IR and the optimizer, such as `vector.scalarize` |
| `codegen` | code generation and the constant-time validation of emitted code |
| `link` | linking an executable or library |

A key names a rule, not a phase, so one key can arrive from two origins:
`secret.branch` is `sema` when the type checker finds it and `codegen` when
the constant-time validator does.

## Stability

- Every record carries `"schema": 1`.
- **Compatible changes keep the version:** a new member in a record or a span,
  a new record type, and a new value of an enumerated member (`severity`,
  `origin`, `record`). A tool ignores members it does not know, skips records
  whose `record` it does not know, and accepts an `origin` it does not know.
- **Any other change bumps the version:** removing or renaming a member,
  changing its type or meaning, or changing the span convention.
- A `code` keeps its meaning for good, under the rules of the
  [registry](diagnostics.md#the-registry).

## Reserved record types

These record types are reserved in schema 1 for reports mach does not yet
write as records (#3794). Each carries `schema` and `record` as above.

- `"record": "failure"`: a build, link, manifest, dependency or command-line
  failure reported outside the diagnostic store. It has the members of a
  diagnostic record, with `primary` `null` when the failure has no location.
- `"record": "test"`: one test's result under `mach test`, naming the test by
  `name`, `module`, `file` and `line`, with its `outcome` (the kinds
  `mach test --format json` reports) and its exit `code`.
- `"record": "summary"`: the last record of a run: `errors`, `warnings` and
  `notes` counts over the records before it, the `outcome` (`"success"` or
  `"failure"`) and the `exit_code`.

## See also

- [diagnostics.md](diagnostics.md) — keys, the registry and its never-reused rule
- [test.md](test.md#json-output) — `mach test --format json`, the test runner's event stream on stdout
