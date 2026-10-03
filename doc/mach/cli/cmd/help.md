# mach.cli.cmd.help

## val COMMAND

```mach
pub val COMMAND: args.CommandSpec = args.CommandSpec;
```

`mach help`; its handler renders the route argv resolves to, as dispatch does before any
command is read

## def HelpRouteKind

```mach
pub def HelpRouteKind: u8
```

what a help request resolved to

## val HELP_ROUTE_NONE

```mach
pub val HELP_ROUTE_NONE: HelpRouteKind = 0
```

argv is not a help request

## val HELP_ROUTE_OVERVIEW

```mach
pub val HELP_ROUTE_OVERVIEW: HelpRouteKind = 1
```

the command overview

## val HELP_ROUTE_COMMAND

```mach
pub val HELP_ROUTE_COMMAND: HelpRouteKind = 2
```

one command's page

## val HELP_ROUTE_MALFORMED

```mach
pub val HELP_ROUTE_MALFORMED: HelpRouteKind = 3
```

a help request that names an unknown command or has the wrong shape

## val HELP_ROUTE_ACTION

```mach
pub val HELP_ROUTE_ACTION: HelpRouteKind = 4
```

one action's block of a command that takes actions

## val HELP_ROUTE_UNKNOWN_ACTION

```mach
pub val HELP_ROUTE_UNKNOWN_ACTION: HelpRouteKind = 5
```

a help request naming an action its command does not have

## rec HelpRoute

```mach
pub rec HelpRoute;
```

where a help request routes

kind: the HELP_ROUTE_* value
command: the command whose page to render, for HELP_ROUTE_COMMAND and the two action routes
action: the action whose block to render, for HELP_ROUTE_ACTION
token: the unknown command word, for HELP_ROUTE_MALFORMED (empty when the shape itself was
         wrong), or the unknown action word, for HELP_ROUTE_UNKNOWN_ACTION

## fun resolve_help_route

```mach
pub fun resolve_help_route(commands: args.CommandSet, argc: usize, argv: **u8) HelpRoute;
```

decide whether argv is a help request and which page it wants
`mach help`, `mach -h`, `mach --help`, and `mach help help` route to the overview;
`mach help <cmd>` and `mach <cmd> -h|--help` to the command page, and `mach help <cmd> <action>`
and `mach <cmd> <action> -h|--help` to one action's block when the command takes actions. a help
word with other extra arguments, an unknown command followed by a help token, or a help token anywhere else
among the arguments is malformed; tokens after `--` are not inspected for commands that
truncate at the separator. fewer than two arguments is not a help request

commands: the commands argv is read against
argc: process argument count
argv: process arguments, argv[0] the program
ret: the route; HELP_ROUTE_NONE when argv is not a help request

## fun run

```mach
pub fun run(argv: **u8, inv: *args.ParsedInvocation) i64;
```

`mach help`: render the route argv resolves to against the commands it was read against

argv: the full process arguments
inv: the parsed invocation
ret: as render_route

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

## fun render_route

```mach
pub fun render_route(a: *A.Allocator, commands: args.CommandSet, out: *io_writer.Writer, fault: *io_writer.Writer, route: *HelpRoute) i64;
```

render a resolved help route

a: allocator for the text buffer
commands: the commands the route was resolved against
out: overview, command and action pages go here
fault: malformed-request and unknown-action errors go here
route: from resolve_help_route
ret: exit.OK for a rendered page, exit.USER for a malformed request, an unknown action or
          HELP_ROUTE_NONE, or the code `exit.of` maps a rendering failure to

