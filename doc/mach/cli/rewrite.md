# mach.cli.rewrite

the pass a command makes over source files, rewriting each into the text a
transform makes of it: a project's own source tree, one named file, or a
stream. `mach fmt` and `mach migrate` are two transforms over it

## def Transform

```mach
pub def Transform: fun(*A.Allocator, str, *diagnostic.DiagnosticStore) res[str, fail.Fail]
```

the text a command makes of one source's text, such as its canonical layout;
a refusal `fail.is_reported` holds was reported into the store and marks the
source malformed, and any other ends the command

## rec Report

```mach
pub rec Report;
```

one command's pass over its inputs

visited: source files read
differing: files the transform changed (rewritten, or listed by --check)
malformed: files the transform refused as reported; each was left unchanged
diagnostics: where a malformed file's diagnostics are rendered
transform: the text each source becomes

## val CHECK

```mach
pub val CHECK: args.FlagSpec = args.FlagSpec;
```

`--check`, which every command over this pass takes

## fun report

```mach
pub fun report(diagnostics: *io_writer.Writer, transform: Transform) Report;
```

a pass with nothing read yet

## val STREAM_NAME

```mach
pub val STREAM_NAME: str = "<stdin>"
```

how diagnostics and the check listing name the stream

## def Sink

```mach
pub def Sink: u8
```

where the transformed text of one input goes

file: replace the input file in place and name it on stdout
stream: write the transformed text to stdout, whether or not it differs
name: write no source; name the input on stdout when the transform changes it

## val SINK_FILE

```mach
pub val SINK_FILE:   Sink = 0
```

## val SINK_STREAM

```mach
pub val SINK_STREAM: Sink = 1
```

## val SINK_NAME

```mach
pub val SINK_NAME:   Sink = 2
```

## fun source_dir_of

```mach
pub fun source_dir_of(a: *A.Allocator, doc: *manifest.Doc) res[str, fail.Fail];
```

`source_dir` of a manifest already read

## fun project

```mach
pub fun project(a: *A.Allocator, backing: *A.Allocator, project_root: str, manifest_path: str, sink: Sink, report: *Report) err[fail.Fail];
```

rewrite or check one project's source: the manifest names the source
directory, the dependency tree is identified so it can never be selected, and
the walk visits only the source tree's `.mach` files

## fun tree

```mach
pub fun tree(a: *A.Allocator, backing: *A.Allocator, project_root: str, rel: str, sink: Sink, report: *Report) err[fail.Fail];
```

`project` over the source directory `rel`, cleaned and relative to the root

## fun file

```mach
pub fun file(a: *A.Allocator, backing: *A.Allocator, operand: str, sink: Sink, report: *Report) err[fail.Fail];
```

rewrite one file named directly, with no manifest read: the file's own
directory is opened so the leaf is handled exactly as the project walk handles it

## fun stream

```mach
pub fun stream(a: *A.Allocator, in_fd: usize, out_fd: usize, sink: Sink, report: *Report) err[fail.Fail];
```

rewrite the source arriving on one descriptor onto another; nothing on disk
is read or written, so no manifest is involved at any point. the descriptors
are parameters rather than the process streams so the caller owns that binding

