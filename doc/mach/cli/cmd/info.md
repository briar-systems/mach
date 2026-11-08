# mach.cli.cmd.info

## val COMMAND

```mach
pub val COMMAND: std_args.Command = std_args.Command;
```

`mach info`

## val POLICY

```mach
pub val POLICY: args.Policy = args.Policy;
```

what mach holds `mach info` to beyond the parse

## fun run

```mach
pub fun run(cx: *args.Call) res[i64, fail.Fail];
```

`mach info`: print "mach <version>" and the host target this binary resolves for the
machine it runs on, or with the verb `targets` every target mink's catalog spells; any other
verb is an error

cx: the call
ret: exit.OK, or a user failure for a usage error, or a host resolution failure

