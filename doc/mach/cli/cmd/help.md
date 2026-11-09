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

## fun page

```mach
pub fun page(line: *std_args.Parsed) err[fail.Fail];
```

write the page of the command a parsed line matched to stdout: std.args' page of its usage,
text, nested commands and flags. the root's page is the overview

line: a parsed line whose command is the page's
ret: ok, or the write that failed

