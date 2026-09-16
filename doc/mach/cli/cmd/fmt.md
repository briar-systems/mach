# mach.cli.cmd.fmt

## rec Report

```mach
pub rec Report;
```

`mach fmt`: the project's own source in the one canonical layout

visited: source files read
differing: files whose layout is not canonical (rewritten, or listed by --check)
malformed: files that do not parse; each was reported and left unchanged

## def Operand

```mach
pub def Operand: u8
```

how the operand names its input

stream: `-`, the source arrives on stdin
source: one source file, formatted with no manifest read
project: a directory holding a mach.toml, or the mach.toml itself

## val OPERAND_STREAM

```mach
pub val OPERAND_STREAM:  Operand = 0
```

## val OPERAND_SOURCE

```mach
pub val OPERAND_SOURCE:  Operand = 1
```

## val OPERAND_PROJECT

```mach
pub val OPERAND_PROJECT: Operand = 2
```

## val STREAM_OPERAND

```mach
pub val STREAM_OPERAND: str = "-"
```

the operand spelling that names stdin

## val STREAM_NAME

```mach
pub val STREAM_NAME: str = "<stdin>"
```

how diagnostics and the check listing name the stream

## def Sink

```mach
pub def Sink: u8
```

where the canonical text of one input goes

file: replace the input file in place and name it on stdout
stream: write the canonical text to stdout, whether or not it differs
name: write no source; name the input on stdout when its layout differs

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

## fun format_project

```mach
pub fun format_project(a: *A.Allocator, backing: *A.Allocator, project_root: str, manifest_path: str, sink: Sink) res[Report, outcome.Fail];
```

format or check one project: the manifest names the source directory, the
dependency tree is identified so it can never be selected, and the walk
visits only the source tree

## fun format_one_file

```mach
pub fun format_one_file(a: *A.Allocator, backing: *A.Allocator, operand: str, sink: Sink) res[Report, outcome.Fail];
```

format one source file named directly, with no manifest read: the file's own
directory is opened so the leaf is handled exactly as the project walk handles it

## fun format_stream

```mach
pub fun format_stream(a: *A.Allocator, in_fd: i32, out_fd: i32, sink: Sink) res[Report, outcome.Fail];
```

format the source arriving on one descriptor onto another; nothing on disk is
read or written, so no manifest is involved at any point. the descriptors are
parameters rather than the process streams so the caller owns that binding

## fun classify

```mach
pub fun classify(operand: str) res[Operand, outcome.Fail];
```

which form the operand takes. `-` is the stream; a directory, or a file named
mach.toml, is a project; any other existing file is a single source

## fun run

```mach
pub fun run(argv: **u8, inv: *args.ParsedInvocation) i64;
```

`mach fmt <path>|<file>|- [--check]`

argv: the full process arguments
inv: the parsed invocation for this command
ret: 0 every input is canonical (or was made so), 1 an input differs under
      --check, an input is malformed, or the invocation is a user error, 2 internal
      failure, 3 filesystem failure

