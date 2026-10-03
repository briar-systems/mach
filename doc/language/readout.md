# The readout

`mach build`, `mach check` and `mach test` report their progress as it
happens. The compiler and the test runner report each event the moment it
happens: a unit starting, a phase closing, a module finishing a phase, a
diagnostic, a failure, a test finishing. The command renders each event as it
arrives, in human text or as JSON records, so nothing waits for the end of the
build or the run to be written. This page is the contract for what each level
shows, on which stream, and when.

## Levels

| Flag | `build` and `check` | `test` |
|---|---|---|
| none | diagnostics, failures, the `building <artifact>` banner when the plan has several units, the closing tally | the build's output as for `build`, then one roll-up line per module and the closing summary |
| `-v` | also one row per phase as the phase closes, the `other` row and the `built` line of each unit | the build's phase rows, and one line per test instead of the roll-ups |
| `-vv` | also one line per module or file a phase processed, as it finishes, and each phase row names its slowest item | the build's per-module lines, and the captured output of passing tests |

`-v` and `-vv` cannot be combined with `--quiet` or `-q`. `--quiet` drops the
banners and the `profile <name>:` headers.

## Streams

The build's readout goes to stderr: banners, phase rows, items, `built` lines,
diagnostics, failures and the tally. A test run goes to stdout: the roll-ups or
per-test lines and the closing summary. `--diagnostics=json` turns stderr into
records, and `mach test --format json` turns stdout into the test runner's
events. The two are independent.

## The build readout

A phase row is written when the phase closes. A phase that runs inside another
(optimization inside lowering) is timed by the spans moved out of the phase
that hosts it, so no time is counted twice, and `other` is the unit's wall time
that no row accounts for.

```
load         48 modules     77ms
codegen      48 modules     75ms  (16 threads)  (slowest std.filesystem 9ms)
emit         48 objects      6ms
other        -              32ms
built out/app  48 modules  1 MiB  in 358ms
```

`(N threads)` shows when a phase ran on more than one worker, and
`(N target-gated, not carried forward)` when it dropped modules gated to
another target. Under `-vv` each module or file prints as an indented line
under the phase that processed it, in the order they finish, and the row names
the slowest of them.

Under `--diagnostics=json` every one of these lines is a record, written when
the line would be: a `phase` record for a row, a `phase_item` record for an
item, a `phase` record named `other` without a count, and each `built` line as
a member of the closing `summary` record. Every unit opens with a `unit` record
naming its artifact, target and profile, even when the text prints no banner or
`profile` header, so a tool can tell apart the rows of several units and of
several profiles.
[diagnostics-json.md](diagnostics-json.md#readout-records) has the schema.

## The test readout

Tests run concurrently, up to `--jobs` at once, and each is written when it
finishes, so a slow test never holds back one that has finished. Without `-v`
a module's roll-up is written once the last of its tests finishes, so a slow
test holds back only its own module's line:

```
app.parser                  12 ok     40ms
app.main                     1 ok  1 FAIL      3ms
  FAIL  app.main#fails_on_purpose  src/main.mach:11  (exit 3)
    rerun: ./out/linux-x86_64/debug/test/app/app 4
```

Under `-v` each test is its own line, written when it finishes:

```
ok    app.parser#rejects_trailing_comma   2ms
FAIL  app.main#fails_on_purpose           1ms  (exit 3)  src/main.mach:11
    rerun: ./out/linux-x86_64/debug/test/app/app 4
```

Lines and roll-ups therefore come out in the order the tests finish, which can
differ from run to run. The closing summary does not: it re-lists every failure
in collection order, the order `--list` prints, and ends with the counts.

```
failures:
  app.main#fails_on_purpose  src/main.mach:11  (exit 3)

13 passed, 1 failed, 14 total  (61ms)
```

Under `--format json` each test is a `test` event on stdout as it finishes,
between a `run_start` and a closing `summary`, as [test.md](test.md#json-output)
describes. Under `--diagnostics=json` each finished test is also a `test` record
on stderr.

## See also

- [test.md](test.md) — the `mach test` workflow
- [diagnostics-json.md](diagnostics-json.md) — the record schema
