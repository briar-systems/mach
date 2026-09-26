# `test` — test declaration

A `test` declaration names a block of statements the test runner can
execute on its own. Tests live alongside the code they exercise: any
module may declare them, and `mach test` collects every test in the
selected artifact's closure into a single test binary.

## Grammar

```mach fragment
test "label" { ... }
```

The label is required: a **string literal**, or an identifier, which
labels the test exactly as the string literal of the same text does
(`test leap_year { ... }` is `test "leap_year" { ... }`). The body is a
block of statements. A test takes no parameters and is not callable from ordinary
code; it exists only for the runner to invoke.

`test` is a reserved keyword and appears at module (declaration) scope,
the same level as `fun`, `rec`, and `val`. Visibility modifiers such as
`pub` are syntactically accepted before `test` but carry no meaning — a
test is never part of a module's public surface.

## Examples

```mach
use std.runtime;
use std.types.bool.bool;

fun is_leap_year(y: i64) bool {
    if (y % 400 == 0) { ret 1; }
    if (y % 100 == 0) { ret 0; }
    ret y % 4 == 0;
}

fun debug(msg: *u8) {
    if (msg == nil) { ret; }
}

fun info(msg: *u8) {
    if (msg == nil) { ret; }
}

test "date: is_leap_year" {
    if (!is_leap_year(2000)) { ret 1; }
    if (is_leap_year(1900))  { ret 1; }
    if (is_leap_year(2023))  { ret 1; }
    ret 0;
}

test "log: nil message does not crash" {
    debug(nil);
    info(nil);
    ret 0;
}
```

A test body may use anything in scope in the enclosing module, just like a
function body.

## Semantics

Every build resolves and type-checks each `test` body in the modules its artifact
entry reaches. `mach test` builds exactly what `mach build` builds for the same
artifact: the closure its entry reaches through `use` and `fwd`. A module no
selected artifact reaches is not loaded under test either, so it is not checked and
its tests do not run (see [Which tests run](#which-tests-run)). Each
test then lowers to a zero-parameter, `i32`-returning function tagged as a test entry
point so the runner can iterate it. The label is interned and becomes the lowered
function's name. Ordinary builds omit test bodies from IR and object files.

A helper or fixture that exists only for tests is marked
[`#[testing]`](decorators.md#testing--test-only-declaration). It gets the same
treatment: checked in every build and omitted from ordinary ones. Only a test
body or another `#[testing]` declaration may reference it.

The body is checked against an `i32` return type. A test reports its result
through that return value, treated as a process-style status in the range
`0..255`:

- `ret 0` — pass.
- any `ret N` with `N` in `1..255` — fail, reported as `(exit N)`.
- falling off the end of the body returns `0` (the default terminator for
  a non-void function is a zero return), so a body that never returns
  explicitly is treated as a pass.

The result is the test process's exit status, and a process exit status is
eight bits wide on every host (`mach test` reads the same eight bits on
windows). The range is therefore part of the protocol, enforced at both
ends:

- A `ret` in a test body whose value is a literal (or a literal-shaped
  expression: a negated literal, or an arithmetic expression over literals)
  outside `0..255` is a compile error located at the `ret`, naming the value:
  `test result 256 is outside the status range 0..255`. An ordinary function
  returning the same value is unaffected; only test bodies carry the range.
- A result computed at run time that lands outside `0..255` — a bit mask that
  has grown past eight bits, a negative code — is folded by the dispatcher to
  `255` before the process exits. It is reported as `(exit 255)` and is
  always a failure. The low eight bits are never used on their own, so a
  result of `256` cannot read as a pass.

Within the range the compiler attaches no special pass/fail meaning to
particular non-zero codes, nor does it provide built-in assertion
intrinsics. A test signals failure by returning non-zero — typically by
returning early from a failed check, as in the example above. A test that
accumulates a bit mask must keep it within eight bits; past that, return
the ordinal of the first failing check instead.

### Collection across modules

Tests are not tied to a single file. Every `test` declaration in every
module of the artifact under test that belongs to the current project is
collected, and `mach test` builds one dispatcher executable in place of the
artifact's normal entry and runs each test through it in its own process.

By default collection is scoped to the current project's own modules: tests
declared in dependency modules are excluded, so a library's own suite never
runs (or fails) as part of your project's `mach test`. Pass `--include-deps`
to collect dependency tests as well — useful when working on a dependency
in-tree. `--filter` narrows the run by test name in either mode.

## The `mach test` workflow

`mach test <path>` is `mach build` with a different goal: it builds one test
**dispatcher** executable covering every collected test, then runs each
selected test as its own process (`<exe> <index>`), captures its output, times
it, and renders a per-module readout — collapsing all-passing modules to a
single roll-up line and expanding any module with a failure to show the
failing test's captured output and location. The full flag reference is
`mach help test`; the options that select and shape a run are:

```
--jobs <n>               run up to n test processes at once (default: host CPUs)
--filter <substr>        run only tests whose label contains the substring
--include-deps           also run tests declared in dependency modules
--list                   list the collected tests and exit
--format <human|json>    the live readout, or an NDJSON event stream
--runner <cmd>           launch each test through a host-side command
--timeout_seconds <n>    terminate a test and its process group after n seconds
```

A roll-up is `<module>  <ok> ok[  <fail> FAIL]  <duration>`. Each expanded
failure shows `file:line`, the exit code (`(exit N)`), signal (`(signal N)`)
or `(timed out after <n>s)`, the child's captured output indented beneath, and
the exact `rerun:` command; a passing test stays quiet. The run closes with a
summary that re-lists every failure:

```
failures:
  fails on purpose  src/main.mach:11  (exit 3)

1 passed, 1 failed, 2 total  (1ms)
```

The exit code of `mach test`:

- `0` — every test that ran passed.
- `1` — at least one test failed, was killed by a signal, or timed out; also a
  user error such as an unknown flag.
- `2` — a build or internal error before the tests could run, or a test that
  failed for an infrastructure reason (the harness, not the test).

`--list` enumerates the collected tests and exits without running them.
`--filter <substr>` selects at run time; the built dispatcher is identical
regardless of filter. `--emit` is rejected under `mach test`
(`--emit is not applicable to 'test'; test always builds its internal test
dispatcher`).

### Timeouts

`--timeout_seconds <n>` bounds each spawned test process independently, from
its own spawn, on its whole process group, so a process the test started dies
with it. A test that exceeds the bound is the distinct outcome **timed out**:
it renders as `(timed out after <n>s)`, is counted separately on the summary
line, and is still a failing test for the exit code, so the suite exits `1`.

```
failures:
  spins  src/main.mach:4  (timed out after 1s)

0 passed, 1 failed (1 timed out), 1 total  (1.0s)
```

`<n>` is a positive integer number of seconds with no default: omitting the
flag leaves every test unbounded.

### JSON output

`--format json` replaces the readout with one JSON object per line on stdout
(`run_start`, one `test` per result, `summary`; `case` under `--list`), with
build diagnostics kept on stderr. A timed-out test reports `"kind":"timeout"`
with its bound in `timeout_seconds`. The schema is versioned (`"schema":1`
on every event) and its writer is `mach.cli.cmd.testing`.

## The runner

The compiler (`mach.lang.me.lower.testrunner`) lowers every collected test to
a zero-parameter, `i32`-returning function under a compiler-private symbol
that never collides with, reserves, or rewrites a user symbol, and synthesizes
one dispatcher object whose entry selects a test by its index argument. That
object links with the project's objects into a single executable, even for a
library artifact; in a test build the project's own entry is neutralised so
the dispatcher is the sole program entry.

The dispatcher's entry calls the selected test and exits with its result:
as is when the result is in `0..255`, and `255` otherwise (see
[Semantics](#semantics)). A missing, malformed, or out-of-range index exits
`2`.

`mach test` then keeps up to `--jobs` children in flight, each spawned as
`<exe> <index>`, captures each child's stdout and stderr to a per-test file
under `log/` beside the dispatcher (a passing test's file is removed on the
spot, a failing test's file stays), and reads its exit status. Results render
in collection order regardless of completion order.

## Which tests run

`mach test` selects the artifact under test the way every command that needs
one artifact does: `--bin <name>` or `--lib <name>` names it; otherwise the sole
artifact the selected target builds is chosen, or among several the one marked
`default = true` (see [manifest.md](manifest.md#artifactname)). It then builds that
artifact's closure, the same module set `mach build` compiles for it, and runs
the tests declared there. Each run tests one artifact, so `$bin.name` in a test
block, and in every module the run compiles, is the artifact under test.

Tests live inline alongside the code they cover: a `test "..." { }` declaration
in a module the artifact reaches runs with no further wiring. A module that
exists only for tests, a suite too large to sit beside the code or a harness
that drives the whole compiler, is reached by no artifact and so never runs on
its own. Give such modules a **test artifact**: an ordinary library artifact
whose entry `use`s each of them, tested by name.

```toml
[artifact.app]
kind    = "bin"
default = true
entry   = "main.mach"
out     = "bin/app"
targets = ["*"]
link    = []
need    = []

# the test-only modules no other artifact reaches
[artifact.tests]
kind    = "static"
entry   = "test/all.mach"
out     = "lib/tests"
targets = ["*"]
link    = []
need    = []
```

```mach fragment
# src/test/all.mach
use std.runtime;

use app.test.parser;
use app.test.roundtrip;
```

`mach test .` then runs the tests `app` reaches and `mach test . --lib tests`
the test-only suites. The entry reaches the runtime's startup (`use std.runtime;`)
because a library artifact's closure is all the test dispatcher links. Marking
`app` `default = true` keeps `mach build .` and `mach check .` to `app`, since with
no selector they take the marked artifacts; `mach build . --lib tests` builds the
test artifact.

## See also

- [fun.md](fun.md) — functions; a test body is checked like a function body
- [decorators.md](decorators.md#testing--test-only-declaration) — `#[testing]`
  for declarations that exist only for tests
- [statements.md](statements.md) — `if`/`or`, `ret`, and the other
  statements a test body uses
- [files.md](files.md) — project layout the build (and `mach test`)
  discovers
