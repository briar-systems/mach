# `test` — test declaration

A `test` declaration names a block of statements the test runner can
execute on its own. Any module may declare them, and `mach test` collects
every test in the selected artifact's closure and links the selected ones into
one small test binary.
What earns a test, and where it sits, is set by the
[test policy](#test-policy).

## Grammar

```mach fragment
test <identifier> { ... }
```

The name is an identifier, following the ordinary identifier rules. A string
in its place (`test "label" { ... }`) is a compile error located at the string
that names the identifier form. The body is a block of statements. A test takes
no parameters and is not callable from ordinary code; it exists only for the
runner to invoke.

Tests live in their own namespace in each module. A test's name is never in
scope in code, so `test str_len { ... }` and `fun str_len` in one module do not
conflict, and no code can name, call or reference a test. Two tests with the
same name in one module are a compile error located at the second.

A test's **qualified name** is its module path, `#`, and its name:
`std.types.string#str_len__empty`. `#` appears in no identifier or module path,
so a qualified name never collides with another symbol. It is the test's symbol
(hidden, like every other symbol that is not exported), and it is the name
`--list`, `--filter`, the readout and `--format json` show. A debugger takes it
unquoted: `break std.types.string#str_len__empty` in gdb.

Related tests group under a common subject as `subject__case`
(`str_len__empty`, `str_len__multibyte`), and a regression test is named
`regression__*`. Both are conventions the compiler does not check.

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

test is_leap_year__centuries {
    if (!is_leap_year(2000)) { ret 1; }
    if (is_leap_year(1900))  { ret 1; }
    if (is_leap_year(2023))  { ret 1; }
    ret 0;
}

test log__nil_message_does_not_crash {
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
point so the runner can iterate it. Its qualified name is the lowered function's
name. Ordinary builds omit test bodies from IR and object files.

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
collected, and `mach test` links one dispatcher executable in place of the
artifact's normal entry and runs each test through it in its own process.

By default collection is scoped to the current project's own modules: tests
declared in dependency modules are excluded, so a library's own suite never
runs (or fails) as part of your project's `mach test`. Pass `--include-deps`
to collect dependency tests as well — useful when working on a dependency
in-tree. `--filter` narrows the run by qualified name in either mode.

## The `mach test` workflow

`mach test <path>` is `mach build` with a different goal: it builds the
artifact's objects exactly as `mach build` does, adds a test object beside each
module that declares tests, links one test **dispatcher** executable covering
the selected tests, then runs each of them as its own process
(`<exe> <index>`), captures its output, times
it, and renders a per-module readout — collapsing all-passing modules to a
single roll-up line and expanding any module with a failure to show the
failing test's captured output and location. The full flag reference is
`mach help test`; the options that select and shape a run are:

```
--jobs <n>               run up to n test processes at once (default: host CPUs)
--filter <substr>        select only tests whose qualified name contains the substring
--include-deps           also run tests declared in dependency modules
--list                   list the collected tests and exit
--format <human|json>    the live readout, or an NDJSON event stream
--runner <cmd>           launch each test through a host-side command
--timeout <duration>     terminate a test and its process group after the duration
```

A roll-up is `<module>  <ok> ok[  <fail> FAIL]  <duration>`. Each expanded
failure shows `file:line`, the exit code (`(exit N)`), signal (`(signal N)`)
or `(timed out after <duration>)`, the child's captured output indented beneath, and
the exact `rerun:` command; a passing test stays quiet. The run closes with a
summary that re-lists every failure:

```
failures:
  app.main#fails_on_purpose  src/main.mach:11  (exit 3)

1 passed, 1 failed, 2 total  (1ms)
```

The exit code of `mach test`:

- `0` — every test that ran passed.
- `1` — at least one test failed, was killed by a signal, or timed out; also a
  user error such as an unknown flag.
- `2` — a build or internal error before the tests could run, or a test that
  failed for an infrastructure reason (the harness, not the test).
- `3` — an environment failure before the tests could run: a file, directory
  or process operation the machine refused.

These are the codes every `mach` command shares (`mach help <command>` lists
them under `exit:`); `test` only adds what `1` also means.

`--list` prints each selected test's qualified name and the test object that
holds it, and exits without linking or running anything:

```
app.parser#rejects_trailing_comma ./out/linux-x86_64/debug/obj/app/parser.test.o
```

`--filter <substr>` selects before the dispatcher links, so the dispatcher
holds only the selected tests and what they reach, and changing the filter
relinks without recompiling. `--emit` is rejected under `mach test`
(`--emit is not applicable to 'test'; test always builds its internal test
dispatcher`).

### Timeouts

`--timeout <duration>` bounds each spawned test process independently, from
its own spawn, on its whole process group, so a process the test started dies
with it. A test that exceeds the bound is the distinct outcome **timed out**:
it renders as `(timed out after <duration>)`, is counted separately on the
summary line, and is still a failing test for the exit code, so the suite
exits `1`.

```
failures:
  app.main#spins  src/main.mach:4  (timed out after 1s)

0 passed, 1 failed (1 timed out), 1 total  (1.0s)
```

`<duration>` is a positive integer followed by a unit: `ms`, `s`, `m` or `h`
(`30ms`, `30s`, `5m`, `1h`). A bare number, a fraction, zero or any other
unit is a usage error naming the accepted forms. There is no default: omitting
the flag leaves every test unbounded.

### JSON output

`--format json` replaces the readout with one JSON object per line on stdout
(`run_start`, one `test` per result, `summary`; `case` under `--list`), with
build diagnostics kept on stderr. A `test` or `case` event names its test by
qualified name in `name`, beside its `module`, `file`, `line`, test `object`
and dispatcher `index`. A timed-out test reports `"kind":"timeout"` with its
bound in nanoseconds in `timeout_ns`. The schema is versioned (`"schema":2` on
every event) and its writer is `mach.cli.cmd.testing`.

## The runner

A test build compiles every module's object exactly as `mach build` does and
shares it: after `mach build`, `mach test` recompiles no module object. A module
that declares tests or `#[testing]` declarations also gets a **test object**,
`obj/<project>/<module>.test.o`. The compiler (`mach.lang.me.lower.testrunner`)
lowers each of its tests to a zero-parameter, `i32`-returning function under
its qualified name, a symbol that never collides with, reserves, or rewrites a
user symbol.

The test object references every symbol the module's object defines, private
ones included, so a test reads and writes the same globals and calls the same
functions the module's own code does, and every symbol has exactly one
definition. It defines only what the module's object lacks: the tests, the
`#[testing]` declarations, a private function inlined everywhere or called
only from tests, a private global only tests use, and generic instances only
tests use. The module's object never changes for tests, and its cache key
leaves the module's test declarations out, so editing a test recompiles only
that module's test object and relinks. The test object's key is the module
object's key and the module's whole source.

Each run synthesizes one dispatcher object, `test/<artifact>/dispatch.o`,
whose entry selects a test by its index argument, and links it with the test
objects and the module objects into `test/<artifact>/<artifact>`, even for a
library artifact. The link keeps only what the selected tests reach. The
dispatcher is the program's `main`: an artifact's own `main` yields to it in
the link, so its object is linked unchanged.

The dispatcher's entry calls the selected test and exits with its result:
as is when the result is in `0..255`, and `255` otherwise (see
[Semantics](#semantics)). A missing, malformed, or out-of-range index exits
`2`.

`mach test` then keeps up to `--jobs` children in flight, each spawned as
`<exe> <index>`, captures each child's stdout and stderr to a per-test file
under `log/` beside the dispatcher, `test/<artifact>/log/` unless `-o` moves
the dispatcher (a passing test's file is removed on the spot, a failing test's
file stays), and reads its exit status. Results render
in collection order regardless of completion order.

## Which tests run

`mach test` selects the artifact under test the way every command that needs
one artifact does: `--bin <name>` or `--lib <name>` names it; otherwise the sole
artifact the selected target builds is chosen, or among several the one marked
`default = true` (see [manifest.md](manifest.md#artifactname)). It then builds that
artifact's closure, the same module set `mach build` compiles for it, and runs
the tests declared there. Each run tests one artifact, so `$bin.name` in a test
block, and in every module the run compiles, is the artifact under test.

An inline `test name { }` declaration in a module the artifact reaches runs
with no further wiring. A module that exists only for tests, such as a suite
that exercises several modules together, is reached by no artifact and so never
runs on its own. Give such modules a **test artifact**: an ordinary library artifact
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

## Test policy

A change does not need a test of its own. A unit test exists only if it:

- covers a unique surface, duplicating no other test
- covers functionality critical to correctness that cannot be allowed to break
- is deterministic, never depending on timing or performance
- is valuable to check automatically
- covers logic that is not blatantly simple
- checks correctness
- is no more complicated than the code it tests, unless that is unavoidable
- does not pin a problem that no longer exists

Coverage means branches and known failure points, not volume. `str_len` gets
the inputs that exercise each of its branches, not a pile of strings, and a
parser's tests cover its surface concisely, not exhaustively.

Inline tests are small and sit in their module for convenience or because they
need private access. A test lives outside the module it covers to declutter it,
because the test is significant, or, most often, because it exercises several
modules together (see [Which tests run](#which-tests-run)).

Regression tests are a separate kind, and rare: they are kept only for
regressions that are easy to reintroduce, and are named `regression__*`. They
may sit next to unit tests. The name is a convention, not a mechanism.

The compiler's codegen corpus and link cases get the same scrutiny, scoped to
what cannot be tested inside the compiler: the final codegen and link result on
disk.

## See also

- [fun.md](fun.md) — functions; a test body is checked like a function body
- [decorators.md](decorators.md#testing--test-only-declaration) — `#[testing]`
  for declarations that exist only for tests
- [statements.md](statements.md) — `if`/`or`, `ret`, and the other
  statements a test body uses
- [files.md](files.md) — project layout the build (and `mach test`)
  discovers
