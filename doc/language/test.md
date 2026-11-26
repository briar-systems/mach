# `test`: test declaration

A `test` declaration names a block of statements the test runner can
execute on its own. Any module may declare them, and `mach test` collects
every test in the selected artifact's closure and links the selected ones into
one small test binary.

## Grammar

```mach fragment
test <identifier> { ... }
```

The name is an identifier, following the ordinary identifier rules. A string
in its place (`test "label" { ... }`) is the compile error `test.string_name`,
located at the string, and its message names the identifier form. The body is
a block of statements. A test takes no parameters and is not callable from
ordinary code. It exists only for the runner to invoke.

Tests live in their own namespace in each module. A test's name is never in
scope in code, so `test str_len { ... }` and `fun str_len` in one module do not
conflict, and no code can name, call or reference a test. Two tests with the
same name in one module are the compile error `test.duplicate`, located at the
second.

A test's **qualified name** is its module path, `#`, and its name:
`std.types.string#str_len__empty`. `#` appears in no identifier or module path,
so a qualified name never collides with another symbol. It is the test's symbol
(hidden, like every other symbol that is not exported), and it is the name
`--list`, `--filter`, the readout and the test records show. A debugger takes it
unquoted: `break std.types.string#str_len__empty` in gdb.

Related tests group under a common subject as `subject__case`
(`str_len__empty`, `str_len__multibyte`). This is a convention the compiler
does not check.

`test` is a reserved keyword and appears at module (declaration) scope,
the same level as `fun`, `rec`, and `val`. Visibility modifiers such as
`pub` are syntactically accepted before `test` but carry no meaning. A
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
`0..255`. There are three outcomes:

- `ret 0`: pass.
- `ret 77`: skip. The test could not check anything on this host, so it
  neither passes nor fails. It does not fail the run, and the runner counts it
  apart from the passes.
- any other `ret N` with `N` in `1..255`: fail, reported as `(exit N)`.

Falling off the end of the body returns `0` (the default terminator for a
non-void function is a zero return), so a body that never returns explicitly
is treated as a pass. `77` is the skip status automake, meson and TAP use. std's
panic and `os.abort` exit with `255`, so a crash never reads as a skip.

The result is the test process's exit status, and a process exit status is
eight bits wide on every host (`mach test` reads the same eight bits on
windows). The range is therefore part of the protocol, enforced at both
ends:

- A `ret` in a test body whose value is a literal (or a literal-shaped
  expression: a negated literal, or an arithmetic expression over literals)
  outside `0..255` is the compile error `test.status_range`, located at the
  `ret` and naming the value: `test result 256 is outside the status range
  0..255`. An ordinary function returning the same value is unaffected. Only
  test bodies carry the range.
- A result computed at run time that lands outside `0..255`, such as a bit mask that
  has grown past eight bits or a negative code, is folded by the dispatcher to
  `255` before the process exits. It is reported as `(exit 255)` and is
  always a failure. The low eight bits are never used on their own, so a
  result of `256` cannot read as a pass.

Within the range the compiler attaches no meaning to any code but `0` and
`77`, nor does it provide built-in assertion or skip intrinsics. A test signals failure by returning non-zero, typically by
returning early from a failed check, as in the example above. A test that
accumulates a bit mask must keep it within eight bits. Past that, return
the ordinal of the first failing check instead.

### Collection across modules

Tests are not tied to a single file. Every `test` declaration in every
module of the artifact under test that belongs to the current project is
collected, and `mach test` links one dispatcher executable in place of the
artifact's normal entry and runs each test through it in its own process.

By default collection is scoped to the current project's own modules: tests
declared in dependency modules are excluded, so a library's own suite never
runs (or fails) as part of your project's `mach test`. Pass `--include-deps`
to collect dependency tests as well, which is useful when working on a dependency
in-tree. `--filter` narrows the run by qualified name in either mode.

## The `mach test` workflow

`mach test <path>` is `mach build` with a different goal: it builds the
artifact's objects exactly as `mach build` does, adds a test object beside each
module that declares tests, links one test **dispatcher** executable covering
the selected tests, then runs each of them as its own process
(`<exe> <index>`), captures its output, times
it, and writes a result per module as its tests finish, each failure as a
`test.failed` diagnostic the moment its test finishes, then the run's result.
What `-v` and
`-vv` add, and the order lines come out in, is the [readout](readout.md). The
full flag reference is `mach help test`, and the options that select and shape a
run are:

```
--threads <n>            compile and link on n threads and run up to n test processes at once (default: host CPUs)
--filter <substr>        select only tests whose qualified name contains the substring
--include-deps           also run tests declared in dependency modules
--dep-warnings           also show dependencies' warnings, each under its own allow and #[expect]
--list                   list the collected tests and exit
--report <human|json|lines>  write the run's records to stderr as NDJSON or record lines
--runner <cmd>           launch each test through a host-side command, in place of the target's runner
--timeout <duration>     stop each test after the duration (default: 60s)
--allow-unrunnable       exit 0 when nothing selected can run on this host, after building it
```

A module's result is `passed`, `failed` when any of its tests failed, or
`skipped` when every one was skipped, then the module, and as fields its count
of `tests`, how many `failed`, `timed out` and were `skipped`, each only when
not zero, and its slowest test's `time`. It is written once the module's last
test finishes, so the results come out in the order modules finish. A failure is an `error[test.failed]`
diagnostic, written the moment its test finishes: why it failed (`exited with
N`, `was stopped by signal N`, `timed out after <duration>`), where the test is
declared, then as notes the child's captured output, where its whole output is
when the capture was cut, and the command that reruns it alone. A passing or
skipped test stays quiet. The run's result closes the run: `passed` or
`failed` for the artifact, its `target`, the same counts as a module's, and the
time since the command began (see [readout.md](readout.md#the-test-readout)):

```
passed app.parser
  tests  1
  time   323us
error[test.failed] test fails_on_purpose exited with 3
   @ src/bin/main.mach:13:1
13 | test fails_on_purpose {
   | ^^^^^^^^^^^^^^^^^^^^^^^
   note rerun it alone with ./out/linux-x86_64/debug/test/app/app 1

failed app.bin.main
  tests    3
  failed   1
  skipped  1
  time     615us
failed app
  target   linux-x86_64 debug
  tests    4
  failed   1
  skipped  1
  time     764ms
```

The exit code of `mach test`:

- `0`: every test that ran passed or skipped. A project that declares no
  test runs none, reports `passed <artifact>` with `tests  0` and exits `0`.
- `1`: at least one test failed, was killed by a signal, or timed out. It is
  also a user error, such as an unknown flag, a rejected manifest, or a source
  the build refuses (a test body that does not compile).
- `2`: an internal error of the compiler before the tests could run, or a test
  that failed for an infrastructure reason (the harness, not the test).
- `3`: an environment failure before the tests could run: storage, or a file,
  directory or process operation the machine refused.

These are the codes every `mach` command shares. `test` only adds what `1`
also means.

`--list` prints each selected test's qualified name and the test object that
holds it, the names padded to the widest, and exits without linking or running
anything:

```
app.bin.main#passes                ./out/linux-x86_64/debug/obj/app/bin/main.test.o
app.bin.main#fails_on_purpose      ./out/linux-x86_64/debug/obj/app/bin/main.test.o
app.bin.main#skips_here            ./out/linux-x86_64/debug/obj/app/bin/main.test.o
app.parser#rejects_trailing_comma  ./out/linux-x86_64/debug/obj/app/parser.test.o
```

`--filter <substr>` selects before the dispatcher links, so the dispatcher
holds only the selected tests and what they reach, and changing the filter
relinks without recompiling. `--emit` is rejected under `mach test` with
`cli.flag_inapplicable` (`'--emit' does not apply to test. test always builds
its internal test dispatcher`).

### Timeouts

Every test runs under a bound, 60 seconds unless `--timeout <duration>` gives
another. The bound applies to each spawned test process independently, from
its own spawn. Tests declare no time limits of their own: a test that waits for
an event waits, and one that hangs is ended by its bound. When the bound
passes, the test's process group is asked to stop (`SIGTERM` on Linux and
macOS) and given one second. Whatever is still alive in the group then is
killed as a group (`SIGKILL`), so no process the test started outlives it,
even one that ignores the request. The second runs out early once nothing is
left in the group. On Windows the test's job object is terminated, which ends
every process in it. A test that exceeds
the bound is the distinct outcome **timed out**: its diagnostic says `timed out
after <duration>` with the command that reruns it, it is counted separately on
the module's and the run's results, and it is still a failing test for the
exit code, so the suite exits `1`.

```
error[test.failed] test spins timed out after 1s
  @ src/bin/main.mach:8:1
8 | test spins {
  | ^^^^^^^^^^^^
  note rerun it alone with ./out/linux-x86_64/debug/test/app/app 0

failed app.bin.main
  tests      1
  failed     1
  timed out  1
  time       1.0s
failed app
  target     linux-x86_64 debug
  tests      1
  failed     1
  timed out  1
  time       1.2s
```

A timed-out test counts in the `run_end` record's `resource_exhaustion` beside
`timed_out`.

`<duration>` is a positive integer followed by a unit: `ms`, `s`, `m` or `h`
(`30ms`, `30s`, `5m`, `1h`). A bare number, a fraction, zero or any other
unit is a usage error naming the accepted forms, so no value removes the bound.
The default is rendered in the largest whole unit, so a test that reaches it
reports `timed out after 1m`.

### Records

`--report json` or `--report lines` writes the run as records on stderr beside the build's
diagnostics: a `run_start`, one `test` per result as it finishes and a
`run_end` for each (target, profile) that runs, a `skip` for one that is built
and not run, a `case` per test under `--list`, and the closing `summary`, as
[report-records.md](report-records.md#test-records) describes. A `test` or
`case` record names its test by qualified name in `name`, beside its `module`,
`file`, `line` and dispatcher `index`. A skipped test reports
`"outcome":"skip"` and counts in the `run_end` record's `skipped`. A timed-out test reports
`"outcome":"timeout"` with its bound in nanoseconds in `timeout_ns`. The human
results and `test.failed` diagnostics are not written then.

## The runner

A test build compiles every module's object exactly as `mach build` does and
shares it: after `mach build`, `mach test` recompiles no module object. A module
that declares tests or `#[testing]` declarations also gets a **test object**,
`obj/<project>/<module>.test.o`. The compiler (`mach.lower.testrunner`)
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
the link, so its object is linked unchanged. The process entry that calls `main`
comes from the artifact's closure as it does for a program, never from the test
build, so a library's entry states `use std.runtime;` as `mach init --lib`
writes it. A closure that reaches no process entry is refused at the artifact's
`entry` key, naming the missing entry symbol and the `use` that supplies it (see
[files.md](files.md#executable-entry)).

The dispatcher's entry calls the selected test and exits with its result:
as is when the result is in `0..255`, and `255` otherwise (see
[Semantics](#semantics)). A missing, malformed, or out-of-range index exits
`2`.

`mach test` then keeps up to `--threads` children in flight, each spawned as
`<exe> <index>`, or through the target's runner (see [Which tests run](#which-tests-run)), captures each child's stdout and stderr to a per-test file
under `log/` beside the dispatcher, `test/<artifact>/log/` unless `-o` moves
the dispatcher, named by the test's position in the run (a passing or skipped
test's file is removed on the spot, a failing test's file stays), and reads its
exit status: `0` is a pass, `77` a skip, and any other status, a signal or a
timeout a failure. Results render as the tests finish, each module's line once
its last test has.

## Which tests run

`mach test` selects its cells with `-a`, `-t` and `-p` as every command does (see
[build.md](build.md#selection-and-the-build-matrix)): each takes an exact
name or a glob and repeats, and `--all` fills every axis no option names with
`*`. With no `-a`, the sole artifact the selected target builds is chosen, or
among several the one marked `default = true` (see
[manifest.md](manifest.md#artifactname)). Each selected artifact's closure, the
same module set `mach build` compiles for it, is built into a test dispatcher for
each selected target and profile, so `$bin.name` in a test block, and in every
module a dispatcher compiles, is the artifact under test.

Tests run once per (target, profile). When several artifacts are selected there,
the tests they reach are combined and each qualified name runs once. A target that
declares a [`runner`](manifest.md#targetname) runs its tests through it, whether or
not it is the host: each test launches as the expanded runner followed by its index,
`{artifact.out}` naming the dispatcher. A target whose `sys` and `arch` are the host's
and that declares none runs each test as `<exe> <index>`. Any other target is built
and reported on a `skip` line, since an emulator the host happens to have is not the
target unless the manifest says so. `--runner <cmd>` replaces the target's runner for
one invocation, launching each test as `<cmd> <exe> <index>`, and needs the selection
to resolve to one (artifact, target, profile), foreign or not.

A [finished-module target](manifest.md#finished-module-targets), a shader on
`vulkan`, links no executable, so it has no dispatcher. A
cell on one is built as `mach build` builds it, its module written to the
artifact's own `out`, and its (target, profile) is reported as a `skipped` result
that gives the reason while the other cells build and run:

```
skipped shader
  target   spirv debug
  reason   a finished-module target links no test executable
  instead  built, not run
```

Under `--list` such a cell is type checked, not built, and its result says
`instead  checked, not listed`. This holds whether the selection named the cell exactly or reached it
through a glob or `--all`, so `mach test --all` also catches a shader that does not
compile. `--runner` over a finished-module cell is refused with
`target.output_unsupported`, since there is no program to launch, and a selection
whose every target is a finished-module one exits `1` with `target.not_runnable`
unless `--allow-unrunnable` accepts it.

A test launched through a runner is judged by the runner's exit status under the
same rule as a native test: `0` is a pass, `77` a skip, and anything else a failure.
mach adds nothing for the guest: how a program under an emulator gets its status
out, through semihosting, a debug-exit port or the emulator's own passthrough, is
the runner's business, and `wine`, for one, passes the program's status through. The
note that reruns a failure shows the whole launch command. A run that executed
nothing exits `1`, naming the host, so a passing run always ran tests. A
selection with nothing runnable on the host exits `0` under `--allow-unrunnable`,
after building it and printing each skip line, and a build failure still exits non-zero.

An inline `test name { }` declaration in a module the artifact reaches runs
with no further wiring, and that is the only way a test is placed: a test is
found through the import chain from an artifact's entry, never by the manifest.
There is no test-only artifact and no module that exists only to gather tests.

A test that can run only on some hosts sits in a declaration-level `$if`, so on
any other host it is absent rather than reported as a pass:

```mach fragment
$if ($mach.build.sys != $mach.sys.windows) {
    test spawn__a_shell_script_runs {
        ret 0;
    }
}
```

A test whose gate is only known at run time, such as a CPU feature or a tool
the host may lack, checks it first and returns `77` when it is closed, so the
run reports a skip rather than a pass that checked nothing:

```mach fragment
test crypto__aes128_matches_fips_197 {
    if (!cpu_has_aes()) { ret 77; }
    ret 0;
}
```

A test that checks several arms, each with its own gate, fails when any arm
fails and otherwise returns `77` when any arm skipped.

## See also

- [fun.md](fun.md) - functions, and a test body is checked like a function body
- [decorators.md](decorators.md#testing--test-only-declaration) - `#[testing]`
  for declarations that exist only for tests
- [statements.md](statements.md) - `if`/`or`, `ret`, and the other
  statements a test body uses
- [files.md](files.md) - project layout the build (and `mach test`)
  discovers
- [readout.md](readout.md) - what `mach test` writes, at each level, and when
