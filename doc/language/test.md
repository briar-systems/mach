# `test` — test declaration

A `test` declaration names a block of statements the test runner can
execute on its own. Tests live alongside the code they exercise: any
module may declare them, and `mach test` collects every test across the
project into a single test binary.

## Grammar

```mach
test "label" { ... }
```

The label is a **string literal** — it is required, and it must be a
string literal (not an identifier or a bare word). The body is a block of
statements. A test takes no parameters and is not callable from ordinary
code; it exists only for the runner to invoke.

`test` is a reserved keyword and appears at module (declaration) scope,
the same level as `fun`, `rec`, and `val`. Visibility modifiers such as
`pub` are syntactically accepted before `test` but carry no meaning — a
test is never part of a module's public surface.

## Examples

```mach
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
entry reaches. `mach test` roots its build at every module in the current project's
source tree, so it checks and collects tests in modules no artifact imports. Each
test then lowers to a zero-parameter, `i32`-returning function tagged as a test entry
point so the runner can iterate it. The label is interned and becomes the lowered
function's name. Ordinary builds omit test bodies from IR and object files.

The body is checked against an `i32` return type. A test reports its result
through that return value, treated as a process-style status:

- `ret 0` — pass.
- any non-zero `ret N` — fail.
- falling off the end of the body returns `0` (the default terminator for
  a non-void function is a zero return), so a body that never returns
  explicitly is treated as a pass.

The return value is an ordinary integer status; the compiler does not
attach any special pass/fail meaning to particular non-zero codes, nor does
it provide built-in assertion intrinsics. A test signals failure by
returning non-zero — typically by returning early from a failed check, as
in the example above.

> **Note.** The convention above (`0` = pass, non-zero = fail) is the
> interpretation the test runner applies to each test's exit status; it is
> not enforced by the type system. Existing standard-library tests are not
> all consistent about which non-zero codes they use, and some return `1`
> on the success path. When writing new tests, prefer `ret 0` for pass and
> a non-zero `ret` for failure.

### Collection across modules

Tests are not tied to a single file. Every `test` declaration in every
module of the current project is collected, and `mach test` builds one
dispatcher executable in place of the project's normal entry and runs each
test through it in its own process.

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
failing test's captured output and location. The full flag reference is in
[cli.md](../cli.md#mach-test); the options that select and shape a run are:

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
  fails on purpose  src/root.mach:11  (exit 3)

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
  spins  src/root.mach:4  (timed out after 1s)

0 passed, 1 failed (1 timed out), 1 total  (1.0s)
```

`<n>` is a positive integer number of seconds with no default: omitting the
flag leaves every test unbounded.

### JSON output

`--format json` replaces the readout with one JSON object per line on stdout
(`run_start`, one `test` per result, `summary`; `case` under `--list`), with
build diagnostics kept on stderr. A timed-out test reports `"kind":"timeout"`
with its bound in `timeout_seconds`. The schema is versioned and documented in
[tooling/test-json.md](../tooling/test-json.md).

## The runner

The compiler (`mach.lang.me.lower.testrunner`) lowers every collected test to
a zero-parameter, `i32`-returning function under a compiler-private symbol
that never collides with, reserves, or rewrites a user symbol, and synthesizes
one dispatcher object whose entry selects a test by its index argument. That
object links with the project's objects into a single executable, even for a
library artifact; in a test build the project's own entry is neutralised so
the dispatcher is the sole program entry.

`mach test` then keeps up to `--jobs` children in flight, each spawned as
`<exe> <index>`, captures each child's stdout and stderr to a per-test file
under `log/` beside the dispatcher (a passing test's file is removed on the
spot, a failing test's file stays), and reads its exit status. Results render
in collection order regardless of completion order.

Tests live inline alongside the code they cover: write `test "..." { }`
declarations directly in the relevant `src/` module, or group them under
`src/test/`. Every test declaration across the project's modules is collected
automatically, with no separate corpus project.

## See also

- [fun.md](fun.md) — functions; a test body is checked like a function body
- [statements.md](statements.md) — `if`/`or`, `ret`, and the other
  statements a test body uses
- [files.md](files.md) — project layout the build (and `mach test`)
  discovers
- [../cli.md](../cli.md#mach-test) — every `mach test` flag
- [../tooling/test-json.md](../tooling/test-json.md) — the `--format json` schema
