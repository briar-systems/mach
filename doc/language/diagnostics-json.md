# Machine-readable diagnostics

`mach build`, `mach check` and `mach test` take `--diagnostics=<human|json>`.
`human`, the default, is the rendering shown in [diagnostics.md](diagnostics.md).
`json` writes everything the command reports as JSON objects, one per line of
stderr (NDJSON), for editors, CI and other tools: each diagnostic, each failure
raised outside the compiler's diagnostics (a manifest, dependency, build-step,
link or command-line refusal), each test result under `mach test`, and a
closing summary. The records are the contract: the human text may change from
release to release, and a record changes only under the
[stability rule](#stability). The flag changes no exit code.

```
mach check . --diagnostics=json
```

A bare `--diagnostics` or any other value is refused with
`error[cli.flag_value]`, in human text since no format has been chosen. `-v`
and `-vv`, whose phase readout is text on stderr, are refused beside `json`
with a `cli.flag_conflict` failure record.

## The stream

- Records go to stderr, one per line, in the order the human rendering shows
  the same reports. Stdout keeps what the command writes there, such as the
  events of `mach test --format json` or a build step's banner.
- Every line mach writes to stderr is a record, a JSON object on a line of its
  own, and the last one is the [summary](#the-summary-record). Output a build
  step or a test writes itself is not mach's and is not a record.
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
| `record` | string | the record type, `"diagnostic"`, or `"failure"` for a [failure](#failure-records) |
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
| `test` | the test runner of `mach test`: running the collected tests |

A key names a rule, not a phase, so one key can arrive from two origins:
`secret.branch` is `sema` when the type checker finds it and `codegen` when
the constant-time validator does.

## Failure records

A failure raised outside the compiler's diagnostics, the ones the human
rendering prints as an `error[<key>]: <message>` line with no source excerpt,
is a record with `"record": "failure"` and the members of a diagnostic record.
Its `severity` is `"error"`, its `code` the failure's key, and `notes`, `help`
and `fixes` are empty.

```json
{"schema":1,"record":"failure","severity":"error","code":"link.entry_missing","message":"undefined entry symbol '_start'; ensure the target startup library is linked","origin":"link","primary":null,"related":[],"notes":[],"help":[],"fixes":[]}
```

`primary` is the span in the file that caused the failure, or `null` when no
file did. A manifest refusal points into the `mach.toml` that made the claim it
refuses: a value it rejects, the key token of a name or key it rejects, or the
table a required key is missing from. A dependency's refusal points into the
dependency's own manifest, not the root that imported it. Its `file` follows
the rule for every span, so a dependency's manifest is `dep/libx/mach.toml`,
and `dep\libx\mach.toml` on Windows. The human rendering shows the same place on
the line after the headline, as `--> <file>:<line>:<column>`, naming the file by
the path it was read at, as it names a source file.

```json
{"schema":1,"record":"failure","severity":"error","code":"version.invalid_range","message":"mach.toml: [project].mach = \"^x\": expected a number (clause 1)","origin":"build","primary":{"file":"mach.toml","line":2,"column":8,"end_line":2,"end_column":12,"byte_start":17,"byte_end":21},"related":[],"notes":[],"help":[],"fixes":[]}
```

A refusal raised later from the parsed manifest, while a build is laid out or
its steps run, points at the entry that caused it the same way:

- a path template that does not expand, at its value: `[project].out`, an
  artifact's `out`, a local link's `path`, and a step's `argv`, `env`, `in` and
  `out` entries
- a step output under a directory the compiler owns, at the output
- a local link found neither among the step outputs nor on disk, at its `path`
- an artifact that does not build for the selected target, at its `targets`
- no artifact building for the selected target, at every artifact's `targets`

A refusal between several entries points at the first and lists the others in
`related`, each with a `null` label:

- two steps declaring one output, or two artifacts writing one path, at the
  first claim
- more than one `default = true` target, profile or artifact, at the first
  `default` value
- a selection several declarations could satisfy (`native` matching several
  targets, several targets, profiles or artifacts with none marked default), at
  the first candidate's table key
- a cycle among `need` entries, at the entry of its first edge, the entries of
  the other edges related

The human rendering shows each related place on its own `-->` line after the
primary's. A target or profile named on the command line that the manifest does
not declare is caused by no entry in it, so its refusal has no `primary`.

```json
{"schema":1,"record":"failure","severity":"error","code":"need.cycle","message":"mach.toml: build step 'a' is part of a 'need' cycle","origin":"build","primary":{"file":"mach.toml","line":12,"column":9,"end_line":12,"end_column":17,"byte_start":141,"byte_end":149},"related":[{"file":"mach.toml","line":18,"column":9,"end_line":18,"end_column":17,"byte_start":216,"byte_end":224,"label":null}],"notes":[],"help":[],"fixes":[]}
```

`origin` names the phase the failure came from: `build` for the manifest,
dependency resolution and build steps, the phase that failed for a failure
inside the compiler (`link` for the linker), and `test` for the test runner. A
refusal of the command line itself, such as an unknown flag or a project path
that names nothing, has no `origin`.

## Test records

Under `mach test` each test's result is a record, in the order the tests
finish:

```json
{"schema":1,"record":"test","name":"app.main#parses","module":"app.main","file":"src/main.mach","line":10,"outcome":"exit","code":3,"origin":"test"}
```

| Member | Type | Meaning |
|---|---|---|
| `name` | string | the test's qualified name |
| `module` | string | the module that declares it |
| `file` | string | the source file, spelled as a span's `file` is |
| `line` | integer | the line of its declaration, from 1 |
| `outcome` | string | how it ended, the `kind` `mach test --format json` reports: `"pass"`, `"exit"`, `"signal"`, `"timeout"`, `"spawn"` or `"other"` |
| `code` | integer | its exit code, or `0` when it has none |
| `origin` | string | `"test"` |

A test record carries no `severity` and counts in no summary total; the
summary's `outcome` and `exit_code` say whether a test failed.

## The summary record

Every run ends with one summary record, the last line on stderr:

```json
{"schema":1,"record":"summary","errors":1,"warnings":0,"notes":0,"outcome":"failure","exit_code":1}
```

| Member | Type | Meaning |
|---|---|---|
| `errors` | integer | the diagnostic and failure records before it with severity `"error"` |
| `warnings` | integer | those with severity `"warning"` |
| `notes` | integer | those with severity `"note"` |
| `outcome` | string | `"success"` when the command exits 0, otherwise `"failure"` |
| `exit_code` | integer | the command's exit code |

## Stability

- Every record carries `"schema": 1`.
- **Compatible changes keep the version:** a new member in a record or a span,
  a new record type, and a new value of an enumerated member (`severity`,
  `origin`, `record`). A tool ignores members it does not know, skips records
  whose `record` it does not know, and accepts an `origin` it does not know.
- **Any other change bumps the version:** removing or renaming a member,
  changing its type or meaning, or changing the span convention.
- A diagnostic or failure `code` keeps its meaning for good, under the rules of the
  [registry](diagnostics.md#the-registry).

## See also

- [diagnostics.md](diagnostics.md) — keys, the registry and its never-reused rule
- [test.md](test.md#json-output) — `mach test --format json`, the test runner's event stream on stdout
