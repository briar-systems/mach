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

### ASCII escaping (considered alternative)

The escaper is `ensure_ascii`: it never emits a raw byte `>= 0x80`. Verbatim
UTF-8 passthrough would be equally deterministic and equally valid under RFC
8259 (which mandates UTF-8), and would be smaller and more readable. The
`ensure_ascii` form was chosen for two properties a machine stream benefits
from: it makes no assumption about the consumer's byte-encoding handling, and it
is robust to invalid UTF-8 in the underlying bytes — malformed sequences (a path
on Linux is an arbitrary byte string) decode to the U+FFFD replacement escape
rather than propagating raw or producing invalid JSON. The tradeoff is size and
human readability, which do not matter for a machine stream.

### Paths and ordering

`file`, `exe`, and `output` are emitted exactly as the compiler references them,
which is the form the human readout prints: rooted at the project path as it
was given on the command line (`src/root.mach` for `mach test .`, an absolute
path for `mach test /abs/project`). Resolve a relative one against the working
directory the run was invoked from.

`test` events arrive in **collection order**, the same deterministic order as
the human readout, regardless of completion order; `index` is the stable
per-test dispatch index and equals that position. `run_start` always precedes
every `test`, and `summary` always follows every `test`.

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
{"schema":1,"event":"run_start","tests":2}
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
| `timeout_seconds` | int       | the bound the test exceeded (`kind` = `timeout`); `0` otherwise |
| `duration_ns` | int           | wall time of the test process, in nanoseconds |
| `index`       | int           | the test's collection-order dispatch index (`<exe> <index>` reruns exactly this test; sort by it for declaration order) |
| `exe`         | string        | the dispatcher executable path |
| `output`      | string / null | path to the retained capture file, or `null` |

`kind` is one of:

| `kind`   | Meaning |
|----------|---------|
| `pass`   | exited `0` |
| `exit`   | exited non-zero; `code` carries the exit code |
| `signal` | killed by a signal; `code` carries the signal number |
| `spawn`  | the test executable could not be spawned |
| `timeout`| exceeded `--timeout_seconds`; `timeout_seconds` carries the bound |
| `other`  | neither exited nor signaled (should not occur) |

`output` references the on-disk capture file holding the test's full stdout and
stderr. It is a path for a `test` whose capture file persists — every failure
(`exit`, `signal`, `timeout`, `other`) — and `null` otherwise: a `pass` file is unlinked on
completion, and a `spawn` failure never produced one. The file is the complete
output (unlike the human readout's 64KB inline excerpt).

The capture file is **single-run scoped**: the next `mach test` invocation wipes
the `log/` directory beside the dispatcher — regardless of `--filter` — before
it runs, so read a referenced file before starting another run. The path is
composed to fit exactly, so `output` is never `null` for a failure on account of
path length.

```json
{"schema":1,"event":"test","label":"adds","module":"p1.root","file":"src/root.mach","line":10,"kind":"pass","code":0,"timeout_seconds":0,"duration_ns":257163,"index":0,"exe":"./out/linux-x86_64/debug/test/p1","output":null}
{"schema":1,"event":"test","label":"fails on purpose","module":"p1.root","file":"src/root.mach","line":11,"kind":"exit","code":3,"timeout_seconds":0,"duration_ns":278743,"index":1,"exe":"./out/linux-x86_64/debug/test/p1","output":"./out/linux-x86_64/debug/test/log/1.log"}
{"schema":1,"event":"test","label":"spins","module":"p1.root","file":"src/root.mach","line":4,"kind":"timeout","code":0,"timeout_seconds":1,"duration_ns":1000881714,"index":0,"exe":"./out/linux-x86_64/debug/test/p1","output":"./out/linux-x86_64/debug/test/log/0.log"}
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
| `mismatch`    | int    | tests that exited non-zero or were signaled |
| `unsupported` | int    | tests whose input the gate could not accept |
| `target_unavailable` | int | tests whose tool or target was unavailable |
| `infrastructure_failure` | int | tests that failed for a harness reason |
| `resource_exhaustion` | int | tests that exhausted a bound, including a timeout |
| `timed_out`   | int    | tests that exceeded `--timeout_seconds` |
| `invalid_injection` | int | tests whose injected fault was never reached |

`failed` counts every non-passing test, so a timed-out test is in both `failed`
and `timed_out`, and it exits the suite `1` like any other failure. The exit
code is `2` when `infrastructure_failure` is nonzero, otherwise `1` when
`failed` is nonzero, otherwise `0`; read `timed_out` to separate the class.

```json
{"schema":1,"event":"summary","passed":1,"failed":1,"total":2,"duration_ns":629807,"mismatch":1,"unsupported":0,"target_unavailable":0,"infrastructure_failure":0,"resource_exhaustion":0,"timed_out":0,"invalid_injection":0}
{"schema":1,"event":"summary","passed":0,"failed":1,"total":1,"duration_ns":1001365600,"mismatch":0,"unsupported":0,"target_unavailable":0,"infrastructure_failure":0,"resource_exhaustion":1,"timed_out":1,"invalid_injection":0}
```

The second line is a run whose one test timed out: it counts under `failed`,
`timed_out`, and `resource_exhaustion`.

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
{"schema":1,"event":"case","label":"adds","module":"p1.root","file":"src/root.mach","line":10,"index":0}
{"schema":1,"event":"case","label":"fails on purpose","module":"p1.root","file":"src/root.mach","line":11,"index":1}
```

## Stream shape

A run is `run_start`, then zero or more `test` (or, under `--list`, zero or more
`case`), then `summary`. A filtered run that matched nothing still brackets an
empty run with `run_start` (`tests: 0`) and `summary`. `--list` emits `case`
lines only — no `run_start` or `summary`.

## See also

- [cli.md](../cli.md) — the `mach test` command and its flags.
