# mach.cli.cmd.testing

## fun run

```mach
pub fun run(argv: **u8, inv: *args.ParsedInvocation) i64;
```

`mach test`: build the project's test dispatcher and run the collected tests. `-O1` and
`--emit` are refused; `--list` prints the collected tests and stops; `--format json`
writes the machine-readable event stream; `--filter`, `--runner`, `--timeout_seconds`,
and `--jobs` shape the run

argv: the full process arguments
inv: the parsed invocation for this command
ret: 0 every test passed, 1 a usage error, a build user error, or at least one failed
      test, 2 an internal or infrastructure failure or out of memory, 3 an environment
      failure during the build

