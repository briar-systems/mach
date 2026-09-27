# mach.cli.cmd.testing

## fun run

```mach
pub fun run(argv: **u8, inv: *args.ParsedInvocation) i64;
```

`mach test`: build the project's test dispatcher and run the collected tests. `-O1` and
`--emit` are refused; `--list` prints the collected tests and stops; `--format json`
writes the machine-readable event stream; `--filter`, `--runner`, `--timeout`,
and `--jobs` shape the run; `-o` names the dispatcher binary the way `mach build -o`
names a built one, inside the project root, so runs in one tree with different `-o`
do not collide

argv: the full process arguments
inv: the parsed invocation for this command
ret: exit.OK when every test passed, exit.USER for at least one failed test, exit.INTERNAL
      for an infrastructure failure, otherwise the code `exit.of` maps the printed failure to

