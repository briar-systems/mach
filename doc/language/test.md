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

Each `test` lowers to a zero-parameter, `i32`-returning function tagged as
a test entry point so the runner can iterate it. The label is interned and
becomes the lowered function's name.

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
module that is part of the build — the project and its dependencies — is
collected. `mach test` drives a build whose entry point is a synthesized
test runner that iterates the collected tests rather than the project's
normal `main`.

> **Note.** Because dependency tests are collected too, a project that
> depends on a library carrying tests that do not follow the `0` = pass
> convention will see those tests reported as failures. Use `--filter` to
> scope a run to your own tests while a dependency's suite is being
> reconciled.

## The `mach test` workflow

`mach test` builds the project with the test-runner entry point and then
runs the resulting binary, surfacing its exit code:

```
usage: mach test [options]

build the project's tests and run them.

options:
  --filter <pattern>  run only tests whose name contains <pattern>
  --list              list the collected tests and exit

exit: 0 all passed, 1 any failed, 2 build/internal error
```

The command itself does two things:

1. **Build.** It runs the normal build pipeline with the test-runner entry
   point. A build failure (or a missing test binary) makes `mach test`
   exit `2` without running anything.
2. **Run.** It execs the produced test binary, forwarding `--filter
   <pattern>` and `--list` through to the binary as its own arguments so
   the runner can select or enumerate tests itself.

The exit code of `mach test` follows the runner:

- `0` — every test that ran passed.
- `1` — at least one test failed (also returned if the binary is killed by
  a signal rather than exiting normally).
- `2` — a build or internal error before the tests could run.

`--list` enumerates the collected tests and exits without running them.
`--filter <pattern>` restricts the run (or the `--list` output) to tests
whose label contains the given pattern.

## The runner

The runner is synthesized in IR by the compiler (`mach.lang.me.lower.testrunner`)
during a test build and codegen'd into one extra object that joins the link.
For each collected test it emits a body-less extern referencing the test's
linkage name plus a private `.rodata` global holding the label; the synthesized
`main` then walks the table. In a test build the project's own `main` is
neutralised (turned into a body-less extern) so the runner's `main` is the sole
program entry, and an executable is always linked — even for a library target.

The runner prints one line per executed test, `PASS <label>` or `FAIL <label>`,
to stdout, and returns `0` when every selected test passed and `1` otherwise.
It reads `--list` / `--filter` from its own argv (forwarded by `mach test`). The
only OS interaction is a `write` syscall emitted as a small inline-asm helper,
so the runner has no standard-library dependency.

Tests live inline alongside the code they cover: write `test "..." { }`
declarations directly in the relevant `src/` module, or group them under
`src/test/`. The runner collects every `FN_FLAG_TEST` across modules, so
src-local tests are discovered automatically with no separate corpus project.

## See also

- [fun.md](fun.md) — functions; a test body is checked like a function body
- [statements.md](statements.md) — `if`/`or`, `ret`, and the other
  statements a test body uses
- [files.md](files.md) — project layout the build (and `mach test`)
  discovers
