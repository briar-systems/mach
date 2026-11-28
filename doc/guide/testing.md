# Testing

A `test` is a named block that `mach test` runs as a process of its own. Tests sit in the module they exercise.

## Writing tests

```mach
# src/leap.mach
use std.types.bool.bool;

pub fun is_leap(y: i64) bool {
    if (y % 400 == 0) { ret 1; }
    if (y % 100 == 0) { ret 0; }
    ret y % 4 == 0;
}

#[testing]
fun sample_year() i64 {
    ret 2000;
}

test is_leap__centuries {
    if (!is_leap(sample_year())) { ret 1; }
    if (is_leap(1900))           { ret 2; }
}

test is_leap__needs_a_clock {
    ret 77;
}

test is_leap__odd_years {
    if (!is_leap(2023)) { ret 3; }
}
```

The value a test returns is its exit status. The last test above is wrong on purpose.

| Status | Outcome |
|---|---|
| `0`, or reaching the end of the body | pass |
| `77` | skip, for a test that can check nothing on this host |
| any other value up to `255` | fail |

- There are no assertions. A failed check returns early, and a distinct status per check says which one failed.
- A test sees everything in its module, private names included. Nothing can call a test, and its name never clashes with a function's. `subject__case` is the naming convention.
- A [`#[testing]`](decorators.md) declaration exists only in a test build. Only a test or another `#[testing]` declaration may name it.
- A test that fits some targets only goes inside an [`$if`](comptime.md).

## Where tests live

There is no test directory and no test artifact. `mach test` runs the tests in every module of the project that the artifact's entry reaches through `use` and `fwd`.

## Running tests

```sh
mach test .
```

```text
error[test.failed] test is_leap__odd_years exited with 3
   @ src/leap.mach:24:1
24 | test is_leap__odd_years {
   | ^^^^^^^^^^^^^^^^^^^^^^^
   note rerun it alone with ./out/linux-x86_64/debug/test/cal/cal 2

failed cal.leap
  tests    3
  failed   1
  skipped  1
  time     3.5ms
failed cal
  target   linux-x86_64 debug
  tests    3
  failed   1
  skipped  1
  time     690ms
```

Each failure is reported as it happens, then one result per module and one for the run. The command exits 0 when nothing failed and 1 otherwise.

| Flag | Effect |
|---|---|
| `--filter <substr>` | Run only the tests whose name, written `cal.leap#is_leap__centuries`, contains the text |
| `--list` | Print the test names and run nothing |
| `--include-deps` | Run the tests of dependencies too |
| `--timeout <duration>` | Stop each test after a time such as `30s`. The default is `60s` |
| `--threads <n>` | Run up to `n` tests at once |

The full rules are in the reference for [`test`](../language/test.md).
