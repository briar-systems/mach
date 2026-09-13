# mach.cli.cmd.help

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

## rec HelpRoute

```mach
pub rec HelpRoute;
```

where a help request routes

kind: the HELP_ROUTE_* value
command: the command whose page to render, for HELP_ROUTE_COMMAND
token: the unknown command word, for HELP_ROUTE_MALFORMED; empty when the shape itself was wrong

## fun resolve_help_route

```mach
pub fun resolve_help_route(argc: usize, argv: **u8) HelpRoute;
```

decide whether argv is a help request and which page it wants
`mach help`, `mach -h`, `mach --help`, and `mach help help` route to the overview;
`mach help <cmd>` and `mach <cmd> -h|--help` to the command page. a help word with extra
arguments, an unknown command followed by a help token, or a help token anywhere else
among the arguments is malformed; tokens after `--` are not inspected for commands that
truncate at the separator. fewer than two arguments is not a help request

argc: process argument count
argv: process arguments, argv[0] the program
ret: the route; HELP_ROUTE_NONE when argv is not a help request

## fun render_overview_at

```mach
pub fun render_overview_at(a: *A.Allocator, out: *writer.Writer, requested_width: usize) err[outcome.Fail];
```

write the command overview wrapped to a width

a: allocator for the text buffer
out: the destination
requested_width: columns; clamped to the range 56 to 240
ret: ok, or an error when the command schema is invalid, the buffer could not be built,
                 or the write failed

## fun render_overview

```mach
pub fun render_overview(a: *A.Allocator, out: *writer.Writer) err[outcome.Fail];
```

write the command overview at the terminal width read from COLUMNS, 100 when unset or
not a number

a: allocator for the text buffer
out: the destination
ret: as render_overview_at

## fun render_command_page_at

```mach
pub fun render_command_page_at(a: *A.Allocator, out: *writer.Writer, id: args.CommandId,
requested_width: usize) err[outcome.Fail];
```

write one command's help page wrapped to a width

a: allocator for the text buffer
out: the destination
id: the command; an id with no record is an error
requested_width: columns; clamped to the range 56 to 240
ret: ok, or an error when the command schema is invalid, the command is unknown, the
                 buffer could not be built, or the write failed

## fun render_command_page

```mach
pub fun render_command_page(a: *A.Allocator, out: *writer.Writer, id: args.CommandId) err[outcome.Fail];
```

write one command's help page at the terminal width read from COLUMNS, 100 when unset or
not a number

a: allocator for the text buffer
out: the destination
id: the command
ret: as render_command_page_at

## fun render_route

```mach
pub fun render_route(a: *A.Allocator, out: *writer.Writer, fault: *writer.Writer, route: *HelpRoute) i64;
```

render a resolved help route

a: allocator for the text buffer
out: overview and command pages go here
fault: malformed-request errors go here
route: from resolve_help_route
ret: 0 for a rendered page, 1 for a malformed request or HELP_ROUTE_NONE, 2 when rendering failed

