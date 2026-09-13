# mach.cli.cmd.fmt

## rec Report

```mach
pub rec Report;
```

`mach fmt`: the project's own source in the one canonical layout

visited: source files read
differing: files whose layout is not canonical (rewritten, or listed by --check)
malformed: files that do not parse; each was reported and left unchanged

## fun format_project

```mach
pub fun format_project(a: *A.Allocator, backing: *A.Allocator, project_root: str, manifest_path: str, check: bool) res[Report, outcome.Fail];
```

format or check one project: the manifest names the source directory, the
dependency tree is identified so it can never be selected, and the walk
visits only the source tree

## fun run

```mach
pub fun run(argv: **u8, inv: *args.ParsedInvocation) i64;
```

`mach fmt <path> [--check]`

argv: the full process arguments
inv: the parsed invocation for this command
ret: 0 every file is canonical (or was made so), 1 a file differs under
      --check, a file is malformed, or the invocation is a user error, 2 internal
      failure, 3 filesystem failure

