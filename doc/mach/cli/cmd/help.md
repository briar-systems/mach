# mach.cli.cmd.help

## val COMMAND

```mach
pub val COMMAND: std_args.Command = std_args.Command;
```

`mach help`, and the page any command renders for `--help`

## val POLICY

```mach
pub val POLICY: args.Policy = args.Policy;
```

what mach holds `mach help` to beyond the parse

## rec Row

```mach
pub rec Row;
```

one command the overview lists

name: the command word
summary: its one line

## fun page

```mach
pub fun page(a: *A.Allocator, line: *std_args.Parsed, policy: *args.Policy) err[fail.Fail];
```

write the page of the command a parsed line matched to stdout: std.args' page of its usage,
text, nested commands and flags, then the rules mach holds it to and its exit codes

a: owns the exit codes' text
line: a parsed line whose command is the page's
policy: the command's policy
ret: ok, or the write that failed

## fun overview

```mach
pub fun overview(a: *A.Allocator, title: str, root: *std_args.Command, rows: Slice[Row], footer: str) err[fail.Fail];
```

write the overview to stdout: the title, every command with its summary, the options `mach`
takes in place of a command, what follows, and the exit codes

a: owns the text
title: the first line
root: the root command, whose flags are listed
rows: the commands, in order
footer: the text after them
ret: ok, or the write that failed

