# mach.cli.cmd.help

## val COMMAND

```mach
pub val COMMAND: std_args.CommandSpec = std_args.CommandSpec;
```

`mach help`, and the page any command renders for `--help`

## val OVERVIEW

```mach
pub val OVERVIEW: std_help.Page = std_help.Page;
```

the overview's page

## fun render

```mach
pub fun render(a: *A.Allocator, set: *std_args.CommandSet, p: std_help.Page) err[fail.Fail];
```

write a page of a set to stdout at the width COLUMNS names

a: owns the failure's text
set: the commands
p: the page
ret: ok, or an internal failure for a malformed set, or the write that failed

## fun run

```mach
pub fun run(cx: *args.Call) res[i64, fail.Fail];
```

`mach help`, and every command's `--help`: render the page the invocation asks for on stdout:
what `mach help` names, or the page of the command or action that was given `--help`

cx: the call
ret: exit.OK once the page is written, or the failure naming an unknown command or action, or
     the write that failed

