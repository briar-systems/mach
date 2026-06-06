# `mach test` framework corpus

A self-contained, no-std Mach project that exercises the internal `mach test`
framework end to end: test discovery and collection, the `ret 0` = pass /
non-zero = fail convention, exit-code propagation, and the `--list` / `--filter`
flags.

It depends on nothing — it provides its own `_start` — so a `mach test` run here
collects only this project's own tests, with no standard-library noise.

## Running

From this directory, with `mach` on your `PATH` (or via an absolute path to a
built `mach`):

```
mach test                  # run every test
mach test --list           # list the collected tests, do not run them
mach test --filter add     # run only tests whose label contains "add"
mach test --list --filter add
```

Exit codes follow the runner: `0` when every test that ran passed, `1` when any
failed, `2` on a build or internal error.

## Expected output

A default run prints a `PASS`/`FAIL` line per test. The corpus deliberately
includes one failing test (`fails on purpose`, which returns a non-zero status)
so a default run demonstrates a detected failure and a non-zero exit code:

```
PASS add: basic sum
PASS add: negative operands
PASS is_even: even and odd
PASS fall-through is a pass
FAIL fails on purpose
```

Filtering to only-passing tests (e.g. `--filter add`) yields an all-pass run and
exit code `0`. Delete the `fails on purpose` test to make an unfiltered run pass.

See `../../doc/language/test.md` for the full `test` declaration and `mach test`
workflow documentation.
