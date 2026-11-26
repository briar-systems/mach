[mach](../README.md) / [cli](README.md) / **rewrite**

# mach.cli.rewrite

the pass a command makes over source files, rewriting each into the text a
transform makes of it: a project's own source tree, one named file, or a
stream. `mach fmt` and `mach migrate` are two transforms over it

Source: [`src/cli/rewrite.mach`](../../../src/cli/rewrite.mach)

Uses types from [`mach.base.diagnostic`](../base/diagnostic.md), [`mach.base.fail`](../base/fail.md), [`mach.base.source`](../base/source.md), [`mach.cli.diagnostic`](diagnostic.md), [`mach.manifest.load`](../manifest/load.md), [`mach.report.layout`](../report/layout.md), [`mach.report.readout`](../report/readout.md).

| Type | | |
| :--- | :--- | :--- |
| [`Transform`](#transform) | `def` | the text a command makes of one source's text, such as its canonical layout; a refusal `fail.is_reported` holds was reported into the store and marks the source malformed, and any other ends the command |
| [`Heading`](#heading) | `def` | the offset of the line that heads a change at the offset `at` of a source's text, such as the table a manifest line stands in, which a diff names beside the change |
| [`Report`](#report-1) | `rec` | one command's pass over its inputs |
| [`Sink`](#sink) | `def` | where the transformed text of one input goes |
| [`Project`](#project) | `rec` | what a rewrite reads of a manifest: only its `[project]` table, so it never needs a build profile |

| On `Transform` | |
| :--- | :--- |
| [`report`](#report) | a pass with nothing read yet, which begins now: the live line shows its stage from here |

| On `Report` | |
| :--- | :--- |
| [`close`](#close) | the pass is over: the live line goes, under -v its log lines say what the transform and the writes took, then each file it rewrote or found differing has its result line |
| [`result`](#result) | the pass's closing line on stdout, which a stream operand's text never shares |

| On `Sink` | |
| :--- | :--- |
| [`SINK_FILE`](#sink), [`SINK_STREAM`](#sink), [`SINK_NAME`](#sink), [`SINK_DIFF`](#sink) | |
| [`tree`](#tree) | rewrite or check one project's source directory `rel`, cleaned and relative to the root: the dependency tree is identified so it can never be selected, and the walk visits only the source tree's `.mach` files |
| [`file`](#file) | rewrite one file named directly, with no manifest read: the file's own directory is opened so the leaf is handled exactly as the project walk handles it |
| [`stream`](#stream) | rewrite the source arriving on one descriptor onto another; nothing on disk is read or written, so no manifest is involved at any point |

| On `Project` | |
| :--- | :--- |
| [`project_none`](#project_none) | a project not read yet, which project_dnit leaves alone |
| [`project_dnit`](#project_dnit) | |
| [`project_read`](#project_read) | the `[project]` table of the manifest at `manifest_path` |
| [`project_of`](#project_of) | `project_read` of a manifest already read |

| Functions | |
| :--- | :--- |
| [`text_of`](#text_of) | the text of the file at `path`, none when it cannot be read as one |

| Constants | |
| :--- | :--- |
| [`CHECK`](#constants) | `--check`, which every command over this pass takes |
| [`STREAM_NAME`](#constants) | how diagnostics and the check listing name the stream |

---

## Transform

```mach
pub def Transform: fun(*std.allocator.Allocator, str, *mach.base.diagnostic.DiagnosticStore, ptr) res[str, mach.base.fail.Fail]
```

the text a command makes of one source's text, such as its canonical layout;
a refusal `fail.is_reported` holds was reported into the store and marks the
source malformed, and any other ends the command. a text it makes beside
diagnostics in the store marks the source flagged, for what the command left in
it. the last argument is the pass's `context`

### report

```mach
pub fun report(a: *std.allocator.Allocator, diagnostics: *mach.cli.diagnostic.Report, sources: *mach.base.source.SourceMap, transform: Transform, stage: str) Report
```

a pass with nothing read yet, which begins now: the live line shows its stage from here

## Heading

```mach
pub def Heading: fun(str, usize) usize
```

the offset of the line that heads a change at the offset `at` of a source's text, such as
the table a manifest line stands in, which a diff names beside the change. the text's
length where no line heads it

## Report

```mach
pub rec Report {
    visited:     usize;
    differing:   usize;
    malformed:   usize;
    flagged:     usize;
    diagnostics: *mach.cli.diagnostic.Report;
    sources:     *mach.base.source.SourceMap;
    transform:   Transform;
    context:     ptr;
    heading:     Heading;
    verb:        mach.report.layout.Verb;
    stage:       str;
    subject:     str;
    started:     mach.report.readout.Instant;
    changing:    std.chrono.duration.Duration;
    writing:     std.chrono.duration.Duration;
    written:     usize;
    said:        Vector[Said];
    a:           *std.allocator.Allocator;
}
```

one command's pass over its inputs

| Field | Type | |
| :--- | :--- | :--- |
| `visited` | `usize` | source files read |
| `differing` | `usize` | files the transform changed (rewritten, or listed by --check) |
| `malformed` | `usize` | files the transform refused as reported; each was left unchanged |
| `flagged` | `usize` | files the transform made a text of and reported diagnostics for, each about what it left as written |
| `diagnostics` | `*mach.cli.diagnostic.Report` | the command's report, which a malformed file's diagnostics go through |
| `sources` | `*mach.base.source.SourceMap` | the source map a refused file's text is loaded into, which its failure points into |
| `transform` | `Transform` | the text each source becomes |
| `context` | `ptr` | what the command hands its transform |
| `heading` | `Heading` | what heads each change a diff prints, nil for none |
| `verb` | `mach.report.layout.Verb` | what the result line of a rewritten file says was done to it |
| `stage` | `str` | what the log line and the live line call the transform, as `format` |
| `subject` | `str` | what the closing line names, the project's id or the operand, which outlives the pass |
| `started` | `mach.report.readout.Instant` | when the pass began |
| `changing` | `std.chrono.duration.Duration` | the time the transform took over every file |
| `writing` | `std.chrono.duration.Duration` | the time the rewritten files took to write |
| `written` | `usize` | files rewritten in place |
| `said` | `Vector[Said]` | each file the pass rewrote or found differing, said once it closes |
| `a` | `*std.allocator.Allocator` | what holds `said` |

### close

```mach
pub fun close(report: *Report)
```

the pass is over: the live line goes, under -v its log lines say what the transform and the
writes took, then each file it rewrote or found differing has its result line

### result

```mach
pub fun result(report: *Report, check: bool, faulted: bool, sink: Sink)
```

the pass's closing line on stdout, which a stream operand's text never shares. it fails for a
malformed file, a file --check found the transform would change, or a fault the command found
beside, passes under --check, and says unchanged when no file differed. a pass that rewrote files
said so file by file and closes on nothing more

| Parameter | |
| :--- | :--- |
| `report` | the closed pass |
| `check` | whether --check ran it |
| `faulted` | whether the command found a fault beside the pass's files |
| `sink` | the sink the pass wrote through |

## Sink

```mach
pub def Sink: u8
```

where the transformed text of one input goes

| Field | Type | |
| :--- | :--- | :--- |
| `file` |  | replace the input file in place and name it on stdout |
| `stream` |  | write the transformed text to stdout, whether or not it differs |
| `name` |  | write no source; name the input on stdout when the transform changes it |
| `diff` |  | write no source; print the changes as a unified diff with no context lines |

| Name | Value | |
| :--- | :--- | :--- |
| `SINK_FILE` | `0` |  |
| `SINK_STREAM` | `1` |  |
| `SINK_NAME` | `2` |  |
| `SINK_DIFF` | `3` |  |

### tree

```mach
pub fun tree(a: *std.allocator.Allocator, backing: *std.allocator.Allocator, project_root: str, rel: str, sink: Sink, report: *Report) err[mach.base.fail.Fail]
```

rewrite or check one project's source directory `rel`, cleaned and relative to the root: the
dependency tree is identified so it can never be selected, and the walk visits only the source
tree's `.mach` files. each source is named from the root as given, as the manifest is, so a diff
applies from where the command ran

### file

```mach
pub fun file(a: *std.allocator.Allocator, backing: *std.allocator.Allocator, operand: str, sink: Sink, report: *Report) err[mach.base.fail.Fail]
```

rewrite one file named directly, with no manifest read: the file's own
directory is opened so the leaf is handled exactly as the project walk handles it

### stream

```mach
pub fun stream(a: *std.allocator.Allocator, in_fd: usize, out_fd: usize, sink: Sink, report: *Report) err[mach.base.fail.Fail]
```

rewrite the source arriving on one descriptor onto another; nothing on disk
is read or written, so no manifest is involved at any point. the descriptors
are parameters rather than the process streams so the caller owns that binding

## Project

```mach
pub rec Project {
    id:  str;
    src: str;
}
```

what a rewrite reads of a manifest: only its `[project]` table, so it never needs a build profile

| Field | Type | |
| :--- | :--- | :--- |
| `id` | `str` | `project.id`, which a pass's closing line names |
| `src` | `str` | `project.src`, cleaned, relative to the project root |

### project_none

```mach
pub fun project_none() Project
```

a project not read yet, which project_dnit leaves alone

### project_dnit

```mach
pub fun project_dnit(a: *std.allocator.Allocator, p: *Project)
```

### project_read

```mach
pub fun project_read(a: *std.allocator.Allocator, sources: *mach.base.source.SourceMap, manifest_path: str) res[Project, mach.base.fail.Fail]
```

the `[project]` table of the manifest at `manifest_path`

### project_of

```mach
pub fun project_of(a: *std.allocator.Allocator, doc: *mach.manifest.load.Doc) res[Project, mach.base.fail.Fail]
```

`project_read` of a manifest already read

## Functions

### text_of

```mach
pub fun text_of(a: *std.allocator.Allocator, path: str) opt[str]
```

the text of the file at `path`, none when it cannot be read as one

## Constants

| Name | Type | Value | |
| :--- | :--- | :--- | :--- |
| `CHECK` | `std.args.Flag` | `std_args.Flag{long: "check", short: 0, value: nil, repeat: false, negatable: false, help: "report what would change without writing"}` | `--check`, which every command over this pass takes |
| `STREAM_NAME` | `str` | `"<stdin>"` | how diagnostics and the check listing name the stream |

