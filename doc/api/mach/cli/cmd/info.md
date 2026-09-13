# mach.cli.cmd.info

## fun run

```mach
pub fun run(argv: **u8, inv: *args.ParsedInvocation) i64;
```

`mach info`: print "mach <version>", the host os and arch, and every registered target's
capabilities. `--version` prints the version alone; the verb `targets` prints the target
matrix; combining the two, or any other verb, is an error

argv: the full process arguments
inv: the parsed invocation for this command
ret: 0 success, 1 usage error, 2 target registration failure

