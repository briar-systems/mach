# mach.cli.cmd.help

## val HELP_ROUTE_NONE

```mach
pub val HELP_ROUTE_NONE: HelpRouteKind = 0
```

argv is not a help request

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
pub fun resolve_help_route(argc: usize, argv: **u8) HelpRoute;
```

decide whether argv is a help request and which page it wants
`mach help`, `mach -h`, `mach --help`, and `mach help help` route to the overview;
`mach help <cmd>` and `mach <cmd> -h|--help` to the command page, and `mach help <cmd> <action>`
and `mach <cmd> <action> -h|--help` to one action's block when the command takes actions. a help
word with other extra arguments, an unknown command followed by a help token, or a help token anywhere else
among the arguments is malformed; tokens after `--` are not inspected for commands that
truncate at the separator. fewer than two arguments is not a help request

argc: process argument count
argv: process arguments, argv[0] the program
ret: the route; HELP_ROUTE_NONE when argv is not a help request

## fun render_overview

```mach
pub fun render_overview(a: *A.Allocator, out: *io_writer.Writer) err[outcome.Fail];
```

write the command overview at the terminal width read from COLUMNS, 100 when unset or
not a number

a: allocator for the text buffer
out: the destination
ret: as render_overview_at

## fun render_route

```mach
pub fun render_route(a: *A.Allocator, out: *io_writer.Writer, fault: *io_writer.Writer, route: *HelpRoute) i64;
```

render a resolved help route

a: allocator for the text buffer
out: overview, command and action pages go here
fault: malformed-request and unknown-action errors go here
route: from resolve_help_route
ret: exit.OK for a rendered page, exit.USER for a malformed request, an unknown action or
       HELP_ROUTE_NONE, or the code `exit.of` maps a rendering failure to

