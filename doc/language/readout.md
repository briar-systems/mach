# The readout

`mach build`, `mach check` and `mach test` report their progress as it
happens. The compiler and the test runner report each event the moment it
happens: a unit starting, a phase closing, a module finishing a phase, a
diagnostic, a failure, a test finishing. The command renders each event as it
arrives, in human text or as [records](report-records.md). A record is written
the moment its event arrives. Human text writes progress lines, diagnostics,
failures and test results as they arrive, and holds a unit's `-v` table until
the unit closes, so its bars can be scaled to the unit's largest row. This page is the contract for what each level
shows, on which stream, and when.

## Levels

| Flag | `build` and `check` | `test` |
|---|---|---|
| none | diagnostics, failures, the progress line of the unit, or the units table of a selection of several, before a unit's first diagnostic or failure a line naming the unit when the selection has several, and the closing tally when an error or warning was reported | the build's output as for `build` (the test dispatcher's own line only under `-v`), then one row per module, each failure once and the verdict |
| `-v` | also each unit's table of stages with its `other` and `total` rows and its counter lines, and before each table a banner naming the unit (its target, artifact and profile, as the units table spells them) when the selection has several | the build's tables, and one line per test instead of the module rows |
| `-vv` | also, under each stage, one line per module or file it processed, each stage naming its slowest, and a table of each module's counters | the build's per-module lines, and the captured output of passing tests |

`-v` and `-vv` cannot be combined with `--quiet` or `-q`, which is refused as
`cli.flag_conflict`. `--quiet` drops the progress lines, the units table and
the banners, and keeps the line naming a unit that reports a diagnostic.

Every command that reports an error or a warning in human text ends its stderr
with one tally of them all, whichever unit, phase or check raised each, a refusal
raised before any unit is built included:

```
error[selection.unknown] mach.toml: no target named 'nosuch' (the declared ones are native)

1 error / 0 warnings
```

## Progress lines

Every progress line of every command has one shape: a verb right-aligned in a
gutter ten columns wide, then what it acted on, with any detail on an indented
line beneath it. The verbs are a closed set, one table in
`mach.report.layout`: `created`, `realized`, `updated`, `unchanged`, `added`,
`removed`, `built`, `checked`, `formatted`, `migrated`, `differs`,
`documented`, `verified`, `fetched`, `reading`, `passed`, `failed`, `skipped`
and `compiling`. A diagnostic, `error[...]`, `warning[...]` or a `note`
attached to one, keeps its own form.

A build says what it did. A cold build names the artifact, target and profile,
then beneath them its output, size, modules and wall time. A build that took
some objects from the cache says how many of its units it rebuilt in place of
the module count, and a build with nothing to do says `unchanged` on one line:

```
     built  demo  linux-x86_64  debug
            demo/out/linux-x86_64/debug/bin/demo   51 KiB   51 modules   453ms
     built  demo  linux-x86_64  debug
            demo/out/linux-x86_64/debug/bin/demo   51 KiB   1 of 51 modules rebuilt   226ms
 unchanged  demo  linux-x86_64  debug   166ms
   checked  demo  linux-x86_64  debug   158ms
```

The rebuilt count is the units the build compiled code for, read from what it
compiled. A unit is a module, or an object where the target makes one object
for several modules, and the line says `objects` then.

A selection of several units, across targets, artifacts or profiles, prints one
table under one verb when the last of them closes: `checked` when every unit was
a check, `unchanged` when every unit found everything current, `built`
otherwise. A unit that rebuilt nothing in a `built` table shows `0 of N modules
rebuilt`. The profile column shows only when the units differ in profile:

```
     built  target         artifact  profile  modules       size   time
            linux-x86_64   demo      debug    51 modules  51 KiB  412ms
            linux-aarch64  demo      debug    51 modules  54 KiB  636ms
            linux-x86_64   demo      release  51 modules  10 KiB  454ms
            linux-aarch64  demo      release  51 modules  13 KiB   1.5s
```

When stderr is a terminal and the report is human text, a build shows a live
line while it compiles, rewritten in place and cleared before anything else is
written:

```
 compiling  37 of 53   mirl.machine.select
```

It is the units the compile has finished, of the build's, and the last one
finished. It is written at most five times a second, only with a carriage
return and the erase-line sequence, and never in a pipe, under `--report json`
or `lines`, under `--quiet`, or when `TERM` is unset or `dumb`.

## Streams

The build's readout goes to stderr: progress lines, the units table, banners,
stage tables, diagnostics, failures and the tally. A test run goes to stdout:
the module rows or per-test lines, the failures and the verdict, and so does
the listing `mach check
--declassified` writes ([secrecy.md](secrecy.md#declassification)).
`--report json` and `--report lines` turn stderr into records, a test run's
among them, and leave stdout as it is. A dependency command writes its
progress lines to stdout.

## The build readout

Under `-v` each unit prints a table of its stages when it closes. A phase that runs inside another
(lowering inside code generation) is timed by the spans moved out of the phase
that hosts it, so no time is counted twice, and `other` is the unit's wall time
that no row accounts for. The `target` row is the build's target resolved and
its back half opened once for the unit: mink's formats and mirl's target,
assembled and readied. When the cache is on, a `cache` row follows `load` and
counts the modules whose objects it reused, and the phases the cache spared
count 0.

```
stage         count              time
target            1 target       15ms    ##
load             55 modules      49ms    ######
cache             0 modules      39ms    #####
resolve          55 modules      16ms    ##
sema             55 modules      69ms    ########
lower            51 objects     156ms *  ##################
verify           51 objects       0us *
codegen          51 objects     136ms    ################
passes           51 objects      78ms *  #########
select           51 objects     151ms *  ##################
alloc            51 objects     203ms *  ########################
frame            51 objects     104ms *  ############
mirl emit        51 objects     140ms *  #################
layout           51 objects      52ms *  ######
encode           51 objects     9.3ms *  #
emit             51 objects      55ms    ######
link              1 binary       32ms    ####
other                           2.0ms
total                           411ms
* summed over 16 threads
code    876 functions  30,591 ir  41,442 selected  42,239 allocated  1,035 alloc rounds
memory  26 MiB peak  349 MiB allocated in 481,509 blocks
     built  demo  linux-x86_64  debug
            demo/out/linux-x86_64/debug/bin/demo   51 KiB   51 modules   411ms
```

The table has a header row, then one row per phase in the order the phases
closed, then `other` and `total`, the unit's wall time. The rows share one set
of columns: the label, the count right-aligned with its noun beside it, singular
for a count of one, the time right-aligned, the `*` mark, and a bar of `#`. A
row with nothing to count leaves its count blank. A bar is at most 24 columns,
the longest row's, and each other row's is its time scaled to that, rounded to
the nearest column, so a row too small to show one has none. Under `-vv` a row
names its slowest item after its bar, as `(slowest std.format 9.9ms)`, and the
line for a module or file puts its time in the time column and its name
after it.

A time is rounded to the nearest of its last digit: microseconds below a
millisecond and milliseconds below a second, each to a tenth while under ten
(`8.3us`, `2.8ms`, `468ms`), seconds to a tenth below a minute (`18.5s`), then
minutes and whole seconds (`1m2s`). A time of zero is `0us`. A size is exact
bytes below a KiB, then KiB, MiB or GiB, to a tenth while under ten of the unit
and whole above (`1.5 KiB`, `78 KiB`, `393 MiB`).

Each module's object is made by a step that lowers it, holds it to mirl's
verifier and has mirl compile it. A target whose back end compiles an artifact
whole, as SPIR-V's does, makes one object by one step that lowers every module,
and its rows count that one object. The `lower` and `verify` rows are the sums of
those stages over the modules, moved out of the `codegen` row, which keeps the
rest. `lower` also holds the preparation of the lowering's inputs before the
first step runs. mirl times its own stages when a readout is shown, and each moves out of
`codegen` into a row of its own:

| Row | What it times | Read from |
|---|---|---|
| `verify` | mach's verifier run over the lowered module, and every run mirl makes in a checked compile | mach, mirl |
| `passes` | mirl's pass schedule: the target's lowering rows (calling convention, addresses, aggregates) and the neutral passes | mirl |
| `select` | instruction selection, with the constant-time check of selected code | mirl |
| `alloc` | register allocation | mirl |
| `frame` | the frame and the steps that finish framed code | mirl |
| `mirl emit` | block layout, variable ranges and the texts asked for, less masc's two rows | mirl |
| `layout` | masc laying out the object | masc |
| `encode` | masc encoding the settled layout | masc |

A test build lowers, verifies and generates its test objects in rows of their
own, `test lower`, `test verify` and `test codegen`, and a test object's mirl
and masc stages add to the same rows as above.

A build compiles its modules on `--threads` threads, the host's processors when
the flag is absent. The link reads and relocates its inputs on as many, and its
row stays the owner's wall time. The owner thread lowers each module and completes each
compile in module order, so diagnostics, objects and the rows above are the
same at every thread count. On one thread every step runs on the owner and the
rows are as described. On more, the steps' compiles run side by side: `codegen`
stays the owner's own time (it then also holds the owner's waits for compiles
to finish), and the rows the steps time are the time every thread spent in them,
added up. Such a row carries the `*` mark, explained once by the line
`* summed over N threads` beneath the table (`up to N` when the rows were summed
over different counts). A marked row is not part of the unit's wall time, so
`other` stays the time no wall row accounts for.

A target whose back end compiles an artifact whole compiles it on the owner.
The counters do not depend on the thread count, except std's heap counters.
Every step's heap draws its spans from one pool the threads share, and a large
block's room is the span the pool grants, so on more than one thread those
counters follow which spans the pool held when each step asked.

Each object is written to `obj/` as its step finishes, and the build holds no
copy of it after: a link reads it back from its file. The `emit` row is the
time of those writes, moved out of the `codegen` row (a test object's out of
`test codegen`), and what the emit phase does after the steps, which is noting
where the objects went and storing their digests. When the clock
refuses a read, the stage it would have timed comes up short, and the build
reports `readout.clock_refused` naming the module instead of showing a zero.

The counters' totals over the unit's modules and test objects follow the table
as two lines: `code`, what the code came to, and `memory`, what building it took
of the heap. A count is written with a comma every three digits and a count of
bytes as a size. A unit that ends without closing shows its table and totals
before what follows it. A counter whose total is zero, as `verify runs` is
without `--verify`, is left out. Its word on the line follows its label in the
table below (`ir` for `ir instrs`, `blocks` for `heap blocks`, `allocated` for
`heap bytes` and for `allocated instrs`, `peak` for `heap peak`), singular for a
count of one:

```
code    1 function  5 ir  7 selected  7 allocated  1 alloc round
memory  1.5 MiB peak  150 KiB allocated in 566 blocks
```

| Counter | What it counts | Read from |
|---|---|---|
| `functions` | the functions of the lowered modules | mirl's module |
| `ir instrs` | the instructions lowering built into their bodies | mirl's bodies |
| `verify runs` | the runs of mirl's verifier over them, mach's and those mirl makes in a checked compile | mach, mirl |
| `heap blocks` | the blocks a step allocated from its heap while it lowered and compiled the module, a moved reallocation counting again | std's heap |
| `heap bytes` | the bytes of those allocations, which never go down | std's heap |
| `heap peak` | the most bytes a step's heap mapped, the largest over the modules | std's heap |
| `selected instrs` | the machine instructions selection made | mirl |
| `allocated instrs` | the machine instructions after register allocation | mirl |
| `alloc rounds` | the rounds of register allocation | mirl |

Under `-vv` a table of each module's counters follows the counter lines, a row
per module and a column per counter any module posted, headed by the counters'
labels.

`(N target-gated, not carried forward)` shows when a phase dropped modules
gated to another target. Under `-vv` each module or file prints under the phase
that processed it, in the order they finished, and the row names the slowest
of them. A unit whose project or dependencies declare build steps
opens with a `steps` row, and under `-vv` each step that ran prints by name. A
step its stamp shows is current is counted but prints no line.

Under `-v` a growing cycle of instances (see
[fun.md](fun.md#instantiation-ends-or-is-refused)) prints a line each time its
instance count doubles, written at once, so an expansion that does not stop
shows while it runs:

```
instantiate  growing cycle peel in app.bin.main: 32 instances so far
```

Under `--report json` or `lines` every one of these is a record, written when
its event arrives: a `phase` record for a row, a `phase_item` record for an
item, a `counter` record for a counter's total or a module's value, a `growth`
record for a growing cycle's line, and a `phase` record named `other` without a
count. Every unit that built is a member of the closing `summary` record at
every level. Under `-v` or `-vv` every unit opens with a `unit` record naming
its artifact, target and profile, even when the text prints no banner, so a
tool can tell apart the rows of several units and of several profiles.
[report-records.md](report-records.md#readout-records) has the schema.

## The pass report

`mach build --passes` has mirl report, for each function of each module it
compiles, every optimisation pass that changed the function, in the order the
passes ran, one line each on stdout:

```
promote std.memory.raw_fill
fold std.memory.raw_fill
cse std.memory.raw_fill
bypass std.memory.raw_fill 2
vectorise std.memory.raw_fill 1
unroll std.memory.raw_fill
rotate std.memory.raw_fill 3
```

A line is the pass's name, the function's symbol and, for a pass that counts
what it does, the count: calls inlined (`inline`), loops vectorised
(`vectorise`), loops rotated (`rotate`), instructions moved (`licm`), blocks
bypassed (`bypass`), switches formed (`switch`) and stores removed (`dse`). A pass
that counts nothing gives no count. Each line is written whole, so the lines of
modules compiled side by side never mix, though their order between modules is
the order the modules finish. A build that runs no optimisation pass reports
nothing, and a build with `--passes` compiles every module, since an object the
cache holds carries no report. The flag only asks mirl for the report, which is
mirl's: what each pass counts is stated by its pass record
(`mirl.pass.record.Record.counts`).

## The test readout

Tests run concurrently, up to `--threads` at once, and each is written when it
finishes, so a slow test never holds back one that has finished. Without `-v`
a module's row is written once the last of its tests finishes, so a slow test
holds back only its own module's row:

```
app.bin.main       615us     1 ok     1 skip     1 FAIL
app.parser         323us     1 ok

FAIL  app.bin.main#fails_on_purpose  src/bin/main.mach:13  (exit 3)
      rerun: ./out/linux-x86_64/debug/test/app/app 1

    failed  1 of 4 tests, 1 skipped   764us
```

A module row is the module, its time, and its counts in columns of their own,
a skip or FAIL count only when it is not zero. When the selection holds several
(target, profile) pairs, the rows of each that runs open with a line naming it,
as `linux-x86_64 (debug):`, and each that is built and not run is a `skipped`
line.

Under `-v` each test is its own line, written when it finishes, a passing
test's captured output beneath it under `-vv`:

```
ok    app.bin.main#passes                 477us
FAIL  app.bin.main#fails_on_purpose       461us
skip  app.bin.main#skips_here             383us
ok    app.parser#rejects_trailing_comma   344us
```

When the run ends each failure is shown once, in collection order, the order
`--list` prints, whatever order the tests finished in: its name, where it is
declared and why it failed, its captured output, where its full output is when
the capture was cut, and the command that reruns it alone. Then the verdict, in
the gutter: `passed` with the count, or `failed` with how many of the count
failed, followed by how many of those timed out and how many tests skipped, and
the run's wall time:

```
    passed  1 test   238us
    failed  1 of 4 tests, 1 skipped   739us
    failed  1 of 1 test (1 timed out)   1.0s
```

Every time the test readout prints is wall time. A `-v` line's time is the
test's, from its launch to its exit. A module row's time is the module's wall
span, from the launch of its first test to the exit of its last, so tests that
ran at once count once and a row never shows more time than the run. The
verdict's time is the whole run's wall time, which the `run_end` record's
`duration_ns` also holds. A skipped test is neither a pass nor a failure. A
test that ran past its time bound is a failure.

Under `--report json` or `lines` each test is also a `test` record on stderr as
it finishes, between a `run_start` and a `run_end` record, as
[report-records.md](report-records.md#test-records) describes.

## See also

- [test.md](test.md): the `mach test` workflow
- [report-records.md](report-records.md): the record schema and its two encodings
