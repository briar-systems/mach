# The readout

Every command reports what it did in one output language. The compiler, the
test runner and the dependency commands report each event the moment it
happens: a unit starting, a phase closing, a module finishing a phase, a
diagnostic, a failure, a test finishing. The command renders each event as it
arrives, in human text or as [records](report-records.md). This page is the
contract for what each level shows, on which stream, and when.

Human text has three kinds of line:

- a **result line** says what the command did, once per thing it acted on.
  Result lines are the same at every level.
- a **log line** says how a stage of the work went: when it closed, how long it
  took and what it processed. Log lines show only under `-v` and `-vv`.
- the **live line** says what is running now. It shows only on a terminal, and
  is gone once the work it follows ends.

A diagnostic, `error[...]`, `warning[...]` or a `note` attached to one, keeps
its own form, and is written the moment it is found, followed by a blank line.

## Levels

| Flag | What shows |
|---|---|
| none | diagnostics and result lines |
| `-v` | also a log line for each stage as it closes |
| `-vv` | also a log line for each item a stage processed, the target's facts, and for a build the unit's recap after its last stage |

`-v` and `-vv` cannot be combined with `--quiet` or `-q`, which is refused as
`cli.flag_conflict`. `--quiet` leaves out the result lines and the live line,
and keeps the diagnostics.

## Streams

Result lines go to stdout. Log lines, the recap, diagnostics and the live line
go to stderr. So a pipe that reads stdout sees the same lines at every level,
and a log that keeps stderr holds the work's timing.

`--report json` and `--report lines` turn stderr into records, the test run's
among them, and leave out the result lines and the live line. A listing, as
`mach test --list`, `mach dep list`, `mach dep outdated` or the declassified
listing `mach check --declassified` writes
([secrecy.md](secrecy.md#declassification)), is stdout at every level.

## Result lines

A result line is a verb from one closed table, left-aligned in a gutter ten
columns wide, then what it acted on and its details, each two columns apart.
The verbs are one table in `mach.report.layout`: `created`, `realized`,
`updated`, `unchanged`, `added`, `removed`, `built`, `checked`, `formatted`,
`migrated`, `differs`, `documented`, `verified`, `fetched`, `passed`, `failed`
and `skipped`.

A build names the artifact, its wall time and its output. A build with nothing
to do says `unchanged`, a check says `checked`, and a build or check that
reported errors says `failed` with their count, in place of a tally:

```
built       demo  369ms  out/linux-x86_64/debug/bin/demo
unchanged   demo  179ms  out/linux-x86_64/debug/bin/demo
checked     demo  136ms
failed      demo  2 errors  222ms
```

A count of warnings joins the line the same way, as `built  demo  3 warnings
369ms  out/...`. A count that is zero is never printed, so a line names only
what happened. A path drops its leading `./`.

A selection of several units, across targets, artifacts or profiles, writes one
result line per unit as each closes. The target joins the line when the units
differ in target, and the profile when they differ in profile, each in a column
as wide as its widest value, and the time is right-aligned:

```
unchanged   demo  linux-x86_64   debug     261ms  out/linux-x86_64/debug/bin/demo
built       demo  linux-x86_64   release   725ms  out/linux-x86_64/release/bin/demo
built       demo  linux-aarch64  release    1.7s  out/linux-aarch64/release/bin/demo
```

The other commands follow the same shape:

```
formatted   src/calc/leap.mach
unchanged   demo  2 files  3.6ms
removed     out/linux-x86_64/debug/obj
documented  demo  2 source files  5 pages  8.0ms  doc/demo
created     initq  3 files  initq/
realized    std  ^10  378ad6bb40a8  3.8s  initq/dep/std
```

A run that changes nothing says `unchanged`, so silence never has to be
interpreted.

## Log lines

A log line is a stamp, the stage, its time and what it did:

```
[  0.07] load         59ms  55 modules
```

The stamp is the seconds since the command began, to a hundredth, in a field
six wide. The stage's label is eleven columns wide and its time is
right-aligned in six. Under `-vv` each item a stage processed has a line of its
own before the stage's line, indented by two, its name after its time:

```
[  0.03]   load      2.4ms  std.format
[  0.07] load         59ms  55 modules
```

A command that is not a build logs its own stages the same way: `format` for
`mach fmt`, `remove` for `mach clean`, `load`, `sema` and `write` for `mach
doc`, `migrate` for `mach migrate`, `scaffold`, `git` and `fetch` for `mach
init`, and `fetch` for the dependency commands, whose items are the sources
they read.

## The live line

When stderr is a terminal whose `TERM` is set and not `dumb`, and the report is
human text without `--quiet`, a command shows a live line while a stage runs:
the stage, its time so far, how many of its items it has finished and of how
many, and the last one finished, in a log line's columns with the stamp left
blank:

```
         codegen      1.2s  37/52  std.format
```

It is redrawn every 50ms when it changes, only with carriage returns and
spaces, cut to the terminal's width (`COLUMNS`, or 80 when it names none), and
erased before anything else is written on either stream. A build's live line
follows its phases, a test run's follows its tests, and a dependency command's
follows its fetches.

## The build readout

Under `-v` each phase of a unit writes its log line when it closes. The
`target` line is the build's target resolved and its back half opened once for
the unit, and names the target and profile. `load`, `resolve` and `sema` count
modules, `codegen` counts objects, and `link` gives the binary's size, or `up
to date` when it had nothing to link. When the cache is on, a `cache` line
follows `load` and says how many modules' objects it reused, or `none reused`.
A phase that had nothing to do writes no line, so a build that reused every
object writes no `codegen` line:

```
[  0.01] target       12ms  linux-x86_64 debug
[  0.08] load         64ms  55 modules
[  0.13] cache        57ms  51 reused
[  0.15] resolve      12ms  34 modules
[  0.17] sema         29ms  34 modules
[  0.18] link        413us  up to date
unchanged   demo  179ms  out/linux-x86_64/debug/bin/demo
```

A phase that reported diagnostics says how many on its line, after them, as
`[  0.22] sema  69ms  56 modules  2 errors`. A phase that dropped modules gated
to another target says so as `N target-gated`. A unit whose project or
dependencies declare build steps opens with a `steps` line, and under `-vv`
each step that ran has a line by name. A step its stamp shows is current is
counted but has no line.

Under `-vv` the `target` line is followed by the target's facts, one per line,
its key in a column and then its value (`isa`, `sys`, `env`, `abi`,
`code_model`, `linkage` and `object`). Each module or file has a line under the
phase that processed it, in the order they finished. A module's `codegen` line
is its whole time in code generation, the sum of the parts the recap shows.

Under `-v` a growing cycle of instances (see
[fun.md](fun.md#instantiation-ends-or-is-refused)) writes a line each time its
instance count doubles, written at once, so an expansion that does not stop
shows while it runs:

```
[  0.31] instantiate        growing cycle peel in app.bin.main: 32 instances so far
```

### The recap

Under `-vv` each unit ends with its recap before its result line: a table of
its phases, its counters, and a table each of its modules' code generation
times and counters, each block after a blank line.

```
stage        count       time
target           1      13ms   #
load            56     110ms   ########                  slowest std.system.os 5.4ms
cache                   38ms   ###
resolve         56      17ms   #                         slowest std.format 2.0ms
sema            56      65ms   #####                     slowest std.format 7.8ms
codegen         52     322ms   ########################
  prepare       52     269ms   ####################      slowest std.types.char 4.2ms
  lower         52      76ms*  ######                    slowest std.filesystem 21ms
  passes        52      54ms*  ####                      slowest std.filesystem 16ms
  select        52     139ms*  ##########                slowest std.system.os.linux.x86_64 32ms
  alloc         52     177ms*  #############             slowest std.format 30ms
  frame         52     157ms*  ############              slowest std.format 82ms
  emit          52     192ms*  ##############            slowest std.system.os.linux.shared 67ms
  layout        52      77ms*  ######                    slowest std.system.os.linux.shared 43ms
  encode        52      18ms*  #                         slowest std.system.os.linux.shared 5.4ms
  write         52      53ms   ####                      slowest std.system.os.linux.shared 12ms
link             1      66ms   #####
other                  6.3ms
total                  637ms
* summed over 4 threads

code    878 functions  30,632 ir  41,515 selected  42,087 allocated  1,019 alloc rounds
memory  26 MiB peak  349 MiB allocated in 481,888 blocks
```

The table has a header row, then one row per phase in the order the phases
run, the parts of code generation indented beneath it, then `other`, the unit's
wall time that no row accounts for, and `total`, the unit's wall time. A row is
its label, its count, its time with the `*` mark, and a bar of `#`. A bar is at
most 24 columns, the longest row's, and each other row's is its time scaled to
that, rounded to the nearest column, so a row too small to show one has none. A
row that counted more than one item names its slowest after its bar. The
`cache` row leaves its count blank, as its count is of objects reused and not
of what it processed.

Each module's object is made by a step that lowers it, holds it to mirl's
verifier and has mirl compile it. A target whose back end compiles an artifact
whole, as SPIR-V's does, makes one object by one step that lowers every module,
and its rows count that one object. The parts of `codegen` are timed by mach,
mirl and masc, each moved out of the `codegen` row, so no time is counted
twice:

| Row | What it times | Read from |
|---|---|---|
| `prepare` | the owner's work around the steps: preparing the lowering's inputs, and its waits for compiles to finish | mach |
| `lower` | lowering a module to mirl | mach |
| `verify` | mach's verifier run over the lowered module, and every run mirl makes in a checked compile | mach, mirl |
| `passes` | mirl's pass schedule: the target's lowering rows (calling convention, addresses, aggregates) and the neutral passes | mirl |
| `select` | instruction selection, with the constant-time check of selected code | mirl |
| `alloc` | register allocation | mirl |
| `frame` | the frame and the steps that finish framed code | mirl |
| `emit` | block layout, variable ranges and the texts asked for, less masc's two rows | mirl |
| `layout` | masc laying out the object | masc |
| `encode` | masc encoding the settled layout | masc |
| `write` | writing each object to `obj/` and storing its digest | mach |

A row whose time is zero, as `verify` is without `--verify`, is left out. A
test build's test objects add to the same rows.

A build compiles its modules on `--threads` threads, the host's processors when
the flag is absent. The link reads and relocates its inputs on as many, and its
row stays the owner's wall time. The owner thread lowers each module and
completes each compile in module order, so diagnostics, objects and the rows
above are the same at every thread count. On one thread every step runs on the
owner. On more, the steps' compiles run side by side, and the rows the steps
time are the time every thread spent in them, added up. Such a row carries the
`*` mark, explained once by the line `* summed over N threads` beneath the
table (`up to N` when the rows were summed over different counts). A marked row
is not part of the unit's wall time, so `other` stays the time no wall row
accounts for.

A target whose back end compiles an artifact whole compiles it on the owner.
The counters do not depend on the thread count, except std's heap counters.
Every step's heap draws its spans from one pool the threads share, and a large
block's room is the span the pool grants, so on more than one thread those
counters follow which spans the pool held when each step asked.

Each object is written to `obj/` as its step finishes, and the build holds no
copy of it after: a link reads it back from its file. When the clock refuses a
read, the stage it would have timed comes up short, and the build reports
`readout.clock_refused` naming the module instead of showing a zero.

The counters' totals over the unit's modules and test objects follow the table
as two lines: `code`, what the code came to, and `memory`, what building it took
of the heap. A count is written with a comma every three digits and a count of
bytes as a size. A counter whose total is zero, as `verify runs` is without
`--verify`, is left out. Its word on the line follows its label in the table
below (`ir` for `ir instrs`, `blocks` for `heap blocks`, `allocated` for `heap
bytes` and for `allocated instrs`, `peak` for `heap peak`), singular for a
count of one:

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

Then two tables, a row per module: its time in each part of code generation
with its total, and its value of each counter any module posted, each headed by
the parts' or the counters' labels. A cell a module posted nothing for is
blank.

A time is rounded to the nearest of its last digit: microseconds below a
millisecond and milliseconds below a second, each to a tenth while under ten
(`8.3us`, `2.8ms`, `468ms`), seconds to a tenth below a minute (`18.5s`), then
minutes and whole seconds (`1m2s`). A time of zero is `0us`. A size is exact
bytes below a KiB, then KiB, MiB or GiB, to a tenth while under ten of the unit
and whole above (`1.5 KiB`, `78 KiB`, `393 MiB`).

### Records

Under `--report json` or `lines` every one of these is a record, written when
its event arrives: a `phase` record for a phase or a part of one, a
`phase_item` record for an item, a `fact` record for a fact of the target, a
`counter` record for a counter's total or a module's value, a `growth` record
for a growing cycle's line, and a `phase` record named `other` without a count.
Every unit that built is a member of the closing `summary` record at every
level. Under `-v` or `-vv` every unit opens with a `unit` record naming its
artifact, target and profile, so a tool can tell apart the records of several
units and of several profiles.
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

A test run reads like a build: the test build's readout, then the run's. Tests
run concurrently, up to `--threads` at once. A module's result line is written
once the last of its tests finishes, so a slow test holds back only its own
module's line. A failure is a `test.failed` diagnostic, written the moment its
test finishes. The run closes with one result line for the whole run:

```
passed      demo.bin.main   1 test   1.0ms
error[test.failed] test is_leap_year__negative exited with 3
   @ src/calc/leap.mach:21:1
21 | test is_leap_year__negative {
   | ^^^^^^^^^^^^^^^^^^^^^^^^^^^
   note rerun it alone with ./out/linux-x86_64/debug/test/demo/demo 3

failed      demo.calc.leap  4 tests  1 failed  1 skipped  1.0ms
failed      demo  5 tests  1 failed  1 skipped  705ms
```

A module's line is `passed`, `failed` when any of its tests failed, or
`skipped` when every one was skipped, then the module, its count of tests, how
many failed, timed out and were skipped, each only when not zero, and its
slowest test's time. The run's line is `passed` or `failed` with the same
counts and the time since the command began.

A failure's diagnostic says why the test failed (`exited with 3`, `was stopped
by signal 6`, `timed out after 10s`, `could not be launched`), points at where
the test is declared, and carries the test's captured output, where its full
output is when the capture was cut, and the command that reruns it alone, each
as a note. A skipped test is neither a pass nor a failure. A test that ran past
its time bound is a failure.

Under `-v` the run writes a `run` log line when it ends, its wall time and its
counts. Under `-vv` each test has a line when it finishes, `failed` or
`skipped` after its name when it did not pass, and a passing test's captured
output beneath it:

```
[  0.71]   run       543us  demo.calc.leap#is_leap_year__negative  failed
[  0.71]   run       314us  demo.calc.leap#is_leap_year__far_future  skipped
[  0.71] run         1.4ms  5 tests  1 failed  1 skipped
```

When the selection holds several (target, profile) pairs, the result lines
carry the target and profile as a build's do, and each pair that is built and
not run is a `skipped` line that says why:

```
skipped     demo  linux-aarch64  debug  not native to this host (linux-x86_64), built, not run
```

Under `--report json` or `lines` each test is a `test` record on stderr as it
finishes, between a `run_start` and a `run_end` record, as
[report-records.md](report-records.md#test-records) describes.

## See also

- [test.md](test.md): the `mach test` workflow
- [report-records.md](report-records.md): the record schema and its two encodings
