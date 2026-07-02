# `mach check` JSON diagnostic schema (v1)

`mach check --format json` emits its diagnostics as a machine-readable stream
instead of the framed source snippets: one JSON object per line
(newline-delimited JSON / NDJSON), no interleaved human text. Editors, LSP
shims, and CI annotators parse this stream rather than scraping the human
frames, which stay free to evolve.

The stream is written to **stdout**. Usage errors and internal failures (an
unreadable file, an analysis that could not run) stay on **stderr**, so a
consumer reads a clean event stream on stdout regardless of that noise. Exit
codes are unchanged from the human mode: `0` when the buffer has no
error-severity diagnostics, `1` when it has any (or on a usage / io error).

Every line is a complete JSON object carrying a `schema` version integer and an
`event` discriminator. Strings are escaped to printable ASCII: control
characters use their JSON escapes and every byte `>= 0x80` is emitted as a
`\uXXXX` escape (UTF-8 decoded, astral code points as a surrogate pair), so the
output is byte-for-byte identical on every platform. Numbers are integers. The
escaper is the shared `mach.cli.json` emitter, the same one `mach test --format
json` uses (see [test-json.md](test-json.md) for the escaping rationale).

### Paths and positions

`file` is emitted exactly as the compiler references it — the path passed to
`mach check`, relative to the working directory or absolute as given. Line and
column numbers are **1-based**, matching the human renderer and the compiler's
own convention; a consumer feeding an LSP (whose `Position` is 0-based) subtracts
one from each. A span is half-open: `start` is the position of its first byte
and `end` is the position just past its last, so a zero-width span has
`start == end`.

## Versioning

`schema` is `1`. It is bumped only on an **incompatible** change to the object
shapes — a removed or renamed field, or a changed field type or meaning. Adding
a new optional field (for example a diagnostic `code`) or a new `event` kind is
backward compatible and does **not** bump the version; a consumer must ignore
unknown fields and unknown `event` values. Pin behavior to the `schema` integer,
never to field ordering.

## Events

### `diagnostic`

Emitted once per reported diagnostic, in report order.

| Field      | Type          | Description |
|------------|---------------|-------------|
| `schema`   | int           | schema version (`1`) |
| `event`    | string        | `"diagnostic"` |
| `severity` | string        | `"error"`, `"warning"`, `"info"`, or `"help"` |
| `message`  | string        | the diagnostic message, verbatim |
| `location` | object / null | the primary source location (see below), or `null` when its file does not resolve |
| `note`     | string / null | the attached `= note:` line, or `null` |
| `help`     | string / null | the attached `= help:` suggestion, or `null` |
| `related`  | array         | secondary locations (possibly empty); see below |

A **location** object is:

| Field   | Type   | Description |
|---------|--------|-------------|
| `file`  | string | source file path as the compiler references it |
| `start` | object | inclusive start position `{ "line": int, "col": int }` |
| `end`   | object | exclusive end position `{ "line": int, "col": int }` |

Each **related** element is:

| Field      | Type          | Description |
|------------|---------------|-------------|
| `location` | object / null | the secondary location, or `null` when its file does not resolve |
| `label`    | string / null | the caption rendered under the secondary span, or `null` for a bare context span |

```json
{"schema":1,"event":"diagnostic","severity":"error","message":"expected an expression","location":{"file":"buf.mach","start":{"line":2,"col":12},"end":{"line":2,"col":13}},"note":null,"help":null,"related":[]}
{"schema":1,"event":"diagnostic","severity":"error","message":"duplicate definition of 'dup'","location":{"file":"buf.mach","start":{"line":2,"col":5},"end":{"line":2,"col":8}},"note":null,"help":"rename one of the definitions","related":[{"location":{"file":"buf.mach","start":{"line":1,"col":5},"end":{"line":1,"col":8}},"label":"previous definition here"}]}
```

### `summary`

Emitted once, after every `diagnostic`, closing the stream — even when the
buffer produced no diagnostics.

| Field      | Type   | Description |
|------------|--------|-------------|
| `schema`   | int    | schema version (`1`) |
| `event`    | string | `"summary"` |
| `errors`   | int    | number of error-severity diagnostics |
| `warnings` | int    | number of warning-severity diagnostics |

```json
{"schema":1,"event":"summary","errors":1,"warnings":0}
```

## Stream shape

A run is zero or more `diagnostic` objects followed by exactly one `summary`. A
clean buffer emits only the `summary` (with zero counts), so a consumer always
sees a terminating object. A fatal error before analysis (an unreadable file, a
failed parse setup) prints to stderr and exits non-zero with no stdout stream.

`mach build`'s diagnostics are **out of scope** for this schema: a build
interleaves diagnostics with progress and readout output and warrants its own
event model, so `--format json` shapes only `mach check`.

## See also

- [cli.md](../cli.md) — the `mach check` command and its flags.
- [test-json.md](test-json.md) — the sibling `mach test` event schema.
