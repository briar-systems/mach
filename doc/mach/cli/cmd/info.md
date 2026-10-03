# mach.cli.cmd.info

## val VERSION

```mach
pub val VERSION: args.FlagSpec = args.FlagSpec;
```

`--version`, which `mach --version` gives in place of a command word

## val COMMAND

```mach
pub val COMMAND: args.CommandSpec = args.CommandSpec;
```

`mach info`

## fun run

```mach
pub fun run(cx: *args.Call) res[i64, fail.Fail];
```

`mach info`: print "mach <version>" and the host target this binary resolves for the
machine it runs on. `--version` prints the version alone; the verb `targets` prints
every target the registries compose end-to-end, one full tuple per line; combining
the two, or any other verb, is an error

cx: the call
ret: exit.OK, or a user failure for a usage error, or a target registration or host resolution
     failure

