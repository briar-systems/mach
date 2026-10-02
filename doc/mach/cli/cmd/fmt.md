# mach.cli.cmd.fmt

## rec Report

```mach
pub rec Report;
```

`mach fmt`: the project's own source in the one canonical layout

visited: source files read
differing: files whose layout is not canonical (rewritten, or listed by --check)
malformed: files that do not parse; each was reported and left unchanged
diagnostics: where a malformed file's diagnostics are rendered

## def Operand

```mach
pub def Operand: u8
```

how the operand names its input

stream: `-`, the source arrives on stdin
source: one source file, formatted with no manifest read
project: a directory holding a mach.toml, or the mach.toml itself

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
ret: exit.OK when every input is canonical (or was made so), exit.USER when an input differs
      under --check or is malformed, otherwise the code `exit.of` maps the printed failure to

