# `mach test` JSON event schema (v1)

`mach test --format json` emits a machine-readable stream instead of the human
readout: one JSON object per line (newline-delimited JSON / NDJSON), no
interleaved human text. Editors, CI annotators, and other tooling parse this
stream rather than scraping the live readout, which stays free to evolve.

The stream is written to **stdout**. Build diagnostics and internal errors stay
on **stderr**, so a consumer reads a clean event stream on stdout regardless of
build noise. Exit codes are unchanged from the human mode: `0` all passed, `1`
any failed, `2` build/internal error.

Every line is a complete JSON object carrying a `schema` version integer and an
`event` discriminator. Strings are escaped to printable ASCII: control
characters use their JSON escapes and every byte `>= 0x80` is emitted as a
`\uXXXX` escape (UTF-8 decoded, astral code points as a surrogate pair), so the
output is byte-for-byte identical on every platform. Numbers are integers.

## Versioning

`schema` is `1`. It is bumped only on an **incompatible** change to the event
shapes — a removed or renamed field, or a changed field type or meaning. Adding
a new optional field or a new `event` kind is backward compatible and does
**not** bump the version; a consumer must ignore unknown fields and unknown
`event` values. Pin behavior to the `schema` integer, never to field ordering.

## Events

### `run_start`

Emitted once, before any `test` event.

| Field    | Type   | Description |
|----------|--------|-------------|
| `schema` | int    | schema version (`1`) |
| `event`  | string | `"run_start"` |
| `tests`  | int    | number of tests selected for the run |

```json
{"schema":1,"event":"run_start","tests":438}
```

### `test`

Emitted once per finalized test, in collection order (the same deterministic
order as the human readout), regardless of completion order.

| Field         | Type          | Description |
|---------------|---------------|-------------|
| `schema`      | int           | schema version (`1`) |
| `event`       | string        | `"test"` |
| `label`       | string        | the test's label, verbatim as authored in `test "..."` |
| `module`      | string        | the declaring module's fully-qualified name |
| `file`        | string        | source file path of the declaring module |
| `line`        | int           | 1-based source line of the `test` keyword |
| `kind`        | string        | outcome (see below) |
| `code`        | int           | exit code (`kind` = `exit`) or signal number (`kind` = `signal`); `0` otherwise |
| `duration_ns` | int           | wall time of the test process, in nanoseconds |
| `index`       | int           | the test's dispatch index (`<exe> <index>` reruns exactly this test) |
| `exe`         | string        | the dispatcher executable path |
| `output`      | string / null | path to the retained capture file, or `null` |

`kind` is one of:

| `kind`   | Meaning |
|----------|---------|
| `pass`   | exited `0` |
| `exit`   | exited non-zero; `code` carries the exit code |
| `signal` | killed by a signal; `code` carries the signal number |
| `spawn`  | the test executable could not be spawned |
| `other`  | neither exited nor signaled (should not occur) |

`output` references the on-disk capture file holding the test's full stdout and
stderr. It is a path for a `test` whose capture file persists — every failure
(`exit`, `signal`, `other`) — and `null` otherwise: a `pass` file is unlinked on
completion, and a `spawn` failure never produced one. The file is the complete
output (unlike the human readout's 64KB inline excerpt).

```json
{"schema":1,"event":"test","label":"mach.cli.util.path_into:absolute_and_relative","module":"mach.cli.util","file":"./src/cli/util.mach","line":427,"kind":"pass","code":0,"duration_ns":489607,"index":12,"exe":"./out/linux-x86_64/debug/test/mach","output":null}
{"schema":1,"event":"test","label":"builds:cyclic_import","module":"mach.lang.driver","file":"./src/lang/driver.mach","line":142,"kind":"exit","code":1,"duration_ns":146002310,"index":37,"exe":"./out/linux-x86_64/debug/test/mach","output":"./out/linux-x86_64/debug/test/log/37.log"}
```

### `summary`

Emitted once, after every `test` event.

| Field         | Type   | Description |
|---------------|--------|-------------|
| `schema`      | int    | schema version (`1`) |
| `event`       | string | `"summary"` |
| `passed`      | int    | number of passing tests |
| `failed`      | int    | number of non-passing tests |
| `total`       | int    | number of tests run |
| `duration_ns` | int    | the run's wall time, in nanoseconds |

```json
{"schema":1,"event":"summary","passed":437,"failed":1,"total":438,"duration_ns":268000000}
```

### `case`

Emitted by `mach test --list --format json` — one per collected test, with no
run. Carries the static identity fields only (no outcome). `exe` is absent
because `--list` does not build the dispatcher.

| Field    | Type   | Description |
|----------|--------|-------------|
| `schema` | int    | schema version (`1`) |
| `event`  | string | `"case"` |
| `label`  | string | the test's label, verbatim |
| `module` | string | the declaring module's fully-qualified name |
| `file`   | string | source file path of the declaring module |
| `line`   | int    | 1-based source line of the `test` keyword |
| `index`  | int    | the test's dispatch index |

```json
{"schema":1,"event":"case","label":"mach.cli.util.path_into:absolute_and_relative","module":"mach.cli.util","file":"./src/cli/util.mach","line":427,"index":12}
```

## Stream shape

A run is `run_start`, then zero or more `test` (or, under `--list`, zero or more
`case`), then `summary`. A filtered run that matched nothing still brackets an
empty run with `run_start` (`tests: 0`) and `summary`. `--list` emits `case`
lines only — no `run_start` or `summary`.

## See also

- [cli.md](../cli.md) — the `mach test` command and its flags.
