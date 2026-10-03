# mach.cli.cmd.help

## val COMMAND

```mach
pub val COMMAND: args.CommandSpec = args.CommandSpec;
```

`mach help`, and the page any command renders for `--help`

## rec Page

```mach
pub rec Page;
```

a page help renders

command: the command whose page it is, or nil for the overview
action: the action whose block it is, or nil for the whole page

## fun page_for

```mach
pub fun page_for(a: *A.Allocator, inv: *args.ParsedInvocation, argv: **u8) res[Page, fail.Fail];
```

the page an invocation asks for: what `mach help` names, or the page of the command or action
that was given `--help`

a: owns a refusal's text
inv: the parsed invocation
argv: the argument vector inv was parsed from
ret: the page, or a user failure naming an unknown command or action

## fun run

```mach
pub fun run(cx: *args.Call) res[i64, fail.Fail];
```

`mach help`, and every command's `--help`: render the page the invocation asks for on stdout

cx: the call
ret: exit.OK once the page is written, or the failure naming an unknown command or action, or
     the write that failed

## fun render_page

```mach
pub fun render_page(a: *A.Allocator, commands: args.CommandSet, out: *io_writer.Writer, p: Page, width: usize) err[fail.Fail];
```

write one page wrapped to a width

a: allocator for the text buffer
commands: the commands the page belongs to
out: the destination
p: the page
width: columns; clamped to the range 56 to 240
ret: as render_overview_at, render_command_page_at and render_action_page_at

## fun render_overview_at

```mach
pub fun render_overview_at(a: *A.Allocator, commands: args.CommandSet, out: *io_writer.Writer, requested_width: usize) err[fail.Fail];
```

write the command overview wrapped to a width

a: allocator for the text buffer
commands: the commands to list
out: the destination
requested_width: columns; clamped to the range 56 to 240
ret: ok, or an error when the command set is invalid, the buffer could not be built,
                 or the write failed

## fun render_overview

```mach
pub fun render_overview(a: *A.Allocator, commands: args.CommandSet, out: *io_writer.Writer) err[fail.Fail];
```

write the command overview at the terminal width read from COLUMNS, 100 when unset or
not a number

a: allocator for the text buffer
commands: the commands to list
out: the destination
ret: as render_overview_at

## fun render_command_page_at

```mach
pub fun render_command_page_at(a: *A.Allocator, commands: args.CommandSet, out: *io_writer.Writer, command: *args.CommandSpec,
requested_width: usize) err[fail.Fail];
```

write one command's help page wrapped to a width

a: allocator for the text buffer
commands: the commands the page belongs to
out: the destination
command: the command
requested_width: columns; clamped to the range 56 to 240
ret: ok, or an error when the command set is invalid, the buffer could not be built,
                 or the write failed

## fun render_action_page_at

```mach
pub fun render_action_page_at(a: *A.Allocator, commands: args.CommandSet, out: *io_writer.Writer, command: *args.CommandSpec,
action: *args.ActionSpec, requested_width: usize) err[fail.Fail];
```

write one action's block wrapped to a width

a: allocator for the text buffer
commands: the commands the action's command belongs to
out: the destination
command: the command
action: one of its actions
requested_width: columns; clamped to the range 56 to 240
ret: ok, or an error when the command set is invalid, the buffer could not be built,
                 or the write failed

