[mach](../README.md) / [cli](README.md) / **rewrite**

# mach.cli.rewrite

the pass a command makes over source files, rewriting each into the text a
transform makes of it: a project's own source tree, one named file, or a
stream. `mach fmt` and `mach migrate` are two transforms over it

<sub>source: [`src/cli/rewrite.mach`](../../../src/cli/rewrite.mach) · imported by 2 modules</sub>

<sub>`A` is `std.allocator` · `diagnostic` is [`mach.base.diagnostic`](../base/diagnostic.md) · `fail` is [`mach.base.fail`](../base/fail.md) · `cli_diagnostic` is [`mach.cli.diagnostic`](diagnostic.md) · `base_source` is [`mach.base.source`](../base/source.md) · `manifest_load` is [`mach.manifest.load`](../manifest/load.md) · `std_args` is `std.args`</sub>

| Type | | |
| :--- | :--- | :--- |
| [`Transform`](#transform) | `def` | the text a command makes of one source's text, such as its canonical layout; a refusal `fail.is_reported` holds was reported into the store and marks the source malformed, and any other ends the command |
| [`Report`](#report-1) | `rec` | one command's pass over its inputs |
| [`Sink`](#sink) | `def` | where the transformed text of one input goes |

| On `Transform` | |
| :--- | :--- |
| [`report`](#report) | a pass with nothing read yet |

| On `Sink` | |
| :--- | :--- |
| [`SINK_FILE`](#sink) · [`SINK_STREAM`](#sink) · [`SINK_NAME`](#sink) · [`SINK_DIFF`](#sink) | |
| [`project`](#project) | rewrite or check one project's source: the manifest names the source directory, the dependency tree is identified so it can never be selected, and the walk visits only the source tree's `.mach` files |
| [`tree`](#tree) | `project` over the source directory `rel`, cleaned and relative to the root |
| [`file`](#file) | rewrite one file named directly, with no manifest read: the file's own directory is opened so the leaf is handled exactly as the project walk handles it |
| [`stream`](#stream) | rewrite the source arriving on one descriptor onto another; nothing on disk is read or written, so no manifest is involved at any point |

| Functions | |
| :--- | :--- |
| [`text_of`](#text_of) | the text of the file at `path`, none when it cannot be read as one |
| [`source_dir_of`](#source_dir_of) | `source_dir` of a manifest already read |

| Constants | |
| :--- | :--- |
| [`CHECK`](#constants) | `--check`, which every command over this pass takes |
| [`STREAM_NAME`](#constants) | how diagnostics and the check listing name the stream |

---

## Transform

```mach
pub def Transform: fun(*A.Allocator, str, *diagnostic.DiagnosticStore, ptr) res[str, fail.Fail]
```

the text a command makes of one source's text, such as its canonical layout;
a refusal `fail.is_reported` holds was reported into the store and marks the
source malformed, and any other ends the command. the last argument is the pass's
`context`

### report

```mach
pub fun report(diagnostics: *cli_diagnostic.Report, sources: *base_source.SourceMap, transform: Transform) Report
```

a pass with nothing read yet

## Report

```mach
pub rec Report {
    visited:     usize;
    differing:   usize;
    malformed:   usize;
    diagnostics: *cli_diagnostic.Report;
    sources:     *base_source.SourceMap;
    transform:   Transform;
    context:     ptr;
}
```

one command's pass over its inputs

| Field | Type | |
| :--- | :--- | :--- |
| `visited` | `usize` | source files read |
| `differing` | `usize` | files the transform changed (rewritten, or listed by --check) |
| `malformed` | `usize` | files the transform refused as reported; each was left unchanged |
| `diagnostics` | `*cli_diagnostic.Report` | the command's report, which a malformed file's diagnostics go through |
| `sources` | `*base_source.SourceMap` | the source map a refused file's text is loaded into, which its failure points into |
| `transform` | `Transform` | the text each source becomes |
| `context` | `ptr` | what the command hands its transform |

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
| `diff` |  | write no source; name the input and print the lines the transform changes |

| Name | Value | |
| :--- | :--- | :--- |
| `SINK_FILE` | `0` |  |
| `SINK_STREAM` | `1` |  |
| `SINK_NAME` | `2` |  |
| `SINK_DIFF` | `3` |  |

### project

```mach
pub fun project(a: *A.Allocator, backing: *A.Allocator, project_root: str, manifest_path: str, sink: Sink, report: *Report) err[fail.Fail]
```

rewrite or check one project's source: the manifest names the source
directory, the dependency tree is identified so it can never be selected, and
the walk visits only the source tree's `.mach` files

### tree

```mach
pub fun tree(a: *A.Allocator, backing: *A.Allocator, project_root: str, rel: str, sink: Sink, report: *Report) err[fail.Fail]
```

`project` over the source directory `rel`, cleaned and relative to the root

### file

```mach
pub fun file(a: *A.Allocator, backing: *A.Allocator, operand: str, sink: Sink, report: *Report) err[fail.Fail]
```

rewrite one file named directly, with no manifest read: the file's own
directory is opened so the leaf is handled exactly as the project walk handles it

### stream

```mach
pub fun stream(a: *A.Allocator, in_fd: usize, out_fd: usize, sink: Sink, report: *Report) err[fail.Fail]
```

rewrite the source arriving on one descriptor onto another; nothing on disk
is read or written, so no manifest is involved at any point. the descriptors
are parameters rather than the process streams so the caller owns that binding

## Functions

### text_of

```mach
pub fun text_of(a: *A.Allocator, path: str) opt[str]
```

the text of the file at `path`, none when it cannot be read as one

### source_dir_of

```mach
pub fun source_dir_of(a: *A.Allocator, doc: *manifest_load.Doc) res[str, fail.Fail]
```

`source_dir` of a manifest already read

## Constants

| Name | Type | Value | |
| :--- | :--- | :--- | :--- |
| `CHECK` | `std_args.Flag` | `std_args.Flag{long: "check", short: 0, value: nil, repeat: false, negatable: false, help: "report what would change without writing"}` | `--check`, which every command over this pass takes |
| `STREAM_NAME` | `str` | `"<stdin>"` | how diagnostics and the check listing name the stream |

