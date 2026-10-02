# mach.cli.cmd.info

## fun run

```mach
pub fun run(argv: **u8, inv: *args.ParsedInvocation) i64;
```

`mach info`: print "mach <version>" and the host target this binary resolves for the
machine it runs on. `--version` prints the version alone; the verb `targets` prints
every target the registries compose end-to-end, one full tuple per line; combining
the two, or any other verb, is an error

argv: the full process arguments
inv: the parsed invocation for this command
ret: exit.OK, exit.USER for a usage error, or the code `exit.of` maps a target registration or
      host resolution failure to

